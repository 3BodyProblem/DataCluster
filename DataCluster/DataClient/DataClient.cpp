#include "DataClient.h"
#include "../DataCluster.h"
#include "../DataCenterEngine/DataCenterEngine.h"
#include "../DataCluster.h"
#include <math.h>
#include <string>
#include <algorithm>
#include "../DataNodeWrapper/NodeWrapper.h"
#include "../DataClientWrapper/ClientWrapper.h"


#define MAX_DATABUF_COUNT	(1024*1024*10)


MDataIO::MDataIO()
{
	m_pPkgBuf =0;
}

MDataIO::~MDataIO()
{
}

int		MDataIO::Instance()
{
	Release();

	int  iret = m_PushBuffer.Instance(MAX_DATABUF_COUNT);
	if (iret <0)
	{
		return -1;
	}
	
	m_pPkgBuf = new char[MAX_DATABUF_COUNT];
	if (0 == m_pPkgBuf)
	{
		return -2;
	}
	iret = m_threadClient.Create("DataThread",DataThreadFunc, this);
	if (iret <0)
	{
		return -3;
	}

	return 1;
}

void	MDataIO::Release()
{
	m_threadClient.StopThread();
	m_threadClient.Join(15000);
	m_PushBuffer.Release();
	if (m_pPkgBuf)
	{
		delete []m_pPkgBuf;
		m_pPkgBuf =0;
	}
}

int		MDataIO::PutData(XDFAPI_PkgHead* pHead, const char* pszInBuf, int	nInBytes)
{
	if (nInBytes > 0)
	{
		CriticalLock Lock(m_oSection);
		m_PushBuffer.PutData((char*)pHead, sizeof(XDFAPI_PkgHead));
		m_PushBuffer.PutData(pszInBuf, nInBytes);

		m_oWEvent.Active();
	}
	
	return 1;
}

void* STDCALL MDataIO::DataThreadFunc(void *pParam)
{
	MDataIO * pSelf = (MDataIO*)pParam;

	while( pSelf->m_threadClient.IsAlive() )
    {
		try
		{
			pSelf->m_oWEvent.Wait(1000);	//每隔1秒钟扫描
			pSelf->inner_CheckData();
		}
		catch( std::exception& err )
		{
			char	pszError[1024*2] = { 0 };

			::sprintf( pszError, "MDataIO::DataThreadFunc() : %s", err.what() );
			Global_CBAdaptor.OnLog( 3, pszError );
		}
		catch( ... )
        {
			Global_CBAdaptor.OnLog( 3, "MDataIO::DataThreadFunc() : unknow error occur" );
        }
	}
	
	return 0;
}

void MDataIO::inner_CheckData()
{
	int nsize =0;
	int nheadsize = sizeof(XDFAPI_PkgHead);
	int npkgsize =0;
	char tempbuf[100]={0};
	XDFAPI_PkgHead* pHead = (XDFAPI_PkgHead*)tempbuf;

	do 
	{
		nsize = m_PushBuffer.GetRecordCount();
		if (nsize < nheadsize )
		{
			return;
		}
		m_PushBuffer.LookData(tempbuf, nheadsize);
		npkgsize = pHead->PkgSize;

		if (nsize < (nheadsize + npkgsize) )
		{
			return;
		}
		
		m_PushBuffer.GetData(tempbuf, nheadsize);
		m_PushBuffer.GetData(m_pPkgBuf, npkgsize);

		if (Global_pSpi)
		{
			Global_pSpi->XDF_OnRspRecvData(pHead, m_pPkgBuf, npkgsize);
		}

	} while (1);

}


///< ------------------------------------------------------------------------------


MStreamWrite::MStreamWrite(char* pszinbuf, int ninbytes)
{
	m_pInBuf = pszinbuf;
	m_nInBytes = ninbytes;

	m_nLastType =0;
	m_nMsgCount =0;
	m_nOffset =0;
	m_bError =false;
	m_pUniHead =0;
}

MStreamWrite::MStreamWrite()
{
	m_pInBuf =0;
	m_nInBytes =0;
	m_nLastType =0;
	m_nMsgCount =0;
	m_nOffset =0;
	m_bError =false;
	m_pUniHead =0;
}

MStreamWrite::~MStreamWrite()
{
	
}

void	MStreamWrite::Attach(char* pszinbuf, int ninbytes)
{
	m_pInBuf = pszinbuf;
	m_nInBytes = ninbytes;
	
	m_nLastType =0;
	m_nMsgCount =0;
	m_nOffset =0;
	m_bError =false;
	m_pUniHead =0;
}

bool	MStreamWrite::IsError()
{
	return m_bError;
}
int		MStreamWrite::GetOffset()
{
	return m_nOffset;
}
int		MStreamWrite::GetCount()
{
	return m_nMsgCount;
}

void	MStreamWrite::Detach()
{
	//do nothing now!!
}

void	MStreamWrite::PutSingleMsg(unsigned int ntype, char* pStruct, int nStructSize)
{
	if (m_bError)		//错误已出现
		return;
	
	int noffset = nStructSize + sizeof(XDFAPI_MsgHead);
	if (m_nInBytes - m_nOffset < noffset)	//容量超了
	{
		m_bError =true;
		return;
	}
	
	char* pChar = m_pInBuf+m_nOffset;
	XDFAPI_MsgHead* pHead = (XDFAPI_MsgHead*)(pChar);
	pHead->MsgType = ntype;
	pHead->MsgLen = nStructSize;
	memcpy(pChar+sizeof(XDFAPI_MsgHead), pStruct, nStructSize);

	m_nMsgCount += 1;
	m_nLastType =0;
	m_nOffset += noffset;
	m_pUniHead =0;
}

void	MStreamWrite::PutMsg(unsigned int ntype, char* pStruct, int nStructSize)
{
	if (m_bError)	//错误已出现
		return;
	
	//[1]
	int realoffset = nStructSize;
	bool breuse =false;
	//检查是否m_pUniHead为0
	//检查MsgLen是否越界
	//检查类型和LastType是否一致
	if (0 == m_pUniHead || m_pUniHead->MsgLen >64000 || m_nLastType!= ntype)
	{
		realoffset += sizeof(XDFAPI_UniMsgHead);
		breuse =false;
	}
	else
	{
		breuse =true;
	}

	//[2]容量检查
	if (m_nInBytes - m_nOffset < realoffset) //容量超了
	{
		m_bError =true;
		return;
	}

	if (!breuse)
	{
		char* pChar = m_pInBuf+m_nOffset;
		m_pUniHead = (XDFAPI_UniMsgHead*)pChar;
		m_pUniHead->MsgType = (-1)*ntype;
		m_pUniHead->MsgCount=1;
		m_pUniHead->MsgLen = nStructSize + sizeof(short);
		memcpy(pChar+sizeof(XDFAPI_UniMsgHead), pStruct, nStructSize);
		
	}
	else
	{
		m_pUniHead->MsgCount += 1;
		m_pUniHead->MsgLen += nStructSize;
		char* pChar = m_pInBuf+m_nOffset;
		memcpy(pChar, pStruct, nStructSize);
	}

	m_nLastType = ntype;
	m_nMsgCount += 1;
	m_nOffset += realoffset;
}


///< ------------------------------------------------------------------------------


const unsigned int	MAX_QUERY_BUF_SIZE = 1024*1024*80;
MDataIO				g_oDataIO;


MDataClient::MDataClient()
: m_pQueryBuffer( NULL )
{
}

MDataClient::~MDataClient()
{
	if( NULL != m_pQueryBuffer )
	{
		delete [] m_pQueryBuffer;
		m_pQueryBuffer = NULL;
	}
}

int MDataClient::GetRate( int nMarketID, unsigned int nKind )
{
	T_KIND_MAP::iterator it = m_mapMarketKind.find( nMarketID );

	if( it != m_mapMarketKind.end() )
	{
		if( it->second.size() <= nKind )
		{
			return -1;
		}

		return ::pow( (double)10, it->second[nKind].PriceRate );
	}

	return -2;
}

bool MDataClient::InQuoteClientApiMode()
{
	return NULL != m_pQueryBuffer;
}

int STDCALL	MDataClient::Init()
{
	if( NULL == m_pQueryBuffer )
	{
		m_pQueryBuffer = new char[MAX_QUERY_BUF_SIZE];
	}

	if (!Global_bInit)
	{
		Global_bInit = true;

		{///< 商品期货(上海/郑州/大连)
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_CNF];
		strncpy( refKindTable[0].KindName, "指数保留", 8 );
		refKindTable[0].PriceRate = 0;
		refKindTable[0].LotSize = 0;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "郑州期指", 8 );
		refKindTable[1].PriceRate = 2;
		refKindTable[1].LotSize = 100;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		strncpy( refKindTable[2].KindName, "大连期指", 8 );
		refKindTable[2].PriceRate = 2;
		refKindTable[2].LotSize = 100;
		refKindTable[2].Serial = refKindTable.size() - 1;
		refKindTable[2].WareCount = 0;
		strncpy( refKindTable[3].KindName, "上海期指", 8 );
		refKindTable[3].PriceRate = 2;
		refKindTable[3].LotSize = 100;
		refKindTable[3].Serial = refKindTable.size() - 1;
		refKindTable[3].WareCount = 0;
		strncpy( refKindTable[4].KindName, "郑州合约", 8 );
		refKindTable[4].PriceRate = 2;
		refKindTable[4].LotSize = 100;
		refKindTable[4].Serial = refKindTable.size() - 1;
		refKindTable[4].WareCount = 0;
		strncpy( refKindTable[5].KindName, "大连合约", 8 );
		refKindTable[5].PriceRate = 2;
		refKindTable[5].LotSize = 100;
		refKindTable[5].Serial = refKindTable.size() - 1;
		refKindTable[5].WareCount = 0;
		strncpy( refKindTable[6].KindName, "上海合约", 8 );
		refKindTable[6].PriceRate = 2;
		refKindTable[6].LotSize = 100;
		refKindTable[6].Serial = refKindTable.size() - 1;
		refKindTable[6].WareCount = 0;
		strncpy( refKindTable[7].KindName, "能源合约", 8 );
		refKindTable[7].PriceRate = 2;
		refKindTable[7].LotSize = 100;
		refKindTable[7].Serial = refKindTable.size() - 1;
		refKindTable[7].WareCount = 0;
		}
		{///< 商品期货和商品期权(上海/郑州/大连)
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_CNFOPT];
		strncpy( refKindTable[0].KindName, "指数保留", 8 );
		refKindTable[0].PriceRate = 0;
		refKindTable[0].LotSize = 0;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "郑州期权", 8 );
		refKindTable[1].PriceRate = 2;
		refKindTable[1].LotSize = 100;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		strncpy( refKindTable[2].KindName, "大连期权", 8 );
		refKindTable[2].PriceRate = 2;
		refKindTable[2].LotSize = 100;
		refKindTable[2].Serial = refKindTable.size() - 1;
		refKindTable[2].WareCount = 0;
		strncpy( refKindTable[3].KindName, "上海期权", 8 );
		refKindTable[3].PriceRate = 2;
		refKindTable[3].LotSize = 100;
		refKindTable[3].Serial = refKindTable.size() - 1;
		refKindTable[3].WareCount = 0;
		strncpy( refKindTable[4].KindName, "能源期权", 8 );
		refKindTable[4].PriceRate = 2;
		refKindTable[4].LotSize = 100;
		refKindTable[4].Serial = refKindTable.size() - 1;
		refKindTable[4].WareCount = 0;
		}
		{///< 中金期货
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_CF];
		strncpy( refKindTable[0].KindName, "股指期货", 8 );
		refKindTable[0].PriceRate = 2;
		refKindTable[0].LotSize = 1;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "国债期货", 8 );
		refKindTable[1].PriceRate = 3;
		refKindTable[1].LotSize = 1;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		}
		{///< 中金期权
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_ZJOPT];
		strncpy( refKindTable[0].KindName, "股指期权", 8 );
		refKindTable[0].PriceRate = 2;
		refKindTable[0].LotSize = 1;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "国债期权", 8 );
		refKindTable[1].PriceRate = 3;
		refKindTable[1].LotSize = 1;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		}
		{///< 上证期权
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_SHOPT];
		strncpy( refKindTable[0].KindName, "保留指数", 8 );
		refKindTable[0].PriceRate = 3;
		refKindTable[0].LotSize = 1;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "个股期权", 8 );
		refKindTable[1].PriceRate = 3;
		refKindTable[1].LotSize = 1;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		strncpy( refKindTable[2].KindName, "ETF期权", 8 );
		refKindTable[2].PriceRate = 4;
		refKindTable[2].LotSize = 1;
		refKindTable[2].Serial = refKindTable.size() - 1;
		refKindTable[2].WareCount = 0;
		}
		{///< 深圳期权
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_SZOPT];
		strncpy( refKindTable[0].KindName, "保留指数", 8 );
		refKindTable[0].PriceRate = 3;
		refKindTable[0].LotSize = 1;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "个股期权", 8 );
		refKindTable[1].PriceRate = 3;
		refKindTable[1].LotSize = 1;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		strncpy( refKindTable[2].KindName, "ETF期权", 8 );
		refKindTable[2].PriceRate = 4;
		refKindTable[2].LotSize = 1;
		refKindTable[2].Serial = refKindTable.size() - 1;
		refKindTable[2].WareCount = 0;
		}
		{///< 上证L1
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_SH];
		strncpy( refKindTable[0].KindName, "上证指数", 8 );
		refKindTable[0].PriceRate = 2;
		refKindTable[0].LotSize = 1;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "上证Ａ股 ", 8 );
		refKindTable[1].PriceRate = 2;
		refKindTable[1].LotSize = 100;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		strncpy( refKindTable[2].KindName, "上证Ｂ股", 8 );
		refKindTable[2].PriceRate = 3;
		refKindTable[2].LotSize = 100;
		refKindTable[2].Serial = refKindTable.size() - 1;
		refKindTable[2].WareCount = 0;
		strncpy( refKindTable[3].KindName, "上证基金", 8 );
		refKindTable[3].PriceRate = 3;
		refKindTable[3].LotSize = 100;
		refKindTable[3].Serial = refKindTable.size() - 1;
		refKindTable[3].WareCount = 0;
		strncpy( refKindTable[4].KindName, "上证债券", 8 );
		refKindTable[4].PriceRate = 2;
		refKindTable[4].LotSize = 10;
		refKindTable[4].Serial = refKindTable.size() - 1;
		refKindTable[4].WareCount = 0;
		strncpy( refKindTable[5].KindName, "上证转债", 8 );
		refKindTable[5].PriceRate = 2;
		refKindTable[5].LotSize = 10;
		refKindTable[5].Serial = refKindTable.size() - 1;
		refKindTable[5].WareCount = 0;
		strncpy( refKindTable[6].KindName, "上证回购", 8 );
		refKindTable[6].PriceRate = 3;
		refKindTable[6].LotSize = 100;
		refKindTable[6].Serial = refKindTable.size() - 1;
		refKindTable[6].WareCount = 0;
		strncpy( refKindTable[7].KindName, "上证ETF", 8 );
		refKindTable[7].PriceRate = 3;
		refKindTable[7].LotSize = 100;
		refKindTable[7].Serial = refKindTable.size() - 1;
		refKindTable[7].WareCount = 0;
		strncpy( refKindTable[8].KindName, "基金通", 8 );
		refKindTable[8].PriceRate = 4;
		refKindTable[8].LotSize = 100;
		refKindTable[8].Serial = refKindTable.size() - 1;
		refKindTable[8].WareCount = 0;
		strncpy( refKindTable[9].KindName, "上证权证", 8 );
		refKindTable[9].PriceRate = 3;
		refKindTable[9].LotSize = 100;
		refKindTable[9].Serial = refKindTable.size() - 1;
		refKindTable[9].WareCount = 0;
		strncpy( refKindTable[10].KindName, "上证其它", 8 );
		refKindTable[10].PriceRate = 3;
		refKindTable[10].LotSize = 100;
		refKindTable[10].Serial = refKindTable.size() - 1;
		refKindTable[10].WareCount = 0;
		strncpy( refKindTable[11].KindName, "开放基金", 8 );
		refKindTable[11].PriceRate = 4;
		refKindTable[11].LotSize = 100;
		refKindTable[11].Serial = refKindTable.size() - 1;
		refKindTable[11].WareCount = 0;
		strncpy( refKindTable[12].KindName, "风险警示", 8 );
		refKindTable[12].PriceRate = 3;
		refKindTable[12].LotSize = 100;
		refKindTable[12].Serial = refKindTable.size() - 1;
		refKindTable[12].WareCount = 0;
		strncpy( refKindTable[13].KindName, "上证退市", 8 );
		refKindTable[13].PriceRate = 3;
		refKindTable[13].LotSize = 100;
		refKindTable[13].Serial = refKindTable.size() - 1;
		refKindTable[13].WareCount = 0;
		strncpy( refKindTable[14].KindName, "附加指数", 8 );
		refKindTable[14].PriceRate = 2;
		refKindTable[14].LotSize = 100;
		refKindTable[14].Serial = refKindTable.size() - 1;
		refKindTable[14].WareCount = 0;
		}
		{///< 深证L1
		std::map<int,XDFAPI_MarketKindInfo>&	refKindTable = m_mapMarketKind[XDF_SZ];
		strncpy( refKindTable[0].KindName, "深证指数", 8 );
		refKindTable[0].PriceRate = 2;
		refKindTable[0].LotSize = 1;
		refKindTable[0].Serial = refKindTable.size() - 1;
		refKindTable[0].WareCount = 0;
		strncpy( refKindTable[1].KindName, "深证Ａ股 ", 8 );
		refKindTable[1].PriceRate = 2;
		refKindTable[1].LotSize = 100;
		refKindTable[1].Serial = refKindTable.size() - 1;
		refKindTable[1].WareCount = 0;
		strncpy( refKindTable[2].KindName, "深证Ｂ股", 8 );
		refKindTable[2].PriceRate = 2;
		refKindTable[2].LotSize = 100;
		refKindTable[2].Serial = refKindTable.size() - 1;
		refKindTable[2].WareCount = 0;
		strncpy( refKindTable[3].KindName, "深证基金", 8 );
		refKindTable[3].PriceRate = 3;
		refKindTable[3].LotSize = 100;
		refKindTable[3].Serial = refKindTable.size() - 1;
		refKindTable[3].WareCount = 0;
		strncpy( refKindTable[4].KindName, "深证债券", 8 );
		refKindTable[4].PriceRate = 3;
		refKindTable[4].LotSize = 10;
		refKindTable[4].Serial = refKindTable.size() - 1;
		refKindTable[4].WareCount = 0;
		strncpy( refKindTable[5].KindName, "深证转债", 8 );
		refKindTable[5].PriceRate = 3;
		refKindTable[5].LotSize = 10;
		refKindTable[5].Serial = refKindTable.size() - 1;
		refKindTable[5].WareCount = 0;
		strncpy( refKindTable[6].KindName, "深证回购", 8 );
		refKindTable[6].PriceRate = 3;
		refKindTable[6].LotSize = 10;
		refKindTable[6].Serial = refKindTable.size() - 1;
		refKindTable[6].WareCount = 0;
		strncpy( refKindTable[7].KindName, "深证ETF", 8 );
		refKindTable[7].PriceRate = 3;
		refKindTable[7].LotSize = 100;
		refKindTable[7].Serial = refKindTable.size() - 1;
		refKindTable[7].WareCount = 0;
		strncpy( refKindTable[8].KindName, "中小板块", 8 );
		refKindTable[8].PriceRate = 2;
		refKindTable[8].LotSize = 100;
		refKindTable[8].Serial = refKindTable.size() - 1;
		refKindTable[8].WareCount = 0;
		strncpy( refKindTable[9].KindName, "创业板块", 8 );
		refKindTable[9].PriceRate = 2;
		refKindTable[9].LotSize = 100;
		refKindTable[9].Serial = refKindTable.size() - 1;
		refKindTable[9].WareCount = 0;
		strncpy( refKindTable[10].KindName, "Ｂ转Ｈ股", 8 );
		refKindTable[10].PriceRate = 3;
		refKindTable[10].LotSize = 1;
		refKindTable[10].Serial = refKindTable.size() - 1;
		refKindTable[10].WareCount = 0;
		strncpy( refKindTable[11].KindName, "股份转让", 8 );
		refKindTable[11].PriceRate = 3;
		refKindTable[11].LotSize = 100;
		refKindTable[11].Serial = refKindTable.size() - 1;
		refKindTable[11].WareCount = 0;
		strncpy( refKindTable[12].KindName, "深证优先", 8 );
		refKindTable[12].PriceRate = 3;
		refKindTable[12].LotSize = 100;
		refKindTable[12].Serial = refKindTable.size() - 1;
		refKindTable[12].WareCount = 0;
		strncpy( refKindTable[13].KindName, "LOF基金", 8 );
		refKindTable[13].PriceRate = 3;
		refKindTable[13].LotSize = 100;
		refKindTable[13].Serial = refKindTable.size() - 1;
		refKindTable[13].WareCount = 0;
		strncpy( refKindTable[14].KindName, "非LOF", 8 );
		refKindTable[14].PriceRate = 3;
		refKindTable[14].LotSize = 100;
		refKindTable[14].Serial = refKindTable.size() - 1;
		refKindTable[14].WareCount = 0;
		strncpy( refKindTable[13].KindName, "深证其它", 8 );
		refKindTable[13].PriceRate = 3;
		refKindTable[13].LotSize = 100;
		refKindTable[13].Serial = refKindTable.size() - 1;
		refKindTable[13].WareCount = 0;
		strncpy( refKindTable[14].KindName, "深证退市", 8 );
		refKindTable[14].PriceRate = 2;
		refKindTable[14].LotSize = 100;
		refKindTable[14].Serial = refKindTable.size() - 1;
		refKindTable[14].WareCount = 0;
		}

		if( g_oDataIO.Instance() < 0 )
		{
			return -1;
		}

		if( 0 != StartWork( &Global_CBAdaptor ) )
		{
			return -2;
		}
	}

	return 1;
}

void STDCALL MDataClient::Release()
{
	if( Global_bInit )
	{
		DataIOEngine::GetEngineObj().WriteInfo( "MDataClient::Release() : Releasing ......" );
		SimpleTask::StopAllThread();
		Global_bInit = false;
		RegisterSpi( NULL );
		g_oDataIO.Release();
		DataIOEngine::GetEngineObj().WriteInfo( "MDataClient::Release() : Released ......" );
	}
}

void STDCALL MDataClient::RegisterSpi( QuoteClientSpi* pspi )
{
	Global_pSpi = pspi;
}

int STDCALL	MDataClient::BeginWork()
{
	return 1;
}

void STDCALL MDataClient::EndWork()
{
	::EndWork();
}


static unsigned int	s_Date4CNF = 0;
static unsigned int	s_Date4CNFOPT = 0;


int	 STDCALL		MDataClient::GetMarketInfo( unsigned char cMarket, char* pszInBuf, int nInBytes )
{
	int										nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64						nSerialNo = 0;
	XDFAPI_MarketKindHead					oHead = { 0 };
	T_Inner_MarketInfo						tagMkInfo;
	MStreamWrite							oMSW( pszInBuf, nInBytes );
	BigTableDatabase&						refDatabase = EngineWrapper4DataClient::GetObj().GetDatabaseObj();
	std::map<int,XDFAPI_MarketKindInfo>&	mapKindTable = m_mapMarketKind[cMarket];
	unsigned int							nKindCount = mapKindTable.size();

	if( nInnerMkID < 0 )
	{
		return -1;
	}

	for( int i = 0; i < 3; i++ )
	{
		nSerialNo = 0;
		if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(T_Inner_MarketInfo), nSerialNo ) <= 0 )
		{
			if( XDF_CNF == cMarket || XDF_CNFOPT == cMarket )
			{
				if( XDF_CNF == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
				}

				if( XDF_CNFOPT == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
				}

				continue;
			}

			return -2;
		}

		oHead.WareCount += tagMkInfo.objData.uiWareCount;
		oHead.KindCount = nKindCount;

		if( XDF_CNF != cMarket && XDF_CNFOPT != cMarket )
		{
			break;
		}

		if( XDF_CNF == cMarket || XDF_CNFOPT ==  cMarket )
		{
			if( XDF_CNF == cMarket )
			{
				if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
				if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
			}

			if( XDF_CNFOPT == cMarket )
			{
				if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
				if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
			}
		}
	}

	oMSW.PutSingleMsg( 100, (char*)&oHead, sizeof(XDFAPI_MarketKindHead) );

	if( XDF_CNF == cMarket )
	{
		s_Date4CNF = tagMkInfo.objData.uiMarketDate;
	}
	else if( XDF_CNFOPT == cMarket )
	{
		s_Date4CNFOPT = tagMkInfo.objData.uiMarketDate;
	}

	for( int n = 0; n < nKindCount; n++ )
	{
		XDFAPI_MarketKindInfo			oInfo = { 0 };
		XDFAPI_MarketKindInfo&			refKindTable = mapKindTable[n];

		oInfo.Serial = n;									///< 序号
		memcpy(oInfo.KindName, refKindTable.KindName, 8);	///< 类别的名称
		oInfo.WareCount = refKindTable.WareCount;			///< 该类商品的数量
		oInfo.PriceRate = refKindTable.PriceRate;			///< 该类别中价格放大倍数[10的多少次方]
		oInfo.LotSize = refKindTable.LotSize;				///< 该类别中"手"比率
		oMSW.PutMsg( 101, (char*)&oInfo, sizeof(oInfo) );
	}

	oMSW.Detach();

	return oMSW.GetOffset();
}

int CastKindID4SHL1( char* pszCode )
{
	if( (pszCode[0] == '0' && pszCode[1] == '0' && pszCode[2] == '0') && (pszCode[3] == '0' || pszCode[3] == '1' || pszCode[3] == '2' || pszCode[3] == '3' || pszCode[3] == '4' || pszCode[3] == '7' || pszCode[3] == '8' || pszCode[3] == '9') )
		return 0;	///< 上证指数 

	if( pszCode[0] == '6' )
		return 1;	///< 上证Ａ股 

	if( pszCode[0] == '9' && pszCode[1] == '0' )
		return 2;	///< 上证Ｂ股

	if( pszCode[0] == '5' && pszCode[1] == '0' )
		return 3;	///< 上证基金

	if( (pszCode[0] == '0' && (pszCode[1] == '0' || pszCode[1] == '1' || pszCode[1] == '2' )) || (pszCode[0] == '1' && pszCode[1] == '2') || (pszCode[0] == '1' && pszCode[1] == '3' && (pszCode[2] == '0' || pszCode[2] == '5' || pszCode[2] == '6')) )
		return 4;	///< 上证债券

	if( pszCode[0] == '1' && (pszCode[1] == '0' || pszCode[1] == '1' || (pszCode[1] == '0' && pszCode[2] == '5')) )
		return 5;	///< 上证转债	

	if( (pszCode[0] == '2' && pszCode[1] == '0' && (pszCode[2] == '1' || pszCode[2] == '2' || pszCode[2] == '3' || pszCode[2] == '4')) || (pszCode[0] == '1' && pszCode[1] == '0' && pszCode[2] == '6') )
		return 6;	///< 上证回购
 
	if( pszCode[0] == '5' && pszCode[1] == '1' && (pszCode[2] == '0' || pszCode[2] == '1' || pszCode[2] == '2' || pszCode[2] == '3' || pszCode[2] == '8') )
		return 7;	///< 上证ETF

	if( (pszCode[0] == '5' && pszCode[1] == '1' && pszCode[2] == '9') || (pszCode[0] == '5' && pszCode[1] == '2' && pszCode[2] == '1') || (pszCode[0] == '5' && pszCode[1] == '2' && pszCode[2] == '2') || (pszCode[0] == '5' && pszCode[1] == '2' && pszCode[2] == '3') )
		return 8;	///< 基金通

	if( pszCode[0] == '5' && pszCode[1] == '8' )
		return 9;	///< 上证权证

	return 10;	///< 上证其它
}

int CastKindID4SZL1( const char* pszCode )
{
	if( pszCode[0]=='3' && pszCode[1]=='9' && pszCode[2]=='9' )
	{
		return 0;	///< 指数
	}
	else if( pszCode[0]=='0' && pszCode[1]=='0' && pszCode[2] == '2' )
	{
		return 8;	///< 中小板块
	}
	else if( pszCode[0]=='0' && pszCode[1]=='0' && pszCode[2] == '3' )
	{
		return 8;	///< 中小板块
	}
	else if( pszCode[0]=='0' && pszCode[1]=='0' && pszCode[2] == '4' )
	{
		return 8;	///< 中小板块
	}
	else if( pszCode[0]=='0' && pszCode[1]=='0' && (pszCode[2]=='0' || pszCode[2]=='1' || pszCode[2]=='5' || pszCode[2]=='6' || pszCode[2]=='7' || pszCode[2]=='8' || pszCode[2]=='9') )
	{
		return 1;	///< A股
	}
	else if( pszCode[0]=='2' && pszCode[1]=='0' )
	{
		return 2;	///< B股
	}
	else if( pszCode[0]=='1' && pszCode[1]=='5' && pszCode[2]=='1' )
	{
		return 3;	///< 基金
	}
	else if( pszCode[0]=='1' && pszCode[1]=='7' )
	{
		return 3;	///< 基金
	}
	else if( pszCode[0]=='1' && pszCode[1]=='8' )
	{
		return 3;	///< 基金
	}
	else if( pszCode[0]=='1' && pszCode[1]=='0' )
	{
		return 4;	///< 债券
	}
	else if( pszCode[0]=='1' && pszCode[1]=='1' )
	{
		return 4;	///< 债券
	}
	else if( pszCode[0]=='1' && pszCode[1]=='2' )
	{
		return 5;	///< 转债
	}
	else if( pszCode[0]=='1' && pszCode[1]=='3' )										///< 国债回购
	{
		return 6;	///< 回购
	}
	else if( pszCode[0]=='1' && pszCode[1]=='5' && pszCode[2]=='9' && pszCode[3]=='9' )		///< ETF
	{
		return 7;	///< ETF
	}
	else if( pszCode[0]=='2' && pszCode[1]=='9' )										///< B股转H股
	{
		return 10;	///< B转H股
	}
	else if( pszCode[0]=='3' && pszCode[1]=='0' )										///< 创业板块
	{
		return 9;	///< 创业板块
	}
	else if( pszCode[0]=='1' && pszCode[1]=='5' )
	{
		return 11;	///< LOF基金
	}
	else if( pszCode[0]=='1' && pszCode[1]=='6' )
	{
		return 11;	///< LOF基金
	}
/*	else if( Code[0]=='3' && Code[1]=='7' )										///< 创业板增发
	{
		return 9;	///< 创业板块
	}
	else if( Code[0]=='3' && Code[1]=='8' )										///< 创业板权证
	{
		return 9;	///< 创业板块
	}*/
	else
	{
		return 13;	///< 其它
	}
}

int	CastKindID4CFF( char* pszCode )
{
	if( pszCode[0] == 'T' || (pszCode[0] == 'T' && pszCode[1] == 'F') )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int	CastKindID4CFFOPT( char* pszCode )
{
	if( pszCode[0] == 'T' || (pszCode[0] == 'T' && pszCode[1] == 'F') )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int	CastKindID4CNF( unsigned int nMkID, char* pszCode )
{
	if( nMkID == QUO_MARKET_CZCE )
	{
		return 4;
	}
	else if( nMkID == QUO_MARKET_DCE )
	{
		return 5;
	}
	else if( nMkID == QUO_MARKET_SHFE )
	{
		return 6;
	}
	else
	{
		return 7;
	}
}

int CastKindID4SHOPT( char* pszCode )
{
	if( ::memcmp( pszCode, "1000", 4 ) == 0 )
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

int CastKindID4SZOPT( char* pszCode )
{
	if( ::memcmp( pszCode, "9000", 4 ) == 0 )
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

int	CastKindID4CNFOPT( unsigned int nMkID, char* pszCode )
{
	if( nMkID == QUO_MARKET_CZCEOPT )
	{
		return 1;
	}
	else if( nMkID == QUO_MARKET_DCEOPT )
	{
		return 2;
	}
	else if( nMkID == QUO_MARKET_SHFEOPT )
	{
		return 3;
	}

	return 0;
}


T_KIND_MAP		MDataClient::m_mapMarketKind;


int	STDCALL		MDataClient::GetCodeTable( unsigned char cMarket, char* pszInBuf, int nInBytes, int& nCount )
{
	int							nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64			nSerialNo = 0;
	T_Inner_MarketInfo			tagMkInfo;
	CriticalLock				lock( m_oLock );
	MStreamWrite				oMSW( pszInBuf, nInBytes );
	BigTableDatabase&			refDatabase = EngineWrapper4DataClient::GetObj().GetDatabaseObj();
	unsigned int				MsgType = 0;
	unsigned int				MsgSize = 0;
	int							nDataSize = 0;

	nCount = 0;
	if( nInnerMkID < 0 || NULL == m_pQueryBuffer )
	{
		return -1;
	}

	if( XDF_SH == cMarket || XDF_SZ == cMarket )
	{
		MsgType = 5;
		MsgSize = sizeof(XDFAPI_NameTableSh);
	}
	else if( XDF_CF == cMarket )//中金期货
	{
		MsgType = 4;
		MsgSize = sizeof(XDFAPI_NameTableZjqh);
	}
	else if( XDF_CNF == cMarket )//商品期货(上海/郑州/大连)
	{
		MsgType = 7;
		MsgSize = sizeof(XDFAPI_NameTableCnf);
	}
	else if( XDF_SHOPT == cMarket )//上证期权
	{
		MsgType = 2;
		MsgSize = sizeof(XDFAPI_NameTableShOpt);
	}
	else if( XDF_ZJOPT == cMarket )//中金期权
	{
		MsgType = 3;
		MsgSize = sizeof(XDFAPI_NameTableZjOpt);
	}
	else if( XDF_SZOPT == cMarket )//深圳期权
	{
		MsgType = 9;
		MsgSize = sizeof(XDFAPI_NameTableSzOpt);
	}
	else if( XDF_CNFOPT == cMarket )//商品期权(上海/郑州/大连)
	{
		MsgType = 11;
		MsgSize = sizeof(XDFAPI_NameTableCnfOpt);
	}
	else
	{
		return -3;
	}

	for( int i = 0; i < 3; i++ )
	{
		nSerialNo = 0;
		::memset( &tagMkInfo, 0, sizeof(T_Inner_MarketInfo) );
		if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(T_Inner_MarketInfo), nSerialNo ) <= 0 )
		{
			if( XDF_CNF == cMarket || XDF_CNFOPT ==  cMarket )
			{
				if( XDF_CNF == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
				}

				if( XDF_CNFOPT == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
				}
				continue;
			}

			return -2;
		}

		nCount += tagMkInfo.objData.uiWareCount;

		if( XDF_CNF != cMarket && XDF_CNFOPT != cMarket )
		{
			if( 0 == pszInBuf || 0 == nInBytes )		///< 通过nCount返回码表个数
			{
				return 1;
			}
		}
		else
		{
			if( 0 == pszInBuf || 0 == nInBytes )		///< 通过nCount返回码表个数
			{
				if( XDF_CNF == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
				}

				if( XDF_CNFOPT == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
				}

				continue;
			}
		}

		tagQUO_ReferenceData*		pRefData = (tagQUO_ReferenceData*)m_pQueryBuffer;
		nSerialNo = 0;
		if( (nDataSize=refDatabase.QueryBatchRecords( (nInnerMkID*100+2), m_pQueryBuffer, MAX_QUERY_BUF_SIZE, nSerialNo )) <= 0 )
		{
			return -4;
		}

		for( unsigned int nOffset = 0; nOffset < nDataSize; nOffset+=sizeof(tagQUO_ReferenceData) )
		{
			XDFAPI_NameTableCnf			tagCnfName = { 0 };
			XDFAPI_NameTableSh			tagShL1Name = { 0 };
			XDFAPI_NameTableZjqh		tagZJQHName = { 0 };
			XDFAPI_NameTableShOpt		tagSHOPTName = { 0 };
			XDFAPI_NameTableZjOpt		tagZJOPTName = { 0 };
			XDFAPI_NameTableSzOpt		tagSZOPTName = { 0 };
			XDFAPI_NameTableCnfOpt		tagCnfOPTName = { 0 };
			char						*pData = NULL;

			if( XDF_SH == cMarket || XDF_SZ == cMarket )
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagShL1Name;
				tagShL1Name.Market = XDF_SH;
				memcpy( tagShL1Name.Code, pRefData->szCode, 6 );
				memcpy( tagShL1Name.Name, pRefData->szName, 8 );
				if( XDF_SZ == cMarket )
				{
					tagShL1Name.SecKind = CastKindID4SZL1( pRefData->szCode );
				}
				else
				{
					tagShL1Name.SecKind = CastKindID4SHL1( pRefData->szCode );
				}
			}
			else if( XDF_CF == cMarket )//中金期货
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJQHName;
				tagZJQHName.Market = XDF_CF;
				memcpy( tagZJQHName.Code, pRefData->szCode, 6 );
				tagZJQHName.SecKind = CastKindID4CFF( pRefData->szCode );
				tagZJQHName.ContractMult = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiContractMult;	///< 合约乘数
				//tagZJQHName.ExFlag = pRefData->ExFlag;			///< 最后交易日标记,0x01表示是最后交易日只对普通合约有效；其他值暂未定义
				strncpy(tagZJQHName.Name, pRefData->szName, 8);
				//tagZJQHName->ObjectMId = pRefData->ObjectMId;		///< 标的指数市场编号[参见数据字典-市场编号]，0xFF表示未知
				strncpy(tagZJQHName.ObjectCode, tagMkInfo.objData.mKindRecord[pRefData->uiKindID].szUnderlyingCode,6);
			}
			else if( XDF_CNF == cMarket )//商品期货(上海/郑州/大连)
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfName;
				memcpy( tagCnfName.Code,pRefData->szCode, sizeof(tagCnfName.Code) );
				memcpy( tagCnfName.Name, pRefData->szName, sizeof(tagCnfName.Name) );
				tagCnfName.Market = XDF_CNF;
				tagCnfName.SecKind = CastKindID4CNF( nInnerMkID, tagCnfName.Code );
				tagCnfName.LotFactor = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiLotFactor;
			}
			else if( XDF_SHOPT == cMarket )//上证期权
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagSHOPTName;
				tagSHOPTName.Market = XDF_SHOPT;
				memcpy(tagSHOPTName.Code, pRefData->szCode, 8);
				memcpy(tagSHOPTName.Name, pRefData->szName,20);
				tagSHOPTName.SecKind = CastKindID4SHOPT( pRefData->szCode );
				memcpy(tagSHOPTName.ContractID, pRefData->szContractID, 19);
				tagSHOPTName.OptionType = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].cOptionType;
				tagSHOPTName.CallOrPut = pRefData->cCallOrPut;

				//tagSHOPTName.PreClosePx = pRefData->C;//合约昨收(如遇除权除息则为调整后的收盘价格)(精确到厘)//[*放大倍数]
				//tagSHOPTName.PreSettlePx = pRefData->;//合约昨结//[*放大倍数]
				//tagSHOPTName.LeavesQty = pRefData->;//未平仓合约数 = 昨持仓 单位是(张)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.objData.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//memcpy(tagSHOPTName.UnderlyingName, pRefData-, 6);//标的证券名称
				//memcpy(tagSHOPTName.UnderlyingType, pRefData-, 3);//标的证券类型(EBS -ETF, ASH -A股)
				//tagSHOPTName.UnderlyingClosePx = ;//标的证券的昨收 //[*放大倍数]
				//tagSHOPTName.PriceLimitType = pRefData-//涨跌幅限制类型(N 有涨跌幅)(R 无涨跌幅)
				//tagSHOPTName.UpLimit = pRefData->;//当日期权涨停价格(精确到厘) //[*放大倍数]
				//tagSHOPTName.DownLimit;//当日期权跌停价格(精确到厘) //[*放大倍数]
				tagSHOPTName.LotSize = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiLotSize;//一手等于几张合约
				tagSHOPTName.ContractUnit = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiContractUnit;//合约单位(经过除权除息调整后的合约单位，一定为整数)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice * GetRate( XDF_SHOPT, tagSHOPTName.SecKind ) + 0.5;//行权价格(精确到厘) //[*放大倍数] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//首个交易日(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//最后交易日(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//行权日(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//交割日(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->uiExpireDate;//到期日(YYYYMMDD)
				//tagSHOPTName.UpdateVersion = pRefData->;//期权合约的版本号(新挂合约是'1')
//		unsigned long					MarginUnit;			//单位保证金(精确到分)//[*放大100]
//		short							MarginRatio;		//保证金比例1(%)
//		short							MarginRatio2;		//保证金比例2(%)
//		int								MinMktFloor;		//单笔市价申报下限
//		int								MaxMktFloor;		//单笔市价申报上限
//		int								MinLmtFloor;		//单笔限价申报下限
//		int								MaxLmtFloor;		//单笔限价申报上限
//		char							StatusFlag[8];		//期权合约状态(8个字符,详细定义见文档)
			}
			else if( XDF_ZJOPT == cMarket )//中金期权
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJOPTName;
				tagZJOPTName.Market = XDF_ZJOPT;
				memcpy(tagZJOPTName.Code, pRefData->szCode, 32);
				tagZJOPTName.SecKind = CastKindID4CFFOPT( pRefData->szCode );
				tagZJOPTName.ContractMult = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiContractMult;	//合约乘数
				tagZJOPTName.ContractUnit = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiContractUnit;	//合约单位
				tagZJOPTName.StartDate = pRefData->uiStartDate;		//首个交易日(YYYYMMDD)
				tagZJOPTName.EndDate = pRefData->uiEndDate;				//最后交易日(YYYYMMDD)
				tagZJOPTName.XqDate = pRefData->uiExerciseDate;				//行权日(YYYYMMDD)
				tagZJOPTName.DeliveryDate = pRefData->uiDeliveryDate;	//交割日(YYYYMMDD)
				tagZJOPTName.ExpireDate = pRefData->uiExpireDate;		//到期日(YYYYMMDD)
				//pTable->ObjectMId = pNameTb->ObjectMId;
				strncpy( tagZJOPTName.ObjectCode, tagMkInfo.objData.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6 );
			}
			else if( XDF_SZOPT == cMarket )//深圳期权
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagSZOPTName;
				tagSZOPTName.Market = XDF_SZOPT;
				tagSZOPTName.SecKind = CastKindID4SZOPT( pRefData->szCode );
				memcpy(tagSZOPTName.Code, pRefData->szCode, 8);
				memcpy(tagSZOPTName.Name, pRefData->szName,20);
				memcpy(tagSZOPTName.ContractID, tagMkInfo.objData.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 20);
				tagSZOPTName.OptionType = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].cOptionType;
				tagSZOPTName.CallOrPut = pRefData->cCallOrPut;
				//tagSZOPTName.PreClosePx = pRefData->C;//合约昨收(如遇除权除息则为调整后的收盘价格)(精确到厘)//[*放大倍数]
				//tagSZOPTName.PreSettlePx = pRefData->;//合约昨结//[*放大倍数]
				//tagSZOPTName.LeavesQty = pRefData->;//未平仓合约数 = 昨持仓 单位是(张)
				memcpy(tagSZOPTName.UnderlyingCode, tagMkInfo.objData.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//tagSZOPTName.UpLimit = pRefData->;//当日期权涨停价格(精确到厘) //[*放大倍数]
				//tagSZOPTName.DownLimit;//当日期权跌停价格(精确到厘) //[*放大倍数]
				tagSZOPTName.LotSize = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiLotSize;//一手等于几张合约
				tagSZOPTName.ContractUnit = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiContractUnit;//合约单位(经过除权除息调整后的合约单位，一定为整数)
				tagSZOPTName.XqPrice = pRefData->dExercisePrice * GetRate( XDF_SZOPT, tagSZOPTName.SecKind ) + 0.5;//行权价格(精确到厘) //[*放大倍数] 
				tagSZOPTName.StartDate = pRefData->uiStartDate;//首个交易日(YYYYMMDD)
				tagSZOPTName.EndDate = pRefData->uiEndDate;//最后交易日(YYYYMMDD)
				tagSZOPTName.XqDate = pRefData->uiExerciseDate;//行权日(YYYYMMDD)
				tagSZOPTName.DeliveryDate = pRefData->uiDeliveryDate;//交割日(YYYYMMDD)
				tagSZOPTName.ExpireDate = pRefData->uiExpireDate;//到期日(YYYYMMDD)
				//tagSHOPTName.MarginUnit = pNameTb->MarginUnit;//单位保证金(精确到分)//[*放大100]
			}
			else if( XDF_CNFOPT == cMarket )//商品期货和商品期权(上海/郑州/大连)
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfOPTName;
				tagCnfOPTName.Market = XDF_CNFOPT;
				tagCnfOPTName.SecKind = CastKindID4CNFOPT( nInnerMkID, NULL );
				memcpy(tagCnfOPTName.Code,pRefData->szCode, 20);
				memcpy(tagCnfOPTName.Name, pRefData->szName, 40);
				tagCnfOPTName.LotFactor = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiLotFactor;
				//tagCnfOPTName.LeavesQty = pRefData->;//未平仓合约数 = 昨持仓 单位是(张)
				memcpy(tagCnfOPTName.UnderlyingCode, tagMkInfo.objData.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//pTable->PriceLimitType = pNameTb->PriceLimitType;
				//tagCnfOPTName.UpLimit = pRefData->;//当日期权涨停价格(精确到厘) //[*放大倍数]
				//tagCnfOPTName.DownLimit;//当日期权跌停价格(精确到厘) //[*放大倍数]
				tagCnfOPTName.LotSize = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiLotSize;//一手等于几张合约
				tagCnfOPTName.ContractMult = tagMkInfo.objData.mKindRecord[pRefData->uiKindID].uiContractMult;//合约单位(经过除权除息调整后的合约单位，一定为整数)
				tagCnfOPTName.XqPrice = pRefData->dExercisePrice * GetRate( XDF_CNFOPT, tagCnfOPTName.SecKind ) + 0.5;//行权价格(精确到厘) //[*放大倍数] 
				tagCnfOPTName.StartDate = pRefData->uiStartDate;//首个交易日(YYYYMMDD)
				tagCnfOPTName.EndDate = pRefData->uiEndDate;//最后交易日(YYYYMMDD)
				tagCnfOPTName.XqDate = pRefData->uiExerciseDate;//行权日(YYYYMMDD)
				tagCnfOPTName.DeliveryDate = pRefData->uiDeliveryDate;//交割日(YYYYMMDD)
	//			tagSHOPTName.TypePeriodIndx = pNameTb->TypePeriodIndx;
	//			tagSHOPTName.EarlyNightFlag = pNameTb->EarlyNightFlag;
			}

			if( pData != NULL )
			{
				oMSW.PutMsg( MsgType, pData, MsgSize );
			}
		}

		if( XDF_CNF != cMarket && XDF_CNFOPT != cMarket )
		{
			break;
		}

		if( XDF_CNF == cMarket )
		{
			if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
			if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
		}

		if( XDF_CNFOPT == cMarket )
		{
			if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
			if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
		}
	}

	oMSW.Detach();

	if( 0 == pszInBuf || 0 == nInBytes )		///< 通过nCount返回码表个数
	{
		return 1;
	}

	return oMSW.GetOffset();
}

int STDCALL		MDataClient::GetLastMarketDataAll(unsigned char cMarket, char* pszInBuf, int nInBytes)
{
	int							nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64			nSerialNo = 0;
	CriticalLock				lock( m_oLock );
	MStreamWrite				oMSW( pszInBuf, nInBytes );
	BigTableDatabase&			refDatabase = EngineWrapper4DataClient::GetObj().GetDatabaseObj();
	unsigned int				MsgType = 0;
	unsigned int				MsgSize = 0;
	int							nDataSize = 0;

	if( nInnerMkID < 0 || NULL == m_pQueryBuffer )
	{
		return -1;
	}

	if( XDF_SH == cMarket || XDF_SZ == cMarket )
	{
		MsgType = 22;
		MsgSize = sizeof(XDFAPI_StockData5);
	}
	else if( XDF_CF == cMarket )//中金期货
	{
		MsgType = 20;
		MsgSize = sizeof(XDFAPI_CffexData);
	}
	else if( XDF_CNF == cMarket )//商品期货(上海/郑州/大连)
	{
		MsgType = 26;
		MsgSize = sizeof(XDFAPI_CNFutureData);
	}
	else if( XDF_SHOPT == cMarket )//上证期权
	{
		MsgType = 15;
		MsgSize = sizeof(XDFAPI_ShOptData);
	}
	else if( XDF_ZJOPT == cMarket )//中金期权
	{
		MsgType = 18;
		MsgSize = sizeof(XDFAPI_ZjOptData);
	}
	else if( XDF_SZOPT == cMarket )//深圳期权
	{
		MsgType = 35;
		MsgSize = sizeof(XDFAPI_SzOptData);
	}
	else if( XDF_CNFOPT == cMarket )//商品期货和商品期权(上海/郑州/大连)
	{
		MsgType = 34;
		MsgSize = sizeof(XDFAPI_CNFutOptData);
	}
	else
	{
		return -3;
	}

	for( int i = 0; i < 3; i++ )
	{
		tagQUO_SnapData*			pSnapData = (tagQUO_SnapData*)m_pQueryBuffer;
		nSerialNo = 0;

		if( (nDataSize=refDatabase.QueryBatchRecords( (nInnerMkID*100+3), m_pQueryBuffer, MAX_QUERY_BUF_SIZE, nSerialNo )) <= 0 )
		{
			if( XDF_CNF == cMarket || XDF_CNFOPT ==  cMarket )
			{
				if( XDF_CNF == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
				}

				if( XDF_CNFOPT == cMarket )
				{
					if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
					if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
				}
				continue;
			}

			return -4;
		}

		for( unsigned int nOffset = 0; nOffset < nDataSize; nOffset+=sizeof(tagQUO_SnapData) )
		{
			XDFAPI_CNFutOptData			tagCNFOPTStock = { 0 };
			XDFAPI_SzOptData			tagSZOPTStock = { 0 };
			XDFAPI_ZjOptData			tagZJOPTStock = { 0 };
			XDFAPI_StockData5			tagSHL1Stock = { 0 };
			XDFAPI_CffexData			tagCFFStock = { 0 };
			XDFAPI_CNFutureData			tagCNFStock = { 0 };
			XDFAPI_ShOptData			tagSHOPTStock = { 0 };
			char						*pData = NULL;

			if( XDF_SH == cMarket || XDF_SZ == cMarket )
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSHL1Stock;

				memcpy(tagSHL1Stock.Code, pSnapData->szCode,6);
				int		nKind = 0;
				if( XDF_SH == cMarket )
				{
					nKind = CastKindID4SHL1( tagSHL1Stock.Code );
				}
				else
				{
					nKind = CastKindID4SZL1( tagSHL1Stock.Code );
				}
				tagSHL1Stock.High = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHL1Stock.Open = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHL1Stock.Low = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHL1Stock.PreClose = pSnapData->dPreClosePx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHL1Stock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHL1Stock.Amount = pSnapData->dAmount;
				tagSHL1Stock.Volume = pSnapData->ui64Volume;
				//tagSHL1Stock.Records = pSnapData->Records;
				//tagSHL1Stock.HighLimit = pSnapData->HighLimit;
				//tagSHL1Stock.LowLimit = pSnapData->LowLimit;
				//tagSHL1Stock.Voip = pSnapData->Voip;

				for (int i=0; i<5; i++)
				{
					tagSHL1Stock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagSHL1Stock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagSHL1Stock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagSHL1Stock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CF == cMarket )//中金期货
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCFFStock;

				memcpy(tagCFFStock.Code, pSnapData->szCode,6);
				int		nKind = CastKindID4CFF( tagCFFStock.Code );
				tagCFFStock.High = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;						//最高价格[* 放大倍数]
				tagCFFStock.Open = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
				tagCFFStock.Low = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;						//最低价格[* 放大倍数]
				tagCFFStock.PreClose = pSnapData->dPreClosePx * GetRate( cMarket, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
				tagCFFStock.PreSettlePrice = pSnapData->dPreSettlePx * GetRate( cMarket, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
				tagCFFStock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
				tagCFFStock.Close = pSnapData->dClosePx * GetRate( cMarket, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
				tagCFFStock.SettlePrice	= pSnapData->dSettlePx * GetRate( cMarket, nKind  ) + 0.5;      //今日结算价格[* 放大倍数]
				tagCFFStock.UpperPrice = pSnapData->dUpperLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
				tagCFFStock.LowerPrice = pSnapData->dLowerLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
				tagCFFStock.Amount = pSnapData->dAmount;             //总成交金额[元]
				tagCFFStock.Volume = pSnapData->ui64Volume;             //总成交量[股]
				tagCFFStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagCFFStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]
				tagCFFStock.DataTimeStamp = pSnapData->uiTime;

				for (int i=0; i<5; i++)
				{
					tagCFFStock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagCFFStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCFFStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagCFFStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CNF == cMarket )//商品期货(上海/郑州/大连)
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFStock;

				memcpy(tagCNFStock.Code, pSnapData->szCode,20);
				int		nKind = CastKindID4CNF( nInnerMkID, tagCNFStock.Code );
				tagCNFStock.High = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;						//最高价格[* 放大倍数]
				tagCNFStock.Open = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
				tagCNFStock.Low = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;						//最低价格[* 放大倍数]
				tagCNFStock.PreClose = pSnapData->dPreClosePx * GetRate( cMarket, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
				tagCNFStock.PreSettlePrice = pSnapData->dPreSettlePx * GetRate( cMarket, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
				tagCNFStock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
				tagCNFStock.Close = pSnapData->dClosePx * GetRate( cMarket, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
				tagCNFStock.SettlePrice	 = pSnapData->dSettlePx * GetRate( cMarket, nKind  ) + 0.5;        //今日结算价格[* 放大倍数]
				tagCNFStock.UpperPrice	 = pSnapData->dUpperLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
				tagCNFStock.LowerPrice	 = pSnapData->dLowerLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
				tagCNFStock.Amount		 = pSnapData->dAmount;             //总成交金额[元]
				tagCNFStock.Volume		 = pSnapData->ui64Volume;             //总成交量[股]
				tagCNFStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagCNFStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]
				tagCNFStock.DataTimeStamp = pSnapData->uiTime;
				tagCNFStock.Date = s_Date4CNF % 10000;

				for (int i=0; i<5; i++)
				{
					tagCNFStock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagCNFStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCNFStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagCNFStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_SHOPT == cMarket )//上证期权
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSHOPTStock;

				memcpy(tagSHOPTStock.Code, pSnapData->szCode,8);
				int		nKind = CastKindID4SHOPT( tagSHOPTStock.Code );
				tagSHOPTStock.PreSettlePx = pSnapData->dPreSettlePx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHOPTStock.SettlePrice = pSnapData->dSettlePx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHOPTStock.OpenPx = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHOPTStock.HighPx = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHOPTStock.LowPx = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHOPTStock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSHOPTStock.Volume = pSnapData->ui64Volume;
				tagSHOPTStock.Amount = pSnapData->dAmount;
				memcpy(tagSHOPTStock.TradingPhaseCode, pSnapData->szTradingPhaseCode,4);
				//tagSHOPTStock.AuctionPrice = pSnapData->AuctionPrice;
				//tagSHOPTStock.AuctionQty = pSnapData->AuctionQty;
				tagSHOPTStock.Position = pSnapData->ui64OpenInterest;
				tagSHOPTStock.Time = pSnapData->uiTime;

				for (int i=0; i<5; i++)
				{
					tagSHOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagSHOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagSHOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagSHOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_ZJOPT == cMarket )//中金期权
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagZJOPTStock;

				memcpy(tagZJOPTStock.Code, pSnapData->szCode,32);
				int		nKind = CastKindID4CFFOPT( tagZJOPTStock.Code );
				tagZJOPTStock.High = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;						//最高价格[* 放大倍数]
				tagZJOPTStock.Open = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
				tagZJOPTStock.Low = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;						//最低价格[* 放大倍数]
				tagZJOPTStock.PreClose = pSnapData->dPreClosePx * GetRate( cMarket, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
				tagZJOPTStock.PreSettlePrice = pSnapData->dPreSettlePx * GetRate( cMarket, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]

				tagZJOPTStock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
				tagZJOPTStock.Close	= pSnapData->dClosePx * GetRate( cMarket, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
				tagZJOPTStock.SettlePrice = pSnapData->dSettlePx * GetRate( cMarket, nKind  ) + 0.5;        //今日结算价格[* 放大倍数]
				tagZJOPTStock.UpperPrice = pSnapData->dUpperLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
				tagZJOPTStock.LowerPrice = pSnapData->dLowerLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
				tagZJOPTStock.Amount = pSnapData->dAmount;             //总成交金额[元]
				tagZJOPTStock.Volume = pSnapData->ui64Volume;             //总成交量[股]
				tagZJOPTStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagZJOPTStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]
				tagZJOPTStock.DataTimeStamp = pSnapData->uiTime;

				for (int i=0; i<5; i++)
				{
					tagZJOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagZJOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagZJOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagZJOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_SZOPT == cMarket )//深圳期权
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSZOPTStock;

				memcpy(tagSZOPTStock.Code, pSnapData->szCode,8);
				int		nKind = CastKindID4SZOPT( tagSZOPTStock.Code );
				tagSZOPTStock.PreSettlePx = pSnapData->dPreSettlePx * GetRate( cMarket, nKind  ) + 0.5;
				tagSZOPTStock.SettlePrice = pSnapData->dSettlePx * GetRate( cMarket, nKind  ) + 0.5;
				tagSZOPTStock.OpenPx = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSZOPTStock.HighPx = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSZOPTStock.LowPx = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSZOPTStock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;
				tagSZOPTStock.Volume = pSnapData->ui64Volume;
				tagSZOPTStock.Amount = pSnapData->dAmount;
				memcpy(tagSZOPTStock.TradingPhaseCode, pSnapData->szTradingPhaseCode,4);
				tagSZOPTStock.Position = pSnapData->ui64OpenInterest;
				tagSZOPTStock.Time = pSnapData->uiTime;

				for (int i=0; i<5; i++)
				{
					tagSZOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagSZOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagSZOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagSZOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CNFOPT == cMarket )//商品期货和商品期权(上海/郑州/大连)
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFOPTStock;

				memcpy(tagCNFOPTStock.Code, pSnapData->szCode,20);
				int		nKind = CastKindID4CNFOPT( nInnerMkID, tagCNFStock.Code );
				tagCNFOPTStock.High = pSnapData->dHighPx * GetRate( cMarket, nKind  ) + 0.5;						//最高价格[* 放大倍数]
				tagCNFOPTStock.Open = pSnapData->dOpenPx * GetRate( cMarket, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
				tagCNFOPTStock.Low = pSnapData->dLowPx * GetRate( cMarket, nKind  ) + 0.5;						//最低价格[* 放大倍数]
				tagCNFOPTStock.PreClose = pSnapData->dPreClosePx * GetRate( cMarket, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
				tagCNFOPTStock.PreSettlePrice = pSnapData->dPreSettlePx * GetRate( cMarket, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
				tagCNFOPTStock.Now = pSnapData->dNowPx * GetRate( cMarket, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
				tagCNFOPTStock.Close = pSnapData->dClosePx * GetRate( cMarket, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
				tagCNFOPTStock.SettlePrice = pSnapData->dSettlePx * GetRate( cMarket, nKind  ) + 0.5;        //今日结算价格[* 放大倍数]
				tagCNFOPTStock.UpperPrice = pSnapData->dUpperLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
				tagCNFOPTStock.LowerPrice = pSnapData->dLowerLimitPx * GetRate( cMarket, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
				tagCNFOPTStock.Amount = pSnapData->dAmount;             //总成交金额[元]
				tagCNFOPTStock.Volume = pSnapData->ui64Volume;             //总成交量[股]
				tagCNFOPTStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagCNFOPTStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]
				tagCNFOPTStock.DataTimeStamp = pSnapData->uiTime;
				tagCNFOPTStock.Date = s_Date4CNFOPT % 10000;

				for (int i=0; i<5; i++)
				{
					tagCNFOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagCNFOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCNFOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice * GetRate( cMarket, nKind  ) + 0.5;
					tagCNFOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}

			if( NULL != pData )
			{
				oMSW.PutMsg( MsgType, pData, MsgSize );
			}
		}

		if( XDF_CNF != cMarket && XDF_CNFOPT != cMarket )
		{
			break;
		}

		if( XDF_CNF == cMarket )
		{
			if( 0 == i )	nInnerMkID = QUO_MARKET_CZCE;
			if( 1 == i )	nInnerMkID = QUO_MARKET_SHFE;
		}

		if( XDF_CNFOPT == cMarket )
		{
			if( 0 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
			if( 1 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
		}
	}

	oMSW.Detach();

	return oMSW.GetOffset();
}

int STDCALL		MDataClient::GetMarketStatus( unsigned char cMarket,int& nStatus, unsigned int& ulTime, __int64 * pI64Send, __int64 * pI64Recv )
{
	char					pszStatus[1024] = { 0 };
	unsigned int			nStatusBufLen = sizeof(pszStatus);
	DataCollectorPool&		refPool = DataIOEngine::GetEngineObj().GetCollectorPool();
	DataCollector*			pDataCollector = refPool.GetCollectorByMkID( cMarket );

	if( NULL == pDataCollector )
	{
		nStatus = XRS_None;
		ulTime = 0;
		return -1;
	}

	enum E_SS_Status		eStatus = pDataCollector->InquireDataCollectorStatus( pszStatus, nStatusBufLen );

	if( ET_SS_WORKING == eStatus )
	{
		nStatus = XRS_Normal;
	}
	else if( eStatus >= ET_SS_CONNECTED && eStatus < ET_SS_WORKING )
	{
		nStatus = XRS_Init;
	}
	else
	{
		nStatus = XRS_Unknow;
	}

	return 0;
}

//....................................................................................................................................................................................................................................................................
//....................................................................................................................................................................................................................................................................
//....................................................................................................................................................................................................................................................................
//....................................................................................................................................................................................................................................................................
MPrimeClient::MPrimeClient()
{
}

MPrimeClient::~MPrimeClient()
{
}

int		STDCALL		MPrimeClient::ReqFuncData(int FuncNo, void* wParam, void* lParam)
{
	if( FuncNo == 100 || FuncNo == 101 )		//获取某个市场的市场日期和市场时间(参数:uint8*,   XDFAPI_MarketStatusInfo*)
	{
		unsigned char* pMarket = (unsigned char*)wParam;
		XDFAPI_MarketStatusInfo* pInfo = (XDFAPI_MarketStatusInfo*)lParam;
		if (pMarket && pInfo)
		{
			unsigned char cMarket = *pMarket;
			XDFAPI_MarketStatusInfo oInfo;
			tagQUO_MarketInfo	tagMarketInfo;
			QUO_MARKET_ID		eMarketID = (QUO_MARKET_ID)DataCollectorPool::MkIDCast( cMarket );

			if( GetMarketInfo( eMarketID, &tagMarketInfo ) >= 0 )
			{
				oInfo.MarketID = cMarket;
				oInfo.MarketDate = tagMarketInfo.uiMarketDate;
				oInfo.MarketTime = tagMarketInfo.uiMarketTime;
				oInfo.MarketStatus = 1;

				*pInfo = oInfo;
				return 1;
			}
		}
	}

	return -1;
}

void QuotationAdaptor::OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{
	if( NULL == Global_pSpi )
	{
		return;
	}

	char			outbuf[81920]={0};
	MStreamWrite	oMSW(outbuf, 81920);
	unsigned int	nMsgID = nMessageID % 100;
	unsigned int	nOldMkID = DataCollectorPool::Cast2OldMkID( eMarketID );

	switch( nOldMkID )
	{
	case XDF_SZ:
	case XDF_SH:
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.MarketID = nOldMkID;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
		}
		else if( nMsgID == 2 )
		{
			tagQUO_ReferenceData*		pData = (tagQUO_ReferenceData*)pDataPtr;
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_StockData5			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,6);
			int		nKind = 0;
			if( XDF_SH == nOldMkID )
			{
				nKind = CastKindID4SHL1( tagData.Code );
			}
			else
			{
				nKind = CastKindID4SZL1( tagData.Code );
			}
			tagData.High = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Open = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Low = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.PreClose = pData->dPreClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Amount = pData->dAmount;
			tagData.Volume = pData->ui64Volume;
			//tagSHL1Stock.Records = pData->Records;
			//tagSHL1Stock.HighLimit = pData->HighLimit;
			//tagSHL1Stock.LowLimit = pData->LowLimit;
			//tagSHL1Stock.Voip = pData->Voip;

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(22, (char*)&tagData,sizeof(XDFAPI_StockData5) );
		}
		break;
	case XDF_CF://中金期货
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.MarketID = nOldMkID;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_CffexData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,6);
			int		nKind = CastKindID4CFF( tagData.Code );
			tagData.High = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
			tagData.Close = pData->dClosePx;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice	= pData->dSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;      //今日结算价格[* 放大倍数]
			tagData.UpperPrice = pData->dUpperLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
			tagData.LowerPrice = pData->dLowerLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
			tagData.Amount = pData->dAmount;             //总成交金额[元]
			tagData.Volume = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]
			tagData.DataTimeStamp = pData->uiTime;

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(20, (char*)&tagData,sizeof(XDFAPI_CffexData) );
		}
		break;
	case XDF_CNF://商品期货(上海/郑州/大连)
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.MarketID = nOldMkID;
			tagData.MarketStatus = 1;
			s_Date4CNF = tagData.MarketDate;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_CNFutureData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,20);
			int		nKind = CastKindID4CNF( eMarketID, tagData.Code );
			tagData.High = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
			tagData.Close = pData->dClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice	 = pData->dSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;        //今日结算价格[* 放大倍数]
			tagData.UpperPrice	 = pData->dUpperLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
			tagData.LowerPrice	 = pData->dLowerLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
			tagData.Amount		 = pData->dAmount;             //总成交金额[元]
			tagData.Volume		 = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]
			tagData.DataTimeStamp = pData->uiTime;
			tagData.Date = s_Date4CNF % 10000;

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(26, (char*)&tagData, sizeof(XDFAPI_CNFutureData) );
		}
		break;
	case XDF_SHOPT://上证期权
		if( nMsgID == 1 )
		{
			XDFAPI_ShOptMarketStatus	tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.TradingPhaseCode[0] = '1';

			oMSW.PutSingleMsg(14, (char*)&tagData, sizeof(XDFAPI_ShOptMarketStatus));
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_ShOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,8);
			int		nKind = CastKindID4SHOPT( tagData.Code );
			tagData.Time = pData->uiTime;
			tagData.PreSettlePx = pData->dPreSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.SettlePrice = pData->dSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.OpenPx = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.HighPx = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.LowPx = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Volume = pData->ui64Volume;
			tagData.Amount = pData->dAmount;
			memcpy(tagData.TradingPhaseCode, pData->szTradingPhaseCode,4);
			//tagData.AuctionPrice = pData->AuctionPrice;
			//tagData.AuctionQty = pData->AuctionQty;
			tagData.Position = pData->ui64OpenInterest;
			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(15, (char*)&tagData, sizeof(XDFAPI_ShOptData));
		}
		break;
	case XDF_ZJOPT://中金期权
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.MarketID = nOldMkID;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_ZjOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,32);
			int		nKind = CastKindID4CFFOPT( tagData.Code );
			tagData.High = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
			
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
			tagData.Close	= pData->dClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice = pData->dSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;        //今日结算价格[* 放大倍数]
			tagData.UpperPrice = pData->dUpperLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
			tagData.LowerPrice = pData->dLowerLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
			tagData.Amount = pData->dAmount;             //总成交金额[元]
			tagData.Volume = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]
			tagData.DataTimeStamp = pData->uiTime;

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(18, (char*)&tagData, sizeof(XDFAPI_ZjOptData) );
		}
		break;
	case XDF_SZOPT://深圳期权
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.MarketID = nOldMkID;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_SzOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,8);
			int		nKind = CastKindID4SZOPT( tagData.Code );
			tagData.Time = pData->uiTime;
			tagData.PreSettlePx = pData->dPreSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.SettlePrice = pData->dSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.OpenPx = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.HighPx = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.LowPx = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
			tagData.Volume = pData->ui64Volume;
			tagData.Amount = pData->dAmount;
			memcpy(tagData.TradingPhaseCode, pData->szTradingPhaseCode,4);
			tagData.Position = pData->ui64OpenInterest;
			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(35, (char*)&tagData, sizeof(XDFAPI_SzOptData));
		}
		break;
	case XDF_CNFOPT://商品期权(上海/郑州/大连)
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			T_Inner_MarketInfo*			pData = (T_Inner_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->objData.uiMarketDate;
			tagData.MarketTime = pData->objData.uiMarketTime;
			tagData.MarketID = nOldMkID;
			tagData.MarketStatus = 1;
			s_Date4CNFOPT = tagData.MarketDate;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
		}
		else if( nMsgID == 3 )
		{
			XDFAPI_CNFutOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,20);
			int		nKind = CastKindID4CNFOPT( eMarketID, tagData.Code );
			tagData.High = pData->dHighPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;	//昨日结算价格[* 放大倍数]
			tagData.Now = pData->dNowPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                    //最新价格[* 放大倍数]
			tagData.Close = pData->dClosePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice = pData->dSettlePx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;        //今日结算价格[* 放大倍数]
			tagData.UpperPrice = pData->dUpperLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //涨停价格[* 放大倍数]
			tagData.LowerPrice = pData->dLowerLimitPx * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;         //跌停价格[* 放大倍数]
			tagData.Amount = pData->dAmount;             //总成交金额[元]
			tagData.Volume = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]
			tagData.DataTimeStamp = pData->uiTime;
			tagData.Date = s_Date4CNFOPT % 10000;

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice * MDataClient::GetRate( nOldMkID, nKind  ) + 0.5;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(34, (char*)&tagData, sizeof(XDFAPI_CNFutOptData) );
		}
		break;
	}

	oMSW.Detach();

	//[3]放入池子(PkgHead+PkgBuf)
	XDFAPI_PkgHead pkghead;
	memset(&pkghead, 0, sizeof(XDFAPI_PkgHead));
	pkghead.MarketID = nOldMkID;
	pkghead.PkgSize = oMSW.GetOffset();

	g_oDataIO.PutData( &pkghead, outbuf, oMSW.GetOffset() );
}

void QuotationAdaptor::OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus )
{
	if( Global_pSpi )
	{
		char			pszInfo[128] = { 0 };
		XDFRunStat		eStatus = XRS_None;
		unsigned char	cMkID = DataCollectorPool::Cast2OldMkID(eMarketID);

		if( QUO_STATUS_INIT == eMarketStatus )
		{
			eStatus = XRS_Init; 
		}
		else if( QUO_STATUS_NORMAL == eMarketStatus )
		{
			eStatus = XRS_Normal;
		}
		else
		{
			eStatus = XRS_Unknow;
		}

		///< ------------------------- 秘技: 只在全市场初始化完成时调用 ---------------------------------------
		static std::map<short,enum XDFRunStat>		s_mapMkStatus;								///< 商品期货/期权各市场状态
		if( cMkID == XDF_CNFOPT || cMkID == XDF_CNF )											///< 对商品期货、权部分的状态先进行缓存
		{
			s_mapMkStatus[(short)eMarketID] = eStatus;
		}

		if( cMkID == XDF_CNFOPT || cMkID == XDF_CNF )											///< 秘技,检查状态缓存，判断是否需要通知状态变化
		{
			bool							bNotifyCNF = true;
			bool							bNotifyCNFOPT = true;
			std::map<short,enum XDFRunStat>::iterator itDL = s_mapMkStatus.find( QUO_MARKET_DCE );
			std::map<short,enum XDFRunStat>::iterator itSH = s_mapMkStatus.find( QUO_MARKET_SHFE );
			std::map<short,enum XDFRunStat>::iterator itZZ = s_mapMkStatus.find( QUO_MARKET_CZCE );
			std::map<short,enum XDFRunStat>::iterator itDLOPT = s_mapMkStatus.find( QUO_MARKET_DCEOPT );
			std::map<short,enum XDFRunStat>::iterator itSHOPT = s_mapMkStatus.find( QUO_MARKET_SHFEOPT );
			std::map<short,enum XDFRunStat>::iterator itZZOPT = s_mapMkStatus.find( QUO_MARKET_CZCEOPT );

			if( itDL == s_mapMkStatus.end() && itSH == s_mapMkStatus.end() && itZZ == s_mapMkStatus.end() )	bNotifyCNF = false;
			if( itDLOPT == s_mapMkStatus.end() && itSHOPT == s_mapMkStatus.end() && itZZOPT == s_mapMkStatus.end() )	bNotifyCNFOPT = false;
			if( itDL != s_mapMkStatus.end() ) {				if( itDL->second != XRS_Normal )	bNotifyCNF = false;			}
			if( itSH != s_mapMkStatus.end() ) {				if( itSH->second != XRS_Normal )	bNotifyCNF = false;			}
			if( itZZ != s_mapMkStatus.end() ) {				if( itZZ->second != XRS_Normal )	bNotifyCNF = false;			}
			if( itDLOPT != s_mapMkStatus.end() ) {			if( itDLOPT->second != XRS_Normal )	bNotifyCNFOPT = false;			}
			if( itSHOPT != s_mapMkStatus.end() ) {			if( itSHOPT->second != XRS_Normal )	bNotifyCNFOPT = false;			}
			if( itZZOPT != s_mapMkStatus.end() ) {			if( itZZOPT->second != XRS_Normal )	bNotifyCNFOPT = false;			}

			if( true == bNotifyCNF )
			{
				Global_pSpi->XDF_OnRspStatusChanged( XDF_CNF, XRS_Normal );
				::sprintf( pszInfo, "[Status Changed] ---> MkID = %d, Status = %d", XDF_CNF, XRS_Normal );
				s_mapMkStatus.erase( QUO_MARKET_DCE );s_mapMkStatus.erase( QUO_MARKET_SHFE );s_mapMkStatus.erase( QUO_MARKET_CZCE );
				OnLog( 0, pszInfo );
			}

			if( true == bNotifyCNFOPT )
			{
				Global_pSpi->XDF_OnRspStatusChanged( XDF_CNFOPT, XRS_Normal );
				::sprintf( pszInfo, "[Status Changed] ---> MkID = %d, status = %d", XDF_CNFOPT, XRS_Normal );
				s_mapMkStatus.erase( QUO_MARKET_DCEOPT );s_mapMkStatus.erase( QUO_MARKET_SHFEOPT );s_mapMkStatus.erase( QUO_MARKET_CZCEOPT );
				OnLog( 0, pszInfo );
			}

			return;
		}
		///< --------------------------------------------------------------------------------------------------

		Global_pSpi->XDF_OnRspStatusChanged( cMkID, eStatus );

		::sprintf( pszInfo, "[Status Changed] ---> MkID = %d, Status = %d", cMkID, eStatus );
		OnLog( 0, pszInfo );
	}
}

void QuotationAdaptor::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{
	if( Global_pSpi )
	{
		Global_pSpi->XDF_OnRspOutLog( 0, nLogLevel, pszLogBuf );
	}
	else
	{
		::printf( "%s\n", pszLogBuf );
	}
}





