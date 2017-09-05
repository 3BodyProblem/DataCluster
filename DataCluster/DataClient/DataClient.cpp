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

		if( 0 != Activate( &Global_CBAdaptor ) )
		{
			return -1;
		}
	}

	return 1;
}

void STDCALL MDataClient::Release()
{
	if( Global_bInit )
	{
		Global_bInit = false;
		Destroy();
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
	tagQuoMarketInfo			tagMkInfo = { 0 };
	MStreamWrite				oMSW( pszInBuf, nInBytes );
	DatabaseAdaptor&			refDatabase = DataIOEngine::GetEngineObj().GetDatabaseObj();

	if( nInnerMkID < 0 )
	{
		return -1;
	}

	if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(tagQuoMarketInfo), nSerialNo ) <= 0 )
	{
		return -2;
	}

	oHead.WareCount = tagMkInfo.WareCount;
	oHead.KindCount = tagMkInfo.KindCount;
	oMSW.PutSingleMsg( 100, (char*)&oHead, sizeof(oHead) );

	for( int n = 0; n < tagMkInfo.KindCount; n++ )
	{
		tagQuoCategory			tagCategory = { 0 };
		XDFAPI_MarketKindInfo	oInfo = { 0 };

		if( refDatabase.QueryRecord( (nInnerMkID*100+2), (char*)&tagCategory, sizeof(tagQuoCategory), nSerialNo ) <= 0 )
		{
			return -3;
		}

		oInfo.Serial = n;
		memcpy(oInfo.KindName, tagCategory.KindName, 8);
		oInfo.WareCount = tagCategory.WareCount;
//		oInfo.PriceRate = pMarketdetail->PriceRate;
//		oInfo.LotSize = pMarketdetail->LotSize;
		oMSW.PutMsg( 101, (char*)&oInfo, sizeof(oInfo) );
	}

	oMSW.Detach();

	return oMSW.GetOffset();
}

int	STDCALL		MDataClient::GetCodeTable( unsigned char cMarket, char* pszInBuf, int nInBytes, int& nCount )
{
	int							nInnerMkID = DataCollectorPool::MkIDCast( cMarket );
	unsigned __int64			nSerialNo = 0;
	tagQuoMarketInfo			tagMkInfo = { 0 };
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

	if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(tagQuoMarketInfo), nSerialNo ) <= 0 )
	{
		return -2;
	}

	nCount = tagMkInfo.WareCount;
	if( 0 == pszInBuf || 0 == nInBytes )		///< ͨ��nCount����������
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
		tagQuoReferenceData*		pRefData = (tagQuoReferenceData*)m_pQueryBuffer;
		if( (nDataSize=refDatabase.QueryBatchRecords( (nInnerMkID*100+3), m_pQueryBuffer, 1024*1024*10, nSerialNo )) <= 0 )
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
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagShL1Name;
				tagShL1Name.Market = XDF_SH;
				memcpy( tagShL1Name.Code, pRefData->Code, 6 );
				memcpy( tagShL1Name.Name, pRefData->Name, 8 );
				tagShL1Name.SecKind = pRefData->Kind;
			}
			else if( XDF_CF == cMarket )//�н��ڻ�
			{
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJQHName;
				tagZJQHName.Market = XDF_CF;
				memcpy( tagZJQHName.Code, pRefData->Code, 6 );
				tagZJQHName.SecKind = pRefData->Kind;
				tagZJQHName.ContractMult = pRefData->ContractMult;	///< ��Լ����
				//tagZJQHName.ExFlag = pRefData->ExFlag;			///< ������ձ��,0x01��ʾ���������ֻ����ͨ��Լ��Ч������ֵ��δ����
				strncpy(tagZJQHName.Name, pRefData->Name, 8);
				//tagZJQHName->ObjectMId = pRefData->ObjectMId;		///< ���ָ���г����[�μ������ֵ�-�г����]��0xFF��ʾδ֪
				strncpy(tagZJQHName.ObjectCode, pRefData->UnderlyingCode,6);
			}
			else if( XDF_CNF == cMarket )//��Ʒ�ڻ�(�Ϻ�/֣��/����)
			{
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfName;
				tagCnfName.Market = XDF_CNF;
				tagCnfName.SecKind = pRefData->Kind;
				memcpy( tagCnfName.Code,pRefData->Code, 20 );
				memcpy( tagCnfName.Name, pRefData->Name, 40 );
				tagCnfName.LotFactor = pRefData->LotFactor;
			}
			else if( XDF_SHOPT == cMarket )//��֤��Ȩ
			{
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagSHOPTName;
				tagSHOPTName.Market = XDF_SHOPT;
				memcpy(tagSHOPTName.Code, pRefData->Code, 8);
				memcpy(tagSHOPTName.Name, pRefData->Name,20);
				tagSHOPTName.SecKind = pRefData->Kind;
				memcpy(tagSHOPTName.ContractID, pRefData->ContractID, 19);

				if( pRefData->DerivativeType > 1 )
				{
					tagSHOPTName.OptionType = 'E';
					tagSHOPTName.CallOrPut = pRefData->DerivativeType==2 ? 'C' : 'P';
				}
				else
				{
					tagSHOPTName.OptionType = 'A';
					tagSHOPTName.CallOrPut = pRefData->DerivativeType==0 ? 'C' : 'P';
				}

				//tagSHOPTName.PreClosePx = pRefData->C;//��Լ����(������Ȩ��Ϣ��Ϊ����������̼۸�)(��ȷ����)//[*�Ŵ���]
				//tagSHOPTName.PreSettlePx = pRefData->;//��Լ���//[*�Ŵ���]
				//tagSHOPTName.LeavesQty = pRefData->;//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
				memcpy(tagSHOPTName.UnderlyingCode, pRefData->UnderlyingCode, 6);
				//memcpy(tagSHOPTName.UnderlyingName, pRefData-, 6);//���֤ȯ����
				//memcpy(tagSHOPTName.UnderlyingType, pRefData-, 3);//���֤ȯ����(EBS -ETF, ASH -A��)
				//tagSHOPTName.UnderlyingClosePx = ;//���֤ȯ������ //[*�Ŵ���]
				//tagSHOPTName.PriceLimitType = pRefData-//�ǵ�����������(N ���ǵ���)(R ���ǵ���)
				//tagSHOPTName.UpLimit = pRefData->;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				//tagSHOPTName.DownLimit;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				tagSHOPTName.LotSize = pRefData->LotSize;//һ�ֵ��ڼ��ź�Լ
				tagSHOPTName.ContractUnit = pRefData->ContractUnit;//��Լ��λ(������Ȩ��Ϣ������ĺ�Լ��λ��һ��Ϊ����)
				tagSHOPTName.XqPrice = pRefData->XqPrice;//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
				tagSHOPTName.StartDate = pRefData->StartDate;//�׸�������(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->EndDate;//�������(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->XqDate;//��Ȩ��(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->DeliveryDate;//������(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->ExpireDate;//������(YYYYMMDD)
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
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagZJOPTName;
				tagZJOPTName.Market = XDF_ZJOPT;
				memcpy(tagZJOPTName.Code, pRefData->Code, 32);
				tagZJOPTName.SecKind = pRefData->Kind;
				tagZJOPTName.ContractMult = pRefData->ContractMult;//��Լ����
				tagZJOPTName.ContractUnit = pRefData->ContractUnit;	//��Լ��λ
				tagZJOPTName.StartDate = pRefData->StartDate;		//�׸�������(YYYYMMDD)
				tagZJOPTName.EndDate = pRefData->EndDate;				//�������(YYYYMMDD)
				tagZJOPTName.XqDate = pRefData->XqDate;				//��Ȩ��(YYYYMMDD)
				tagZJOPTName.DeliveryDate = pRefData->DeliveryDate;	//������(YYYYMMDD)
				tagZJOPTName.ExpireDate = pRefData->ExpireDate;		//������(YYYYMMDD)
				//pTable->ObjectMId = pNameTb->ObjectMId;
				strncpy( tagZJOPTName.ObjectCode, pRefData->UnderlyingCode,6 );
			}
			else if( XDF_SZOPT == cMarket )//������Ȩ
			{
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagSZOPTName;
				tagSZOPTName.Market = XDF_SZOPT;
				tagSZOPTName.SecKind = pRefData->Kind;
				memcpy(tagSZOPTName.Code, pRefData->Code, 8);
				memcpy(tagSZOPTName.Name, pRefData->Name,20);
				memcpy(tagSZOPTName.ContractID, pRefData->ContractID, 20);
				if( pRefData->DerivativeType > 1 )
				{
					tagSHOPTName.OptionType = 'E';
					tagSHOPTName.CallOrPut = pRefData->DerivativeType==2 ? 'C' : 'P';
				}
				else
				{
					tagSHOPTName.OptionType = 'A';
					tagSHOPTName.CallOrPut = pRefData->DerivativeType==0 ? 'C' : 'P';
				}
				//tagSHOPTName.PreClosePx = pRefData->C;//��Լ����(������Ȩ��Ϣ��Ϊ����������̼۸�)(��ȷ����)//[*�Ŵ���]
				//tagSHOPTName.PreSettlePx = pRefData->;//��Լ���//[*�Ŵ���]
				//tagSHOPTName.LeavesQty = pRefData->;//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
				memcpy(tagSHOPTName.UnderlyingCode, pRefData->UnderlyingCode, 6);
				//tagSHOPTName.UpLimit = pRefData->;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				//tagSHOPTName.DownLimit;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				tagSHOPTName.LotSize = pRefData->LotSize;//һ�ֵ��ڼ��ź�Լ
				tagSHOPTName.ContractUnit = pRefData->ContractUnit;//��Լ��λ(������Ȩ��Ϣ������ĺ�Լ��λ��һ��Ϊ����)
				tagSHOPTName.XqPrice = pRefData->XqPrice;//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
				tagSHOPTName.StartDate = pRefData->StartDate;//�׸�������(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->EndDate;//�������(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->XqDate;//��Ȩ��(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->DeliveryDate;//������(YYYYMMDD)
				tagSHOPTName.ExpireDate = pRefData->ExpireDate;//������(YYYYMMDD)
				//tagSHOPTName.MarginUnit = pNameTb->MarginUnit;//��λ��֤��(��ȷ����)//[*�Ŵ�100]
			}
			else if( XDF_CNFOPT == cMarket )//��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
			{
				pRefData = (tagQuoReferenceData*)(m_pQueryBuffer + nOffset);

				pData = (char*)&tagCnfOPTName;
				tagCnfOPTName.Market = XDF_CNFOPT;
				tagCnfOPTName.SecKind = pRefData->Kind;
				memcpy(tagCnfOPTName.Code,pRefData->Code, 20);
				memcpy(tagCnfOPTName.Name, pRefData->Name, 40);
				tagCnfOPTName.LotFactor = pRefData->LotFactor;
				//tagSHOPTName.LeavesQty = pRefData->;//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
				memcpy(tagSHOPTName.UnderlyingCode, pRefData->UnderlyingCode, 6);
				//pTable->PriceLimitType = pNameTb->PriceLimitType;
				//tagSHOPTName.UpLimit = pRefData->;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				//tagSHOPTName.DownLimit;//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
				tagSHOPTName.LotSize = pRefData->LotSize;//һ�ֵ��ڼ��ź�Լ
				tagSHOPTName.ContractUnit = pRefData->ContractUnit;//��Լ��λ(������Ȩ��Ϣ������ĺ�Լ��λ��һ��Ϊ����)
				tagSHOPTName.XqPrice = pRefData->XqPrice;//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
				tagSHOPTName.StartDate = pRefData->StartDate;//�׸�������(YYYYMMDD)
				tagSHOPTName.EndDate = pRefData->EndDate;//�������(YYYYMMDD)
				tagSHOPTName.XqDate = pRefData->XqDate;//��Ȩ��(YYYYMMDD)
				tagSHOPTName.DeliveryDate = pRefData->DeliveryDate;//������(YYYYMMDD)
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
	tagQuoMarketInfo			tagMkInfo = { 0 };
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

	if( refDatabase.QueryRecord( (nInnerMkID*100+1), (char*)&tagMkInfo, sizeof(tagQuoMarketInfo), nSerialNo ) <= 0 )
	{
		return -2;
	}

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
		tagQuoSnapData*			pSnapData = (tagQuoSnapData*)m_pQueryBuffer;
		if( (nDataSize=refDatabase.QueryBatchRecords( (nInnerMkID*100+4), m_pQueryBuffer, 1024*1024*10, nSerialNo )) <= 0 )
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
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSHL1Stock;

				memcpy(tagSHL1Stock.Code, pSnapData->Code,6);
				tagSHL1Stock.High = pSnapData->High;
				tagSHL1Stock.Open = pSnapData->Open;
				tagSHL1Stock.Low = pSnapData->Low;
				tagSHL1Stock.PreClose = pSnapData->Close;
				tagSHL1Stock.Now = pSnapData->Now;
				tagSHL1Stock.Amount = pSnapData->Amount;
				tagSHL1Stock.Volume = pSnapData->Volume;
				//tagSHL1Stock.Records = pSnapData->Records;
				//tagSHL1Stock.HighLimit = pSnapData->HighLimit;
				//tagSHL1Stock.LowLimit = pSnapData->LowLimit;
				//tagSHL1Stock.Voip = pSnapData->Voip;

				for (int i=0; i<5; i++)
				{
					tagSHL1Stock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagSHL1Stock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagSHL1Stock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagSHL1Stock.Sell[i].Volume = pSnapData->Sell[i].Volume;
				}
			}
			else if( XDF_CF == cMarket )//�н��ڻ�
			{
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCFFStock;

				//tagCFFStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCFFStock.Code, pSnapData->Code,6);
				tagCFFStock.High = pSnapData->High;						//��߼۸�[* �Ŵ���]
				tagCFFStock.Open = pSnapData->Open;						//���̼۸�[* �Ŵ���]
				tagCFFStock.Low = pSnapData->Low;						//��ͼ۸�[* �Ŵ���]
				tagCFFStock.PreClose = pSnapData->PreClose;				//���ռ۸�[* �Ŵ���]
				tagCFFStock.PreSettlePrice = pSnapData->PreSettlePrice;	//���ս���۸�[* �Ŵ���]
				tagCFFStock.Now = pSnapData->Now;                    //���¼۸�[* �Ŵ���]
				tagCFFStock.Close = pSnapData->Close;                  //�������̼۸�[* �Ŵ���]
				tagCFFStock.SettlePrice	= pSnapData->SettlePrice;      //���ս���۸�[* �Ŵ���]
				tagCFFStock.UpperPrice = pSnapData->UpperPrice;         //��ͣ�۸�[* �Ŵ���]
				tagCFFStock.LowerPrice = pSnapData->LowerPrice;         //��ͣ�۸�[* �Ŵ���]
				tagCFFStock.Amount = pSnapData->Amount;             //�ܳɽ����[Ԫ]
				tagCFFStock.Volume = pSnapData->Volume;             //�ܳɽ���[��]
				tagCFFStock.PreOpenInterest = pSnapData->PreOpenInterest; //���ճֲ���[��]
				tagCFFStock.OpenInterest = pSnapData->Position;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagCFFStock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagCFFStock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagCFFStock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagCFFStock.Sell[i].Volume = pSnapData->Sell[i].Volume;
				}
			}
			else if( XDF_CNF == cMarket )//��Ʒ�ڻ�(�Ϻ�/֣��/����)
			{
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFStock;

				//tagCNFStock.Date = pSnapData->Date;
				//tagCNFStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCNFStock.Code, pSnapData->Code,20);
				tagCNFStock.High = pSnapData->High;						//��߼۸�[* �Ŵ���]
				tagCNFStock.Open = pSnapData->Open;						//���̼۸�[* �Ŵ���]
				tagCNFStock.Low = pSnapData->Low;						//��ͼ۸�[* �Ŵ���]
				tagCNFStock.PreClose = pSnapData->PreClose;				//���ռ۸�[* �Ŵ���]
				tagCNFStock.PreSettlePrice = pSnapData->PreSettlePrice;	//���ս���۸�[* �Ŵ���]
				tagCNFStock.Now = pSnapData->Now;                    //���¼۸�[* �Ŵ���]
				tagCNFStock.Close = pSnapData->Close;                  //�������̼۸�[* �Ŵ���]
				tagCNFStock.SettlePrice	 = pSnapData->SettlePrice;        //���ս���۸�[* �Ŵ���]
				tagCNFStock.UpperPrice	 = pSnapData->UpperPrice;         //��ͣ�۸�[* �Ŵ���]
				tagCNFStock.LowerPrice	 = pSnapData->LowerPrice;         //��ͣ�۸�[* �Ŵ���]
				tagCNFStock.Amount		 = pSnapData->Amount;             //�ܳɽ����[Ԫ]
				tagCNFStock.Volume		 = pSnapData->Volume;             //�ܳɽ���[��]
				tagCNFStock.PreOpenInterest = pSnapData->PreOpenInterest; //���ճֲ���[��]
				tagCNFStock.OpenInterest = pSnapData->Position;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagCNFStock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagCNFStock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagCNFStock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagCNFStock.Sell[i].Volume = pSnapData->Sell[i].Volume;
				}
			}
			else if( XDF_SHOPT == cMarket )//��֤��Ȩ
			{
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSHOPTStock;

				memcpy(tagSHOPTStock.Code, pSnapData->Code,8);
				//tagSHOPTStock.Time = pSnapData->DataTimeStamp;
				tagSHOPTStock.PreSettlePx = pSnapData->PreSettlePrice;
				tagSHOPTStock.SettlePrice = pSnapData->SettlePrice;
				tagSHOPTStock.OpenPx = pSnapData->Open;
				tagSHOPTStock.HighPx = pSnapData->High;
				tagSHOPTStock.LowPx = pSnapData->Low;
				tagSHOPTStock.Now = pSnapData->Now;
				tagSHOPTStock.Volume = pSnapData->Volume;
				tagSHOPTStock.Amount = pSnapData->Amount;
				memcpy(tagSHOPTStock.TradingPhaseCode, pSnapData->TradingPhaseCode,4);
				//tagSHOPTStock.AuctionPrice = pSnapData->AuctionPrice;
				//tagSHOPTStock.AuctionQty = pSnapData->AuctionQty;
				tagSHOPTStock.Position = pSnapData->Position;
				for (int i=0; i<5; i++)
				{
					tagSHOPTStock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagSHOPTStock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagSHOPTStock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagSHOPTStock.Sell[i].Volume = pSnapData->Sell[i].Volume;
				}
			}
			else if( XDF_ZJOPT == cMarket )//�н���Ȩ
			{
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagZJOPTStock;

				//tagZJOPTStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagZJOPTStock.Code, pSnapData->Code,32);
				tagZJOPTStock.High = pSnapData->High;						//��߼۸�[* �Ŵ���]
				tagZJOPTStock.Open = pSnapData->Open;						//���̼۸�[* �Ŵ���]
				tagZJOPTStock.Low = pSnapData->Low;						//��ͼ۸�[* �Ŵ���]
				tagZJOPTStock.PreClose = pSnapData->PreClose;				//���ռ۸�[* �Ŵ���]
				tagZJOPTStock.PreSettlePrice = pSnapData->PreSettlePrice;	//���ս���۸�[* �Ŵ���]
				
				tagZJOPTStock.Now = pSnapData->Now;                    //���¼۸�[* �Ŵ���]
				tagZJOPTStock.Close	= pSnapData->Close;                  //�������̼۸�[* �Ŵ���]
				tagZJOPTStock.SettlePrice = pSnapData->SettlePrice;        //���ս���۸�[* �Ŵ���]
				tagZJOPTStock.UpperPrice = pSnapData->UpperPrice;         //��ͣ�۸�[* �Ŵ���]
				tagZJOPTStock.LowerPrice = pSnapData->LowerPrice;         //��ͣ�۸�[* �Ŵ���]
				tagZJOPTStock.Amount = pSnapData->Amount;             //�ܳɽ����[Ԫ]
				tagZJOPTStock.Volume = pSnapData->Volume;             //�ܳɽ���[��]
				tagZJOPTStock.PreOpenInterest = pSnapData->PreOpenInterest; //���ճֲ���[��]
				tagZJOPTStock.OpenInterest = pSnapData->Position;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagSHOPTStock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagSHOPTStock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagSHOPTStock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagSHOPTStock.Sell[i].Volume = pSnapData->Sell[i].Volume;
				}
			}
			else if( XDF_SZOPT == cMarket )//������Ȩ
			{
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagSZOPTStock;

				memcpy(tagSZOPTStock.Code, pSnapData->Code,8);
				//tagSZOPTStock.Time = pSnapData->DataTimeStamp;
				tagSZOPTStock.PreSettlePx = pSnapData->PreSettlePrice;
				tagSZOPTStock.SettlePrice = pSnapData->SettlePrice;
				tagSZOPTStock.OpenPx = pSnapData->Open;
				tagSZOPTStock.HighPx = pSnapData->High;
				tagSZOPTStock.LowPx = pSnapData->Low;
				tagSZOPTStock.Now = pSnapData->Now;
				tagSZOPTStock.Volume = pSnapData->Volume;
				tagSZOPTStock.Amount = pSnapData->Amount;
				memcpy(tagSZOPTStock.TradingPhaseCode, pSnapData->TradingPhaseCode,4);
				tagSZOPTStock.Position = pSnapData->Position;
				for (int i=0; i<5; i++)
				{
					tagSHOPTStock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagSHOPTStock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagSHOPTStock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagSHOPTStock.Sell[i].Volume = pSnapData->Sell[i].Volume;
				}
			}
			else if( XDF_CNFOPT == cMarket )//��Ʒ�ڻ�����Ʒ��Ȩ(�Ϻ�/֣��/����)
			{
				pSnapData = (tagQuoSnapData*)(m_pQueryBuffer + nOffset);
				pData = (char*)&tagCNFOPTStock;

				//tagCNFOPTStock.Date = pSnapData->Date;
				//tagCNFOPTStock.DataTimeStamp = pSnapData->DataTimeStamp;
				memcpy(tagCNFOPTStock.Code, pSnapData->Code,20);
				tagCNFOPTStock.High = pSnapData->High;						//��߼۸�[* �Ŵ���]
				tagCNFOPTStock.Open = pSnapData->Open;						//���̼۸�[* �Ŵ���]
				tagCNFOPTStock.Low = pSnapData->Low;						//��ͼ۸�[* �Ŵ���]
				tagCNFOPTStock.PreClose = pSnapData->PreClose;				//���ռ۸�[* �Ŵ���]
				tagCNFOPTStock.PreSettlePrice = pSnapData->PreSettlePrice;	//���ս���۸�[* �Ŵ���]
				tagCNFOPTStock.Now = pSnapData->Now;                    //���¼۸�[* �Ŵ���]
				tagCNFOPTStock.Close = pSnapData->Close;                  //�������̼۸�[* �Ŵ���]
				tagCNFOPTStock.SettlePrice = pSnapData->SettlePrice;        //���ս���۸�[* �Ŵ���]
				tagCNFOPTStock.UpperPrice = pSnapData->UpperPrice;         //��ͣ�۸�[* �Ŵ���]
				tagCNFOPTStock.LowerPrice = pSnapData->LowerPrice;         //��ͣ�۸�[* �Ŵ���]
				tagCNFOPTStock.Amount = pSnapData->Amount;             //�ܳɽ����[Ԫ]
				tagCNFOPTStock.Volume = pSnapData->Volume;             //�ܳɽ���[��]
				tagCNFOPTStock.PreOpenInterest = pSnapData->PreOpenInterest; //���ճֲ���[��]
				tagCNFOPTStock.OpenInterest = pSnapData->Position;       //�ֲ���[��]

				for (int i=0; i<5; i++)
				{
					tagSHOPTStock.Buy[i].Price = pSnapData->Buy[i].Price;
					tagSHOPTStock.Buy[i].Volume = pSnapData->Buy[i].Volume;
					tagSHOPTStock.Sell[i].Price = pSnapData->Sell[i].Price;
					tagSHOPTStock.Sell[i].Volume = pSnapData->Sell[i].Volume;
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


void QuotationAdaptor::OnQuotation( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{
	if( NULL == Global_pSpi )
	{
		return;
	}

	switch( nMessageID )
	{
	case 151:
		{
			XDFAPI_MarketStatusInfo			tagData = { 0 };
			tagSHL1MarketStatus_HF151*		pData = (tagSHL1MarketStatus_HF151*)pDataPtr;
/*
			tagData.MarketDate = pData->MarketDate;
			tagData.MarketTime = pData->MarketTime;
			tagData.MarketID = XDF_SH;
			tagData.MarketStatus = pData->MarketStatus;
*/
		}
		break;
	}

}

void QuotationAdaptor::OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{

}

void QuotationAdaptor::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{

}





