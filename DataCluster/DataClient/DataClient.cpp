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
			pSelf->m_oWEvent.Wait(1000);	//ÿ��1����ɨ��

			pSelf->inner_CheckData();
		
		}
		catch (...)
        {
            // �����쳣����
            //Global_WriteLog(ERR, 0,"<TaskThreadFunc>�̳߳���δ֪�쳣");
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
	if (m_bError)		//�����ѳ���
		return;
	
	int noffset = nStructSize + sizeof(XDFAPI_MsgHead);
	if (m_nInBytes - m_nOffset < noffset)	//��������
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
	if (m_bError)	//�����ѳ���
		return;
	
	//[1]
	int realoffset = nStructSize;
	bool breuse =false;
	//����Ƿ�m_pUniHeadΪ0
	//���MsgLen�Ƿ�Խ��
	//������ͺ�LastType�Ƿ�һ��
	if (0 == m_pUniHead || m_pUniHead->MsgLen >64000 || m_nLastType!= ntype)
	{
		realoffset += sizeof(XDFAPI_UniMsgHead);
		breuse =false;
	}
	else
	{
		breuse =true;
	}

	//[2]�������
	if (m_nInBytes - m_nOffset < realoffset) //��������
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
	if( 0 == pszInBuf || 0 == nInBytes )		///< ͨ��nCount�����������
	{
		return 1;
	}

	if( XDF_SH == cMarket || XDF_SZ == cMarket )
	{
		MsgType = 5;
		MsgSize = sizeof(XDFAPI_NameTableSh);
	}
	else if( XDF_CF == cMarket )//�н��ڻ�
	{
		MsgType = 4;
		MsgSize = sizeof(XDFAPI_NameTableZjqh);
	}
	else if( XDF_CNF == cMarket )//��Ʒ�ڻ�(�Ϻ�/֣��/����)
	{
		MsgType = 7;
		MsgSize = sizeof(XDFAPI_NameTableCnf);
	}
	else if( XDF_SHOPT == cMarket )//��֤��Ȩ
	{
		MsgType = 2;
		MsgSize = sizeof(XDFAPI_NameTableShOpt);
	}
	else if( XDF_ZJOPT == cMarket )//�н���Ȩ
	{
		MsgType = 3;
		MsgSize = sizeof(XDFAPI_NameTableZjOpt);
	}
	else if( XDF_SZOPT == cMarket )//������Ȩ
	{
		MsgType = 9;
		MsgSize = sizeof(XDFAPI_NameTableSzOpt);
	}
	else if( XDF_CNFOPT == cMarket )//��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
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
			else if( XDF_CF == cMarket )//�н��ڻ�
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJQHName;
				tagZJQHName.Market = XDF_CF;
				memcpy( tagZJQHName.Code, pRefData->szCode, 6 );
				tagZJQHName.SecKind = pRefData->uiKindID;
				tagZJQHName.ContractMult = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractMult;	///< ��Լ����
				//tagZJQHName.ExFlag = pRefData->ExFlag;			///< ������ձ��,0x01��ʾ���������ֻ����ͨ��Լ��Ч������ֵ��δ����
				strncpy(tagZJQHName.Name, pRefData->szName, 8);
				//tagZJQHName->ObjectMId = pRefData->ObjectMId;		///< ���ָ���г����[�μ������ֵ�-�г����]��0xFF��ʾδ֪
				strncpy(tagZJQHName.ObjectCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode,6);
			}
			else if( XDF_CNF == cMarket )//��Ʒ�ڻ�(�Ϻ�/֣��/����)
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfName;
				tagCnfName.Market = XDF_CNF;
				tagCnfName.SecKind = pRefData->uiKindID;
				memcpy( tagCnfName.Code,pRefData->szCode, 20 );
				memcpy( tagCnfName.Name, pRefData->szName, 40 );
				tagCnfName.LotFactor = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotFactor;
			}
			else if( XDF_SHOPT == cMarket )//��֤��Ȩ
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

				//tagSHOPTName.PreClosePx = pRefData->C;//��Լ����(������Ȩ��Ϣ��Ϊ����������̼۸�)(��ȷ����)//[*�Ŵ���]
				//tagSHOPTName.PreSettlePx = pRefData->;//��Լ���//[*�Ŵ���]
				//tagSHOPTName.LeavesQty = pRefData->;//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//memcpy(tagSHOPTName.UnderlyingName, pRefData-, 6);//���֤ȯ����
				//memcpy(tagSHOPTName.UnderlyingType, pRefData-, 3);//���֤ȯ����(EBS -ETF, ASH -A��)
				//tagSHOPTName.UnderlyingClosePx = ;//���֤ȯ������ //[*�Ŵ���]
				//tagSHOPTName.PriceLimitType = pRefData-//�ǵ�����������(N ���ǵ���)(R ���ǵ���)
				//tagSHOPTName.UpLimit = pRefData->;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				//tagSHOPTName.DownLimit;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				tagSHOPTName.LotSize = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotSize;//һ�ֵ��ڼ��ź�Լ
				tagSHOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;//��Լ��λ(������Ȩ��Ϣ������ĺ�Լ��λ��һ��Ϊ����)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice;//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//�׸�������(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//�������(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//��Ȩ��(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//������(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->uiExpireDate;//������(YYYYMMDD)
				//tagSHOPTName.UpdateVersion = pRefData->;//��Ȩ��Լ�İ汾��(�¹Һ�Լ��'1')
	/*
		unsigned long					MarginUnit;			//��λ��֤��(��ȷ����)//[*�Ŵ�100]
		short							MarginRatio;		//��֤�����1(%)
		short							MarginRatio2;		//��֤�����2(%)
		int								MinMktFloor;		//�����м��걨����
		int								MaxMktFloor;		//�����м��걨����
		int								MinLmtFloor;		//�����޼��걨����
		int								MaxLmtFloor;		//�����޼��걨����
		char							StatusFlag[8];		//��Ȩ��Լ״̬(8���ַ�,��ϸ������ĵ�)
	*/
			}
			else if( XDF_ZJOPT == cMarket )//�н���Ȩ
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJOPTName;
				tagZJOPTName.Market = XDF_ZJOPT;
				memcpy(tagZJOPTName.Code, pRefData->szCode, 32);
				tagZJOPTName.SecKind = pRefData->uiKindID;
				tagZJOPTName.ContractMult = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractMult;	//��Լ����
				tagZJOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;	//��Լ��λ
				tagZJOPTName.StartDate = pRefData->uiStartDate;		//�׸�������(YYYYMMDD)
				tagZJOPTName.EndDate = pRefData->uiEndDate;				//�������(YYYYMMDD)
				tagZJOPTName.XqDate = pRefData->uiExerciseDate;				//��Ȩ��(YYYYMMDD)
				tagZJOPTName.DeliveryDate = pRefData->uiDeliveryDate;	//������(YYYYMMDD)
				tagZJOPTName.ExpireDate = pRefData->uiExpireDate;		//������(YYYYMMDD)
				//pTable->ObjectMId = pNameTb->ObjectMId;
				strncpy( tagZJOPTName.ObjectCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6 );
			}
			else if( XDF_SZOPT == cMarket )//������Ȩ
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
				//tagSHOPTName.PreClosePx = pRefData->C;//��Լ����(������Ȩ��Ϣ��Ϊ����������̼۸�)(��ȷ����)//[*�Ŵ���]
				//tagSHOPTName.PreSettlePx = pRefData->;//��Լ���//[*�Ŵ���]
				//tagSHOPTName.LeavesQty = pRefData->;//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//tagSHOPTName.UpLimit = pRefData->;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				//tagSHOPTName.DownLimit;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				tagSHOPTName.LotSize = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotSize;//һ�ֵ��ڼ��ź�Լ
				tagSHOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;//��Լ��λ(������Ȩ��Ϣ������ĺ�Լ��λ��һ��Ϊ����)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice;//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//�׸�������(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//�������(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//��Ȩ��(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//������(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->uiExpireDate;//������(YYYYMMDD)
				//tagSHOPTName.MarginUnit = pNameTb->MarginUnit;//��λ��֤��(��ȷ����)//[*�Ŵ�100]
			}
			else if( XDF_CNFOPT == cMarket )//��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
			{
				pRefData = (tagQUO_ReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfOPTName;
				tagCnfOPTName.Market = XDF_CNFOPT;
				tagCnfOPTName.SecKind = pRefData->uiKindID;
				memcpy(tagCnfOPTName.Code,pRefData->szCode, 20);
				memcpy(tagCnfOPTName.Name, pRefData->szName, 40);
				tagCnfOPTName.LotFactor = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotFactor;
				//tagSHOPTName.LeavesQty = pRefData->;//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
				memcpy(tagSHOPTName.UnderlyingCode, tagMkInfo.mKindRecord[pRefData->uiKindID].szUnderlyingCode, 6);
				//pTable->PriceLimitType = pNameTb->PriceLimitType;
				//tagSHOPTName.UpLimit = pRefData->;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				//tagSHOPTName.DownLimit;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				tagSHOPTName.LotSize = tagMkInfo.mKindRecord[pRefData->uiKindID].uiLotSize;//һ�ֵ��ڼ��ź�Լ
				tagSHOPTName.ContractUnit = tagMkInfo.mKindRecord[pRefData->uiKindID].uiContractUnit;//��Լ��λ(������Ȩ��Ϣ������ĺ�Լ��λ��һ��Ϊ����)
				tagSHOPTName.XqPrice = pRefData->dExercisePrice;//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
				tagSHOPTName.StartDate = pRefData->uiStartDate;//�׸�������(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->uiEndDate;//�������(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->uiExerciseDate;//��Ȩ��(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->uiDeliveryDate;//������(YYYYMMDD)
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
	else if( XDF_CF == cMarket )//�н��ڻ�
	{
		MsgType = 20;
		MsgSize = sizeof(XDFAPI_CffexData);
	}
	else if( XDF_CNF == cMarket )//��Ʒ�ڻ�(�Ϻ�/֣��/����)
	{
		MsgType = 26;
		MsgSize = sizeof(XDFAPI_CNFutureData);
	}
	else if( XDF_SHOPT == cMarket )//��֤��Ȩ
	{
		MsgType = 15;
		MsgSize = sizeof(XDFAPI_ShOptData);
	}
	else if( XDF_ZJOPT == cMarket )//�н���Ȩ
	{
		MsgType = 18;
		MsgSize = sizeof(XDFAPI_ZjOptData);
	}
	else if( XDF_SZOPT == cMarket )//������Ȩ
	{
		MsgType = 35;
		MsgSize = sizeof(XDFAPI_SzOptData);
	}
	else if( XDF_CNFOPT == cMarket )//��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
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
			else if( XDF_CF == cMarket )//�н��ڻ�
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCFFStock;

				//tagCFFStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCFFStock.Code, pSnapData->szCode,6);
				tagCFFStock.High = pSnapData->dHighPx;						//��߼۸�[* �Ŵ���]
				tagCFFStock.Open = pSnapData->dOpenPx;						//���̼۸�[* �Ŵ���]
				tagCFFStock.Low = pSnapData->dLowPx;						//��ͼ۸�[* �Ŵ���]
				tagCFFStock.PreClose = pSnapData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
				tagCFFStock.PreSettlePrice = pSnapData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
				tagCFFStock.Now = pSnapData->dNowPx;                    //���¼۸�[* �Ŵ���]
				tagCFFStock.Close = pSnapData->dClosePx;                  //�������̼۸�[* �Ŵ���]
				tagCFFStock.SettlePrice	= pSnapData->dSettlePx;      //���ս���۸�[* �Ŵ���]
				tagCFFStock.UpperPrice = pSnapData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagCFFStock.LowerPrice = pSnapData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagCFFStock.Amount = pSnapData->dAmount;             //�ܳɽ����[Ԫ]
				tagCFFStock.Volume = pSnapData->ui64Volume;             //�ܳɽ���[��]
				tagCFFStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //���ճֲ���[��]
				tagCFFStock.OpenInterest = pSnapData->ui64OpenInterest;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagCFFStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagCFFStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCFFStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagCFFStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_CNF == cMarket )//��Ʒ�ڻ�(�Ϻ�/֣��/����)
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFStock;

				//tagCNFStock.Date = pSnapData->Date;
				//tagCNFStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCNFStock.Code, pSnapData->szCode,20);
				tagCNFStock.High = pSnapData->dHighPx;						//��߼۸�[* �Ŵ���]
				tagCNFStock.Open = pSnapData->dOpenPx;						//���̼۸�[* �Ŵ���]
				tagCNFStock.Low = pSnapData->dLowPx;						//��ͼ۸�[* �Ŵ���]
				tagCNFStock.PreClose = pSnapData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
				tagCNFStock.PreSettlePrice = pSnapData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
				tagCNFStock.Now = pSnapData->dNowPx;                    //���¼۸�[* �Ŵ���]
				tagCNFStock.Close = pSnapData->dClosePx;                  //�������̼۸�[* �Ŵ���]
				tagCNFStock.SettlePrice	 = pSnapData->dSettlePx;        //���ս���۸�[* �Ŵ���]
				tagCNFStock.UpperPrice	 = pSnapData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagCNFStock.LowerPrice	 = pSnapData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagCNFStock.Amount		 = pSnapData->dAmount;             //�ܳɽ����[Ԫ]
				tagCNFStock.Volume		 = pSnapData->ui64Volume;             //�ܳɽ���[��]
				tagCNFStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //���ճֲ���[��]
				tagCNFStock.OpenInterest = pSnapData->ui64OpenInterest;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagCNFStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagCNFStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagCNFStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagCNFStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_SHOPT == cMarket )//��֤��Ȩ
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
			else if( XDF_ZJOPT == cMarket )//�н���Ȩ
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagZJOPTStock;

				//tagZJOPTStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagZJOPTStock.Code, pSnapData->szCode,32);
				tagZJOPTStock.High = pSnapData->dHighPx;						//��߼۸�[* �Ŵ���]
				tagZJOPTStock.Open = pSnapData->dOpenPx;						//���̼۸�[* �Ŵ���]
				tagZJOPTStock.Low = pSnapData->dLowPx;						//��ͼ۸�[* �Ŵ���]
				tagZJOPTStock.PreClose = pSnapData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
				tagZJOPTStock.PreSettlePrice = pSnapData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
				
				tagZJOPTStock.Now = pSnapData->dNowPx;                    //���¼۸�[* �Ŵ���]
				tagZJOPTStock.Close	= pSnapData->dClosePx;                  //�������̼۸�[* �Ŵ���]
				tagZJOPTStock.SettlePrice = pSnapData->dSettlePx;        //���ս���۸�[* �Ŵ���]
				tagZJOPTStock.UpperPrice = pSnapData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagZJOPTStock.LowerPrice = pSnapData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagZJOPTStock.Amount = pSnapData->dAmount;             //�ܳɽ����[Ԫ]
				tagZJOPTStock.Volume = pSnapData->ui64Volume;             //�ܳɽ���[��]
				tagZJOPTStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //���ճֲ���[��]
				tagZJOPTStock.OpenInterest = pSnapData->ui64OpenInterest;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagZJOPTStock.Buy[i].Price = pSnapData->mBid[i].dVPrice;
					tagZJOPTStock.Buy[i].Volume = pSnapData->mBid[i].ui64Volume;
					tagZJOPTStock.Sell[i].Price = pSnapData->mAsk[i].dVPrice;
					tagZJOPTStock.Sell[i].Volume = pSnapData->mAsk[i].ui64Volume;
				}
			}
			else if( XDF_SZOPT == cMarket )//������Ȩ
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
			else if( XDF_CNFOPT == cMarket )//��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
			{
				pSnapData = (tagQUO_SnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFOPTStock;

				//tagCNFOPTStock.Date = pSnapData->Date;
				//tagCNFOPTStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCNFOPTStock.Code, pSnapData->szCode,20);
				tagCNFOPTStock.High = pSnapData->dHighPx;						//��߼۸�[* �Ŵ���]
				tagCNFOPTStock.Open = pSnapData->dOpenPx;						//���̼۸�[* �Ŵ���]
				tagCNFOPTStock.Low = pSnapData->dLowPx;						//��ͼ۸�[* �Ŵ���]
				tagCNFOPTStock.PreClose = pSnapData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
				tagCNFOPTStock.PreSettlePrice = pSnapData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
				tagCNFOPTStock.Now = pSnapData->dNowPx;                    //���¼۸�[* �Ŵ���]
				tagCNFOPTStock.Close = pSnapData->dClosePx;                  //�������̼۸�[* �Ŵ���]
				tagCNFOPTStock.SettlePrice = pSnapData->dSettlePx;        //���ս���۸�[* �Ŵ���]
				tagCNFOPTStock.UpperPrice = pSnapData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagCNFOPTStock.LowerPrice = pSnapData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
				tagCNFOPTStock.Amount = pSnapData->dAmount;             //�ܳɽ����[Ԫ]
				tagCNFOPTStock.Volume = pSnapData->ui64Volume;             //�ܳɽ���[��]
				tagCNFOPTStock.PreOpenInterest = pSnapData->ui64PreOpenInterest; //���ճֲ���[��]
				tagCNFOPTStock.OpenInterest = pSnapData->ui64OpenInterest;       //�ֲ���[��]

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
/*	if (FuncNo ==100)		//��ȡĳ���г����г����ں��г�ʱ��(����:uint8*,   XDFAPI_MarketStatusInfo*)
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
	if (FuncNo ==101)		//��ȡĳ���г����г����ں��г�ʱ��(����:uint8*,   XDFAPI_MarketStatusInfo*)
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
	else if (FuncNo == 101)		//��ȡ ��ǰ����Ʒ�ڻ�����Ʒ��Ȩ�����ص���14 ����35, ����(-1) ??( ����:int*  )
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
	case XDF_CF://�н��ڻ�
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
			tagData.High = pData->dHighPx;						//��߼۸�[* �Ŵ���]
			tagData.Open = pData->dOpenPx;						//���̼۸�[* �Ŵ���]
			tagData.Low = pData->dLowPx;						//��ͼ۸�[* �Ŵ���]
			tagData.PreClose = pData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
			tagData.Now = pData->dNowPx;                    //���¼۸�[* �Ŵ���]
			tagData.Close = pData->dClosePx;                  //�������̼۸�[* �Ŵ���]
			tagData.SettlePrice	= pData->dSettlePx;      //���ս���۸�[* �Ŵ���]
			tagData.UpperPrice = pData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.LowerPrice = pData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.Amount = pData->dAmount;             //�ܳɽ����[Ԫ]
			tagData.Volume = pData->ui64Volume;             //�ܳɽ���[��]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //���ճֲ���[��]
			tagData.OpenInterest = pData->ui64OpenInterest;       //�ֲ���[��]

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
	case XDF_CNF://��Ʒ�ڻ�(�Ϻ�/֣��/����)
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
			tagData.High = pData->dHighPx;						//��߼۸�[* �Ŵ���]
			tagData.Open = pData->dOpenPx;						//���̼۸�[* �Ŵ���]
			tagData.Low = pData->dLowPx;						//��ͼ۸�[* �Ŵ���]
			tagData.PreClose = pData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
			tagData.Now = pData->dNowPx;                    //���¼۸�[* �Ŵ���]
			tagData.Close = pData->dClosePx;                  //�������̼۸�[* �Ŵ���]
			tagData.SettlePrice	 = pData->dSettlePx;        //���ս���۸�[* �Ŵ���]
			tagData.UpperPrice	 = pData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.LowerPrice	 = pData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.Amount		 = pData->dAmount;             //�ܳɽ����[Ԫ]
			tagData.Volume		 = pData->ui64Volume;             //�ܳɽ���[��]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //���ճֲ���[��]
			tagData.OpenInterest = pData->ui64OpenInterest;       //�ֲ���[��]

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
	case XDF_SHOPT://��֤��Ȩ
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
	case XDF_ZJOPT://�н���Ȩ
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
			tagData.High = pData->dHighPx;						//��߼۸�[* �Ŵ���]
			tagData.Open = pData->dOpenPx;						//���̼۸�[* �Ŵ���]
			tagData.Low = pData->dLowPx;						//��ͼ۸�[* �Ŵ���]
			tagData.PreClose = pData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
			
			tagData.Now = pData->dNowPx;                    //���¼۸�[* �Ŵ���]
			tagData.Close	= pData->dClosePx;                  //�������̼۸�[* �Ŵ���]
			tagData.SettlePrice = pData->dSettlePx;        //���ս���۸�[* �Ŵ���]
			tagData.UpperPrice = pData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.LowerPrice = pData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.Amount = pData->dAmount;             //�ܳɽ����[Ԫ]
			tagData.Volume = pData->ui64Volume;             //�ܳɽ���[��]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //���ճֲ���[��]
			tagData.OpenInterest = pData->ui64OpenInterest;       //�ֲ���[��]

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
	case XDF_SZOPT://������Ȩ
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
	case XDF_CNFOPT://��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
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
			tagData.High = pData->dHighPx;						//��߼۸�[* �Ŵ���]
			tagData.Open = pData->dOpenPx;						//���̼۸�[* �Ŵ���]
			tagData.Low = pData->dLowPx;						//��ͼ۸�[* �Ŵ���]
			tagData.PreClose = pData->dPreClosePx;				//���ռ۸�[* �Ŵ���]
			tagData.PreSettlePrice = pData->dPreSettlePx;	//���ս���۸�[* �Ŵ���]
			tagData.Now = pData->dNowPx;                    //���¼۸�[* �Ŵ���]
			tagData.Close = pData->dClosePx;                  //�������̼۸�[* �Ŵ���]
			tagData.SettlePrice = pData->dSettlePx;        //���ս���۸�[* �Ŵ���]
			tagData.UpperPrice = pData->dUpperLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.LowerPrice = pData->dLowerLimitPx;         //��ͣ�۸�[* �Ŵ���]
			tagData.Amount = pData->dAmount;             //�ܳɽ����[Ԫ]
			tagData.Volume = pData->ui64Volume;             //�ܳɽ���[��]
			tagData.PreOpenInterest = pData->ui64PreOpenInterest; //���ճֲ���[��]
			tagData.OpenInterest = pData->ui64OpenInterest;       //�ֲ���[��]

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

	//[3]�������(PkgHead+PkgBuf)
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




