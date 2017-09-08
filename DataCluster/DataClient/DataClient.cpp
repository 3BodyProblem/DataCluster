#include "DataClient.h"
#include "../DataCluster.h"
#include "../DataCenterEngine/DataCenterEngine.h"
#include "../DataCluster.h"



#define MAX_DATABUF_COUNT	81920000

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
		catch (...)
        {
            // 处理异常代码
            //Global_WriteLog(ERR, 0,"<TaskThreadFunc>线程出现未知异常");
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

int STDCALL	MDataClient::Init()
{
	if( NULL == m_pQueryBuffer )
	{
		m_pQueryBuffer = new char[1024*1024*10];
	}

	if (!Global_bInit)
	{
		Global_bInit = true;

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
		Global_bInit = false;
		EndWork();
		g_oDataIO.Release();
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
//	Global_DllMgr.EndWork();
//	Global_DataIO.Release();
}

int	 STDCALL		MDataClient::GetMarketInfo( unsigned char cMarket, char* pszInBuf, int nInBytes )
{
	int							nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64			nSerialNo = 0;
	XDFAPI_MarketKindHead		oHead = { 0 };
	tagQUO_MarketInfo			tagMkInfo;
	MStreamWrite				oMSW( pszInBuf, nInBytes );
	DatabaseAdaptor&			refDatabase = DataIOEngine::GetEngineObj().GetDatabaseObj();

	if( nInnerMkID < 0 )
	{
		return -1;
	}

	if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(tagQUO_MarketInfo), nSerialNo ) <= 0 )
	{
		return -2;
	}

	::memset( &tagMkInfo, 0, sizeof(tagQUO_MarketInfo) );
	oHead.WareCount = tagMkInfo.uiWareCount;
	oHead.KindCount = tagMkInfo.uiKindCount;
	oMSW.PutSingleMsg( 100, (char*)&oHead, sizeof(XDFAPI_MarketKindHead) );

	for( int n = 0; n < tagMkInfo.uiKindCount; n++ )
	{
		tagQUO_KindInfo&		tagCategory = tagMkInfo.mKindRecord[n];
		XDFAPI_MarketKindInfo	oInfo = { 0 };

		oInfo.Serial = n;
		memcpy(oInfo.KindName, tagCategory.szKindName, 8);
//		oInfo.WareCount = tagCategory.;
//		oInfo.PriceRate = tagCategory->;
		oInfo.LotSize = tagCategory.uiLotSize;
		oMSW.PutMsg( 101, (char*)&oInfo, sizeof(oInfo) );
	}

	oMSW.Detach();

	return oMSW.GetOffset();
}

int	STDCALL		MDataClient::GetCodeTable( unsigned char cMarket, char* pszInBuf, int nInBytes, int& nCount )
{
	int							nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64			nSerialNo = 0;
	tagQUO_MarketInfo			tagMkInfo;
	CriticalLock				lock( m_oLock );
	MStreamWrite				oMSW( pszInBuf, nInBytes );
	DatabaseAdaptor&			refDatabase = DataIOEngine::GetEngineObj().GetDatabaseObj();
	unsigned int				MsgType = 0;
	unsigned int				MsgSize = 0;
	int							nDataSize = 0;

	if( nInnerMkID < 0 || NULL == m_pQueryBuffer )
	{
		return -1;
	}

	if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(tagQUO_MarketInfo), nSerialNo ) <= 0 )
	{
		return -2;
	}

	::memset( &tagMkInfo, 0, sizeof(tagQUO_MarketInfo) );
	nCount = tagMkInfo.uiWareCount;
	if( 0 == pszInBuf || 0 == nInBytes )		///< 通过nCount返回码表个数
	{
		return 1;
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
	else if( XDF_CNFOPT == cMarket )//商品期货和商品期权(上海/郑州/大连)
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
		tagQUO_ReferenceData*		pRefData = (tagQUO_ReferenceData*)m_pQueryBuffer;
		if( (nDataSize=refDatabase.QueryBatchRecords( (nInnerMkID*100+2), m_pQueryBuffer, 1024*1024*10, nSerialNo )) <= 0 )
		{
			return -4;
		}

		for( unsigned int nOffset = 0; nOffset < nDataSize; nOffset+=MsgSize )
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
				tagShL1Name.SecKind = pRefData->uiKindID;
			}
			else if( XDF_CF == cMarket )//中金期货
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJQHName;
				tagZJQHName.Market = XDF_CF;
				memcpy( tagZJQHName.Code, pRefData->szCode, 6 );
				tagZJQHName.SecKind = pRefData->uiKindID;
				tagZJQHName.ContractMult = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractMult;	///< 合约乘数
				//tagZJQHName.ExFlag = pRefData->ExFlag;			///< 最后交易日标记,0x01表示是最后交易日只对普通合约有效；其他值暂未定义
				strncpy(tagZJQHName.Name, pRefData->szName, 8);
				//tagZJQHName->ObjectMId = pRefData->ObjectMId;		///< 标的指数市场编号[参见数据字典-市场编号]，0xFF表示未知
				strncpy(tagZJQHName.ObjectCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode,6);
			}
			else if( XDF_CNF == cMarket )//商品期货(上海/郑州/大连)
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfName;
				tagCnfName.Market = XDF_CNF;
				tagCnfName.SecKind = pRefData->uiKindID;
				memcpy( tagCnfName.Code,pRefData->szCode, 20 );
				memcpy( tagCnfName.Name, pRefData->szName, 40 );
				tagCnfName.LotFactor = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotFactor;
			}
			else if( XDF_SHOPT == cMarket )//上证期权
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagSHOPTName;
				tagSHOPTName.Market = XDF_SHOPT;
				memcpy(tagSHOPTName.Code, pRefData->szCode, 8);
				memcpy(tagSHOPTName.Name, pRefData->szName,20);
				tagSHOPTName.SecKind = pRefData->uiKindID;
				memcpy(tagSHOPTName.ContractID, pRefData->szContractID, 19);
				tagSHOPTName.OptionType = tagMkInfo.mKindRecord[pRefData->uiKindID].cOptionType;
				tagSHOPTName.CallOrPut = pRefData->cCallOrPut;

				//tagSHOPTName.PreClosePx = pRefData->C;//合约昨收(如遇除权除息则为调整后的收盘价格)(精确到厘)//[*放大倍数]
				//tagSHOPTName.PreSettlePx = pRefData->;//合约昨结//[*放大倍数]
				//tagSHOPTName.LeavesQty = pRefData->;//未平仓合约数 = 昨持仓 单位是(张)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//memcpy(tagSHOPTName.UnderlyingName, pRefData-, 6);//标的证券名称
				//memcpy(tagSHOPTName.UnderlyingType, pRefData-, 3);//标的证券类型(EBS -ETF, ASH -A股)
				//tagSHOPTName.UnderlyingClosePx = ;//标的证券的昨收 //[*放大倍数]
				//tagSHOPTName.PriceLimitType = pRefData-//涨跌幅限制类型(N 有涨跌幅)(R 无涨跌幅)
				//tagSHOPTName.UpLimit = pRefData->;//当日期权涨停价格(精确到厘) //[*放大倍数]
				//tagSHOPTName.DownLimit;//当日期权跌停价格(精确到厘) //[*放大倍数]
				tagSHOPTName.LotSize = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotSize;//一手等于几张合约
				tagSHOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;//合约单位(经过除权除息调整后的合约单位，一定为整数)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice;//行权价格(精确到厘) //[*放大倍数] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//首个交易日(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//最后交易日(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//行权日(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//交割日(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->uiExpireDate;//到期日(YYYYMMDD)
				//tagSHOPTName.UpdateVersion = pRefData->;//期权合约的版本号(新挂合约是'1')
	/*
		unsigned long					MarginUnit;			//单位保证金(精确到分)//[*放大100]
		short							MarginRatio;		//保证金比例1(%)
		short							MarginRatio2;		//保证金比例2(%)
		int								MinMktFloor;		//单笔市价申报下限
		int								MaxMktFloor;		//单笔市价申报上限
		int								MinLmtFloor;		//单笔限价申报下限
		int								MaxLmtFloor;		//单笔限价申报上限
		char							StatusFlag[8];		//期权合约状态(8个字符,详细定义见文档)
	*/
			}
			else if( XDF_ZJOPT == cMarket )//中金期权
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJOPTName;
				tagZJOPTName.Market = XDF_ZJOPT;
				memcpy(tagZJOPTName.Code, pRefData->szCode, 32);
				tagZJOPTName.SecKind = pRefData->uiKindID;
				tagZJOPTName.ContractMult = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractMult;	//合约乘数
				tagZJOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;	//合约单位
				tagZJOPTName.StartDate = pRefData->uiStartDate;		//首个交易日(YYYYMMDD)
				tagZJOPTName.EndDate = pRefData->uiEndDate;				//最后交易日(YYYYMMDD)
				tagZJOPTName.XqDate = pRefData->uiExerciseDate;				//行权日(YYYYMMDD)
				tagZJOPTName.DeliveryDate = pRefData->uiDeliveryDate;	//交割日(YYYYMMDD)
				tagZJOPTName.ExpireDate = pRefData->uiExpireDate;		//到期日(YYYYMMDD)
				//pTable->ObjectMId = pNameTb->ObjectMId;
				strncpy( tagZJOPTName.ObjectCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6 );
			}
			else if( XDF_SZOPT == cMarket )//深圳期权
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagSZOPTName;
				tagSZOPTName.Market = XDF_SZOPT;
				tagSZOPTName.SecKind = pRefData->uiKindID;
				memcpy(tagSZOPTName.Code, pRefData->szCode, 8);
				memcpy(tagSZOPTName.Name, pRefData->szName,20);
				memcpy(tagSZOPTName.ContractID, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 20);
				tagSHOPTName.OptionType = tagMkInfo.mKindRecord[pRefData->uiKindID].cOptionType;
				tagSHOPTName.CallOrPut = pRefData->cCallOrPut;
				//tagSHOPTName.PreClosePx = pRefData->C;//合约昨收(如遇除权除息则为调整后的收盘价格)(精确到厘)//[*放大倍数]
				//tagSHOPTName.PreSettlePx = pRefData->;//合约昨结//[*放大倍数]
				//tagSHOPTName.LeavesQty = pRefData->;//未平仓合约数 = 昨持仓 单位是(张)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//tagSHOPTName.UpLimit = pRefData->;//当日期权涨停价格(精确到厘) //[*放大倍数]
				//tagSHOPTName.DownLimit;//当日期权跌停价格(精确到厘) //[*放大倍数]
				tagSHOPTName.LotSize = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotSize;//一手等于几张合约
				tagSHOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;//合约单位(经过除权除息调整后的合约单位，一定为整数)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice;//行权价格(精确到厘) //[*放大倍数] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//首个交易日(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//最后交易日(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//行权日(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//交割日(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->uiExpireDate;//到期日(YYYYMMDD)
				//tagSHOPTName.MarginUnit = pNameTb->MarginUnit;//单位保证金(精确到分)//[*放大100]
			}
			else if( XDF_CNFOPT == cMarket )//商品期货和商品期权(上海/郑州/大连)
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfOPTName;
				tagCnfOPTName.Market = XDF_CNFOPT;
				tagCnfOPTName.SecKind = pRefData->uiKindID;
				memcpy(tagCnfOPTName.Code,pRefData->szCode, 20);
				memcpy(tagCnfOPTName.Name, pRefData->szName, 40);
				tagCnfOPTName.LotFactor = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotFactor;
				//tagSHOPTName.LeavesQty = pRefData->;//未平仓合约数 = 昨持仓 单位是(张)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//pTable->PriceLimitType = pNameTb->PriceLimitType;
				//tagSHOPTName.UpLimit = pRefData->;//当日期权涨停价格(精确到厘) //[*放大倍数]
				//tagSHOPTName.DownLimit;//当日期权跌停价格(精确到厘) //[*放大倍数]
				tagSHOPTName.LotSize = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotSize;//一手等于几张合约
				tagSHOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;//合约单位(经过除权除息调整后的合约单位，一定为整数)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice;//行权价格(精确到厘) //[*放大倍数] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//首个交易日(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//最后交易日(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//行权日(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//交割日(YYYYMMDD)
	//			tagSHOPTName.TypePeriodIndx = pNameTb->TypePeriodIndx;
	//			tagSHOPTName.EarlyNightFlag = pNameTb->EarlyNightFlag;
			}

			oMSW.PutMsg( MsgType, pData, MsgSize );
		}

		if( XDF_CNF != cMarket && XDF_CNFOPT != cMarket )
		{
			break;
		}

		if( XDF_CNF == cMarket )
		{
			if( 1 == i )	nInnerMkID = QUO_MARKET_CZCE;
			if( 2 == i )	nInnerMkID = QUO_MARKET_SHFE;
		}

		if( XDF_CNF == cMarket )
		{
			if( 1 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
			if( 2 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
		}
	}

	oMSW.Detach();

	return oMSW.GetOffset();
}

int STDCALL		MDataClient::GetLastMarketDataAll(unsigned char cMarket, char* pszInBuf, int nInBytes)
{
	int							nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64			nSerialNo = 0;
	tagQUO_MarketInfo			tagMkInfo;
	CriticalLock				lock( m_oLock );
	MStreamWrite				oMSW( pszInBuf, nInBytes );
	DatabaseAdaptor&			refDatabase = DataIOEngine::GetEngineObj().GetDatabaseObj();
	unsigned int				MsgType = 0;
	unsigned int				MsgSize = 0;
	int							nDataSize = 0;

	if( nInnerMkID < 0 || NULL == m_pQueryBuffer )
	{
		return -1;
	}

	if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(tagQUO_MarketInfo), nSerialNo ) <= 0 )
	{
		return -2;
	}

	::memset( &tagMkInfo, 0, sizeof(tagQUO_MarketInfo) );
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
		if( (nDataSize=refDatabase.QueryBatchRecords( (nInnerMkID*100+3), m_pQueryBuffer, 1024*1024*10, nSerialNo )) <= 0 )
		{
			return -4;
		}

		for( unsigned int nOffset = 0; nOffset < nDataSize; nOffset+=MsgSize )
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
				tagSHL1Stock.High = pSnapData->dHighPx;
				tagSHL1Stock.Open = pSnapData->dOpenPx;
				tagSHL1Stock.Low = pSnapData->dLowPx;
				tagSHL1Stock.PreClose = pSnapData->dPreClosePx;
				tagSHL1Stock.Now = pSnapData->dNowPx;
				tagSHL1Stock.Amount = pSnapData->dAmount;
				tagSHL1Stock.Volume = pSnapData->ui64Volume;
				//tagSHL1Stock.Records = pSnapData->Records;
				//tagSHL1Stock.HighLimit = pSnapData->HighLimit;
				//tagSHL1Stock.LowLimit = pSnapData->LowLimit;
				//tagSHL1Stock.Voip = pSnapData->Voip;

				for (int i=0; i<5; i++)
				{
					tagSHL1Stock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagSHL1Stock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagSHL1Stock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagSHL1Stock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CF == cMarket )//中金期货
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCFFStock;

				//tagCFFStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCFFStock.Code, pSnapData->szCode,6);
				tagCFFStock.High = pSnapData->dHighPx;						//最高价格[* 放大倍数]
				tagCFFStock.Open = pSnapData->dOpenPx;						//开盘价格[* 放大倍数]
				tagCFFStock.Low = pSnapData->dLowPx;						//最低价格[* 放大倍数]
				tagCFFStock.PreClose = pSnapData->dPreClosePx;				//昨收价格[* 放大倍数]
				tagCFFStock.PreSettlePrice = pSnapData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
				tagCFFStock.Now = pSnapData->dNowPx;                    //最新价格[* 放大倍数]
				tagCFFStock.Close = pSnapData->dClosePx;                  //今日收盘价格[* 放大倍数]
				tagCFFStock.SettlePrice	= pSnapData->dSettlePx;      //今日结算价格[* 放大倍数]
				tagCFFStock.UpperPrice = pSnapData->dUpperLimitPx;         //涨停价格[* 放大倍数]
				tagCFFStock.LowerPrice = pSnapData->dLowerLimitPx;         //跌停价格[* 放大倍数]
				tagCFFStock.Amount = pSnapData->dAmount;             //总成交金额[元]
				tagCFFStock.Volume = pSnapData->ui64Volume;             //总成交量[股]
				tagCFFStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagCFFStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]

				for (int i=0; i<5; i++)
				{
					tagCFFStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagCFFStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCFFStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagCFFStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CNF == cMarket )//商品期货(上海/郑州/大连)
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFStock;

				//tagCNFStock.Date = pSnapData->Date;
				//tagCNFStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCNFStock.Code, pSnapData->szCode,20);
				tagCNFStock.High = pSnapData->dHighPx;						//最高价格[* 放大倍数]
				tagCNFStock.Open = pSnapData->dOpenPx;						//开盘价格[* 放大倍数]
				tagCNFStock.Low = pSnapData->dLowPx;						//最低价格[* 放大倍数]
				tagCNFStock.PreClose = pSnapData->dPreClosePx;				//昨收价格[* 放大倍数]
				tagCNFStock.PreSettlePrice = pSnapData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
				tagCNFStock.Now = pSnapData->dNowPx;                    //最新价格[* 放大倍数]
				tagCNFStock.Close = pSnapData->dClosePx;                  //今日收盘价格[* 放大倍数]
				tagCNFStock.SettlePrice	 = pSnapData->dSettlePx;        //今日结算价格[* 放大倍数]
				tagCNFStock.UpperPrice	 = pSnapData->dUpperLimitPx;         //涨停价格[* 放大倍数]
				tagCNFStock.LowerPrice	 = pSnapData->dLowerLimitPx;         //跌停价格[* 放大倍数]
				tagCNFStock.Amount		 = pSnapData->dAmount;             //总成交金额[元]
				tagCNFStock.Volume		 = pSnapData->ui64Volume;             //总成交量[股]
				tagCNFStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagCNFStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]

				for (int i=0; i<5; i++)
				{
					tagCNFStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagCNFStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCNFStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagCNFStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_SHOPT == cMarket )//上证期权
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSHOPTStock;

				memcpy(tagSHOPTStock.Code, pSnapData->szCode,8);
				//tagSHOPTStock.Time = pSnapData->DataTimeStamp;
				tagSHOPTStock.PreSettlePx = pSnapData->dPreSettlePx;
				tagSHOPTStock.SettlePrice = pSnapData->dSettlePx;
				tagSHOPTStock.OpenPx = pSnapData->dOpenPx;
				tagSHOPTStock.HighPx = pSnapData->dHighPx;
				tagSHOPTStock.LowPx = pSnapData->dLowPx;
				tagSHOPTStock.Now = pSnapData->dNowPx;
				tagSHOPTStock.Volume = pSnapData->ui64Volume;
				tagSHOPTStock.Amount = pSnapData->dAmount;
				memcpy(tagSHOPTStock.TradingPhaseCode, pSnapData->szTradingPhaseCode,4);
				//tagSHOPTStock.AuctionPrice = pSnapData->AuctionPrice;
				//tagSHOPTStock.AuctionQty = pSnapData->AuctionQty;
				tagSHOPTStock.Position = pSnapData->ui64OpenInterest;
				for (int i=0; i<5; i++)
				{
					tagSHOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagSHOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagSHOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagSHOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_ZJOPT == cMarket )//中金期权
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagZJOPTStock;

				//tagZJOPTStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagZJOPTStock.Code, pSnapData->szCode,32);
				tagZJOPTStock.High = pSnapData->dHighPx;						//最高价格[* 放大倍数]
				tagZJOPTStock.Open = pSnapData->dOpenPx;						//开盘价格[* 放大倍数]
				tagZJOPTStock.Low = pSnapData->dLowPx;						//最低价格[* 放大倍数]
				tagZJOPTStock.PreClose = pSnapData->dPreClosePx;				//昨收价格[* 放大倍数]
				tagZJOPTStock.PreSettlePrice = pSnapData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
				
				tagZJOPTStock.Now = pSnapData->dNowPx;                    //最新价格[* 放大倍数]
				tagZJOPTStock.Close	= pSnapData->dClosePx;                  //今日收盘价格[* 放大倍数]
				tagZJOPTStock.SettlePrice = pSnapData->dSettlePx;        //今日结算价格[* 放大倍数]
				tagZJOPTStock.UpperPrice = pSnapData->dUpperLimitPx;         //涨停价格[* 放大倍数]
				tagZJOPTStock.LowerPrice = pSnapData->dLowerLimitPx;         //跌停价格[* 放大倍数]
				tagZJOPTStock.Amount = pSnapData->dAmount;             //总成交金额[元]
				tagZJOPTStock.Volume = pSnapData->ui64Volume;             //总成交量[股]
				tagZJOPTStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagZJOPTStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]

				for (int i=0; i<5; i++)
				{
					tagZJOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagZJOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagZJOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagZJOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_SZOPT == cMarket )//深圳期权
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSZOPTStock;

				memcpy(tagSZOPTStock.Code, pSnapData->szCode,8);
				//tagSZOPTStock.Time = pSnapData->DataTimeStamp;
				tagSZOPTStock.PreSettlePx = pSnapData->dPreSettlePx;
				tagSZOPTStock.SettlePrice = pSnapData->dSettlePx;
				tagSZOPTStock.OpenPx = pSnapData->dOpenPx;
				tagSZOPTStock.HighPx = pSnapData->dHighPx;
				tagSZOPTStock.LowPx = pSnapData->dLowPx;
				tagSZOPTStock.Now = pSnapData->dNowPx;
				tagSZOPTStock.Volume = pSnapData->ui64Volume;
				tagSZOPTStock.Amount = pSnapData->dAmount;
				memcpy(tagSZOPTStock.TradingPhaseCode, pSnapData->szTradingPhaseCode,4);
				tagSZOPTStock.Position = pSnapData->ui64OpenInterest;
				for (int i=0; i<5; i++)
				{
					tagSZOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagSZOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagSZOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagSZOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CNFOPT == cMarket )//商品期货和商品期权(上海/郑州/大连)
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFOPTStock;

				//tagCNFOPTStock.Date = pSnapData->Date;
				//tagCNFOPTStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCNFOPTStock.Code, pSnapData->szCode,20);
				tagCNFOPTStock.High = pSnapData->dHighPx;						//最高价格[* 放大倍数]
				tagCNFOPTStock.Open = pSnapData->dOpenPx;						//开盘价格[* 放大倍数]
				tagCNFOPTStock.Low = pSnapData->dLowPx;						//最低价格[* 放大倍数]
				tagCNFOPTStock.PreClose = pSnapData->dPreClosePx;				//昨收价格[* 放大倍数]
				tagCNFOPTStock.PreSettlePrice = pSnapData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
				tagCNFOPTStock.Now = pSnapData->dNowPx;                    //最新价格[* 放大倍数]
				tagCNFOPTStock.Close = pSnapData->dClosePx;                  //今日收盘价格[* 放大倍数]
				tagCNFOPTStock.SettlePrice = pSnapData->dSettlePx;        //今日结算价格[* 放大倍数]
				tagCNFOPTStock.UpperPrice = pSnapData->dUpperLimitPx;         //涨停价格[* 放大倍数]
				tagCNFOPTStock.LowerPrice = pSnapData->dLowerLimitPx;         //跌停价格[* 放大倍数]
				tagCNFOPTStock.Amount = pSnapData->dAmount;             //总成交金额[元]
				tagCNFOPTStock.Volume = pSnapData->ui64Volume;             //总成交量[股]
				tagCNFOPTStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //昨日持仓量[股]
				tagCNFOPTStock.OpenInterest = pSnapData->ui64OpenInterest;       //持仓量[股]

				for (int i=0; i<5; i++)
				{
					tagCNFOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagCNFOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCNFOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagCNFOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}

			oMSW.PutMsg( MsgType, pData, MsgSize );
		}

		if( XDF_CNF != cMarket && XDF_CNFOPT != cMarket )
		{
			break;
		}

		if( XDF_CNF == cMarket )
		{
			if( 1 == i )	nInnerMkID = QUO_MARKET_CZCE;
			if( 2 == i )	nInnerMkID = QUO_MARKET_SHFE;
		}

		if( XDF_CNF == cMarket )
		{
			if( 1 == i )	nInnerMkID = QUO_MARKET_CZCEOPT;
			if( 2 == i )	nInnerMkID = QUO_MARKET_SHFEOPT;
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
/*	if (FuncNo ==100)		//获取某个市场的市场日期和市场时间(参数:uint8*,   XDFAPI_MarketStatusInfo*)
	{
		uint8_t * pMarket = (uint8_t*)wParam;
		XDFAPI_MarketStatusInfo* pInfo = (XDFAPI_MarketStatusInfo*)lParam;
		if (pMarket && pInfo)
		{
			uint8_t cMarket = *pMarket;
			XDFAPI_MarketStatusInfo oInfo;

			oInfo.MarketID = cMarket;
			int iret = Global_DllMgr.GetSimpleMarketInfo(cMarket, &oInfo);
			if (iret >0)
			{
				*pInfo = oInfo;
				return 1;
			}
		}
	}
	if (FuncNo ==101)		//获取某个市场的市场日期和市场时间(参数:uint8*,   XDFAPI_MarketStatusInfo*)
	{
		uint8_t * pMarket = (uint8_t*)wParam;
		XDFAPI_MarketStatusInfo* pInfo = (XDFAPI_MarketStatusInfo*)lParam;
		if (pMarket && pInfo)
		{
			uint8_t cMarket = *pMarket;
			XDFAPI_MarketStatusInfo oInfo;

			oInfo.MarketID = cMarket;
			int iret = Global_DllMgr.GetQuicksSimpleMarketInfo(cMarket, &oInfo);
			if (iret >0)
			{
				*pInfo = oInfo;
				return 1;
			}
		}
	}*/
	/*
	else if (FuncNo == 101)		//获取 当前｛商品期货和商品期权｝挂载的是14 还是35, 还是(-1) ??( 参数:int*  )
	{
		int iret = Global_Option.GetCnfOptMarketID();
		int* pid = (int*)wParam;
		if (pid)
		{
			*pid = iret;
		}
		return 1;
	}
	*/


	return 0;
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
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.MarketID = XDF_SH;
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
			tagData.High = pData->dHighPx;
			tagData.Open = pData->dOpenPx;
			tagData.Low = pData->dLowPx;
			tagData.PreClose = pData->dPreClosePx;
			tagData.Now = pData->dNowPx;
			tagData.Amount = pData->dAmount;
			tagData.Volume = pData->ui64Volume;
			//tagSHL1Stock.Records = pData->Records;
			//tagSHL1Stock.HighLimit = pData->HighLimit;
			//tagSHL1Stock.LowLimit = pData->LowLimit;
			//tagSHL1Stock.Voip = pData->Voip;

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(22, (char*)&tagData,sizeof(XDFAPI_StockData5) );
		}
		break;
	case XDF_CF://中金期货
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.MarketID = XDF_CF;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 4 )
		{
			XDFAPI_CffexData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			//tagCFFStock.DataTimeStamp = pSnapData->DataTimeStamp;
			memcpy(tagData.Code, pData->szCode,6);
			tagData.High = pData->dHighPx;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
			tagData.Now = pData->dNowPx;                    //最新价格[* 放大倍数]
			tagData.Close = pData->dClosePx;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice	= pData->dSettlePx;      //今日结算价格[* 放大倍数]
			tagData.UpperPrice = pData->dUpperLimitPx;         //涨停价格[* 放大倍数]
			tagData.LowerPrice = pData->dLowerLimitPx;         //跌停价格[* 放大倍数]
			tagData.Amount = pData->dAmount;             //总成交金额[元]
			tagData.Volume = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(20, (char*)&tagData,sizeof(XDFAPI_CffexData) );
		}
		break;
	case XDF_CNF://商品期货(上海/郑州/大连)
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.MarketID = XDF_CNF;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 4 )
		{
			XDFAPI_CNFutureData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			//tagData.Date = pData->Date;
			//tagData.DataTimeStamp = pData->DataTimeStamp;
			memcpy(tagData.Code, pData->szCode,20);
			tagData.High = pData->dHighPx;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
			tagData.Now = pData->dNowPx;                    //最新价格[* 放大倍数]
			tagData.Close = pData->dClosePx;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice	 = pData->dSettlePx;        //今日结算价格[* 放大倍数]
			tagData.UpperPrice	 = pData->dUpperLimitPx;         //涨停价格[* 放大倍数]
			tagData.LowerPrice	 = pData->dLowerLimitPx;         //跌停价格[* 放大倍数]
			tagData.Amount		 = pData->dAmount;             //总成交金额[元]
			tagData.Volume		 = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(26, (char*)&tagData, sizeof(XDFAPI_CNFutureData) );
		}
		break;
	case XDF_SHOPT://上证期权
		if( nMsgID == 1 )
		{
			XDFAPI_ShOptMarketStatus	tagData = { 0 };
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.TradingPhaseCode[0] = '1';

			oMSW.PutSingleMsg(14, (char*)&tagData, sizeof(XDFAPI_ShOptMarketStatus));
			
		}
		else if( nMsgID == 4 )
		{
			XDFAPI_ShOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,8);
			//tagData.Time = pData->DataTimeStamp;
			tagData.PreSettlePx = pData->dPreSettlePx;
			tagData.SettlePrice = pData->dSettlePx;
			tagData.OpenPx = pData->dOpenPx;
			tagData.HighPx = pData->dHighPx;
			tagData.LowPx = pData->dLowPx;
			tagData.Now = pData->dNowPx;
			tagData.Volume = pData->ui64Volume;
			tagData.Amount = pData->dAmount;
			memcpy(tagData.TradingPhaseCode, pData->szTradingPhaseCode,4);
			//tagData.AuctionPrice = pData->AuctionPrice;
			//tagData.AuctionQty = pData->AuctionQty;
			tagData.Position = pData->ui64OpenInterest;
			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(15, (char*)&tagData, sizeof(XDFAPI_ShOptData));
		}
		break;
	case XDF_ZJOPT://中金期权
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.MarketID = XDF_ZJOPT;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 4 )
		{
			XDFAPI_ZjOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			//tagData.DataTimeStamp = pData->DataTimeStamp;
			memcpy(tagData.Code, pData->szCode,32);
			tagData.High = pData->dHighPx;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
			
			tagData.Now = pData->dNowPx;                    //最新价格[* 放大倍数]
			tagData.Close	= pData->dClosePx;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice = pData->dSettlePx;        //今日结算价格[* 放大倍数]
			tagData.UpperPrice = pData->dUpperLimitPx;         //涨停价格[* 放大倍数]
			tagData.LowerPrice = pData->dLowerLimitPx;         //跌停价格[* 放大倍数]
			tagData.Amount = pData->dAmount;             //总成交金额[元]
			tagData.Volume = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(18, (char*)&tagData, sizeof(XDFAPI_ZjOptData) );
		}
		break;
	case XDF_SZOPT://深圳期权
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.MarketID = XDF_SZOPT;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 4 )
		{
			XDFAPI_SzOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			memcpy(tagData.Code, pData->szCode,8);
			//tagData.Time = pData->DataTimeStamp;
			tagData.PreSettlePx = pData->dPreSettlePx;
			tagData.SettlePrice = pData->dSettlePx;
			tagData.OpenPx = pData->dOpenPx;
			tagData.HighPx = pData->dHighPx;
			tagData.LowPx = pData->dLowPx;
			tagData.Now = pData->dNowPx;
			tagData.Volume = pData->ui64Volume;
			tagData.Amount = pData->dAmount;
			memcpy(tagData.TradingPhaseCode, pData->szTradingPhaseCode,4);
			tagData.Position = pData->ui64OpenInterest;
			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
				tagData.Sell[i].Volume = pData->mAsk[i].ui64Volume;
			}

			oMSW.PutMsg(35, (char*)&tagData, sizeof(XDFAPI_SzOptData));
		}
		break;
	case XDF_CNFOPT://商品期货和商品期权(上海/郑州/大连)
		if( nMsgID == 1 )
		{
			XDFAPI_MarketStatusInfo		tagData = { 0 };
			tagQUO_MarketInfo*			pData = (tagQUO_MarketInfo*)pDataPtr;

			tagData.MarketDate = pData->uiMarketDate;
			tagData.MarketTime = pData->uiMarketTime;
			tagData.MarketID = XDF_CNFOPT;
			tagData.MarketStatus = 1;

			oMSW.PutSingleMsg(1, (char*)&tagData, sizeof(XDFAPI_MarketStatusInfo));
			
		}
		else if( nMsgID == 4 )
		{
			XDFAPI_CNFutOptData			tagData = { 0 };
			tagQUO_SnapData*			pData = (tagQUO_SnapData*)pDataPtr;

			//tagData.Date = pData->Date;
			//tagData.DataTimeStamp = pData->DataTimeStamp;
			memcpy(tagData.Code, pData->szCode,20);
			tagData.High = pData->dHighPx;						//最高价格[* 放大倍数]
			tagData.Open = pData->dOpenPx;						//开盘价格[* 放大倍数]
			tagData.Low = pData->dLowPx;						//最低价格[* 放大倍数]
			tagData.PreClose = pData->dPreClosePx;				//昨收价格[* 放大倍数]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//昨日结算价格[* 放大倍数]
			tagData.Now = pData->dNowPx;                    //最新价格[* 放大倍数]
			tagData.Close = pData->dClosePx;                  //今日收盘价格[* 放大倍数]
			tagData.SettlePrice = pData->dSettlePx;        //今日结算价格[* 放大倍数]
			tagData.UpperPrice = pData->dUpperLimitPx;         //涨停价格[* 放大倍数]
			tagData.LowerPrice = pData->dLowerLimitPx;         //跌停价格[* 放大倍数]
			tagData.Amount = pData->dAmount;             //总成交金额[元]
			tagData.Volume = pData->ui64Volume;             //总成交量[股]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //昨日持仓量[股]
			tagData.OpenInterest = pData->ui64OpenInterest;       //持仓量[股]

			for (int i=0; i<5; i++)
			{
				tagData.Buy[i].Price = pData->mBid[i].dVPrice;
				tagData.Buy[i].Volume = pData->mBid[i].ui64Volume;
				tagData.Sell[i].Price = pData->mAsk[i].dVPrice;
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

void QuotationAdaptor::OnStatus( QUO_MARKET_ID eMarketID,QUO_MARKET_STATUS eMarketStatus )
{
	if( Global_pSpi )
	{
		Global_pSpi->XDF_OnRspStatusChanged( eMarketID, 1 );
	}
}

void QuotationAdaptor::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{
	if( Global_pSpi )
	{
		Global_pSpi->XDF_OnRspOutLog( 0, nLogLevel, pszLogBuf );
	}
}





