#include "DataClient.h"
//#include "../GlobalIO/GlobalIO.h"


MDataClient::MDataClient()
{
	
}

MDataClient::~MDataClient()
{

}

int STDCALL	MDataClient::Init()
{
/*	if (!Global_bInit)
	{
		Global_bInit = true;
		Global_StartWork();
	}
*/
	return 1;
}

void STDCALL MDataClient::Release()
{
/*	if (Global_bInit)
	{
		Global_bInit=false;
		Global_EndWork();
		Global_pSpi=0;
	}*/
}

void STDCALL MDataClient::RegisterSpi(QuoteClientSpi * pspi)
{
//	Global_pSpi = pspi;
}

int STDCALL	MDataClient::BeginWork()
{
	int iret = 0;/*Global_DataIO.Instance(); 
	if (iret <0)
	{
		return  -1;
	}
	iret = Global_DllMgr.BeginWork();*/
	if (iret<0)
	{
		return -2;
	}
	return iret;
}

void STDCALL MDataClient::EndWork()
{
//	Global_DllMgr.EndWork();
//	Global_DataIO.Release();
}

int	 STDCALL		MDataClient::GetMarketInfo(unsigned char cMarket, char* pszInBuf, int nInBytes)
{
	int iret = 0;//Global_DllMgr.GetMarketInfo(cMarket, pszInBuf, nInBytes);
	return iret;
}

int	STDCALL		MDataClient::GetCodeTable(unsigned char cMarket, char* pszInBuf, int nInBytes, int& nCount)
{
	int iret = 0;//Global_DllMgr.GetCodeTable(cMarket, pszInBuf, nInBytes, nCount);
	return iret;
}

int STDCALL		MDataClient::GetLastMarketDataAll(unsigned char cMarket, char* pszInBuf, int nInBytes)
{
	int iret = 0;//Global_DllMgr.GetLastMarketDataAll(cMarket, pszInBuf, nInBytes);
	return iret;
}

int STDCALL		MDataClient::GetMarketStatus(unsigned char cMarket,int& nStatus, unsigned int& ulTime, __int64 * pI64Send, __int64 * pI64Recv)
{
	int iret = 0;//Global_DllMgr.GetMarketStatus(cMarket,nStatus, ulTime, pI64Send, pI64Recv);
	return iret;
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

}

void QuotationAdaptor::OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{

}

void QuotationAdaptor::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{

}





