


#include "stdafx.h"
#include "DataClient.h"
#include "../GlobalIO/GlobalIO.h"

CTYPENAMEFUNC  QuoteClientApi * STDCALL CreateQuoteApi(const char *pszDebugPath)
{
	if (!Global_bInit)
	{
		Global_bInit = true;
		Global_StartWork();
	}
	return &Global_Client;
}

CTYPENAMEFUNC  QuotePrimeApi * STDCALL CreatePrimeApi()
{
	return &Global_PrimeClient;
}

CTYPENAMEFUNC  const char * STDCALL GetDllVersion(int &nMajorVersion, int &nMinorVersion)
{
	
	static char szbuf[255]={0};
	_snprintf(szbuf,254,"V%.02d B%.02d", Global_MajorVer,Global_MinorVer);
	nMajorVersion = Global_MajorVer;
	nMinorVersion = Global_MinorVer;
	
	return szbuf;
}

CTYPENAMEFUNC  int 	STDCALL GetSettingInfo(tagQuoteSettingInfo* pArrMarket, int nCount)
{
	Global_Option.Instance();
	int nrealcount = Global_Option.GetKeyFileCount();
	if (0 == pArrMarket)
	{
		return nrealcount;
	}
	
	tagKeyFileInfo oInfo;
	int ncopycount = min(nrealcount, nCount);
	for (int i=0; i<ncopycount; i++)
	{
		Global_Option.GetKeyFileInfo(i, oInfo);
		pArrMarket[i].cMarketID = oInfo.nMarketID;
		strcpy(pArrMarket[i].cMarketChn, oInfo.cMarketChn);
		strcpy(pArrMarket[i].cAddress, oInfo.cAddress);
		pArrMarket[i].nStatus = Global_DllMgr.GetMarketStat(oInfo.nMarketID);
	}
	return nrealcount;
}
//....................................................................................................................................................................................................................................................................
//....................................................................................................................................................................................................................................................................
//....................................................................................................................................................................................................................................................................
//....................................................................................................................................................................................................................................................................
MDataClient::MDataClient()
{
	
}

MDataClient::~MDataClient()
{

}

int32_t STDCALL	MDataClient::Init()
{
	if (!Global_bInit)
	{
		Global_bInit = true;
		Global_StartWork();
	}

	return 1;
}

void STDCALL MDataClient::Release()
{
	if (Global_bInit)
	{
		Global_bInit=false;
		Global_EndWork();
		Global_pSpi=0;
	}
}

void STDCALL MDataClient::RegisterSpi(QuoteClientSpi * pspi)
{
	Global_pSpi = pspi;

char pszTmp[64] = { 0 };
::sprintf( pszTmp, "TEST: spi pointer : %x", pspi );
Global_pSpi->XDF_OnRspOutLog( 0, 1, pszTmp );
}

int32_t STDCALL	MDataClient::BeginWork()
{
	int iret = Global_DataIO.Instance(); 
	if (iret <0)
	{
		return  -1;
	}
	iret = Global_DllMgr.BeginWork();
	if (iret<0)
	{
		return -2;
	}
	return iret;
}

void STDCALL MDataClient::EndWork()
{
	Global_DllMgr.EndWork();
	Global_DataIO.Release();
}

int	 STDCALL		MDataClient::GetMarketInfo(unsigned char cMarket, char* pszInBuf, int nInBytes)
{
	int iret = Global_DllMgr.GetMarketInfo(cMarket, pszInBuf, nInBytes);
	return iret;
}

int32_t	STDCALL		MDataClient::GetCodeTable(uint8_t cMarket, char* pszInBuf, int32_t nInBytes, int32_t& nCount)
{
	int iret = Global_DllMgr.GetCodeTable(cMarket, pszInBuf, nInBytes, nCount);
	return iret;
}


int32_t STDCALL		MDataClient::GetLastMarketDataAll(uint8_t cMarket, char* pszInBuf, int32_t nInBytes)
{
	int iret = Global_DllMgr.GetLastMarketDataAll(cMarket, pszInBuf, nInBytes);
	return iret;
}

int32_t STDCALL		MDataClient::GetMarketStatus(uint8_t cMarket,int32_t& nStatus, uint32_t& ulTime, int64_t * pI64Send, int64_t * pI64Recv)
{
	int iret = Global_DllMgr.GetMarketStatus(cMarket,nStatus, ulTime, pI64Send, pI64Recv);
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
	if (FuncNo ==100)		//获取某个市场的市场日期和市场时间(参数:uint8*,   XDFAPI_MarketStatusInfo*)
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
	}
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








