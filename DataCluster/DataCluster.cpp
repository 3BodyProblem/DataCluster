#include "targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCluster.h"
#include "UnitTest/UnitTest.h"
#include "DataCenterEngine/DataCenterEngine.h"
#include "QuoteClientApi.h"


extern "C"
{
	__declspec(dllexport) int __stdcall		GetVersionNo()
	{
		unsigned int	nMajor = 1;
		unsigned int	nRelease = 1;
		unsigned int	nBuild = 1;

		return 1000000 * nMajor + 100000 * nRelease + 1000 * nBuild;
	}

	__declspec(dllexport) int __stdcall		StartWork( I_QuotationCallBack* pIDataHandle )
	{
		return DataIOEngine::GetEngineObj().Initialize( pIDataHandle );
	}

	__declspec(dllexport) void __stdcall	EndWork()
	{
		DataIOEngine::GetEngineObj().Release();
	}

	__declspec(dllexport) int  __stdcall	GetMarketID( QUO_MARKET_ID* lpOut,unsigned int uiSize )
	{
		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetMarketInfo( QUO_MARKET_ID eMarketID, tagQUO_MarketInfo* lpOut )
	{
		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetAllReferenceData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_ReferenceData* lpOut, unsigned int uiSize )
	{
		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetReferenceData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_ReferenceData* lpOut )
	{
		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetAllSnapData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_SnapData* lpOut, unsigned int uiSize )
	{
		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetSnapData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_SnapData* lpOut )
	{
		return 0;
	}

	__declspec(dllexport) void __stdcall	ExecuteUnitTest()
	{
		::printf( "\n\n---------------------- [Begin] -------------------------\n" );
		//ExecuteTestCase();
		::printf( "----------------------  [End]  -------------------------\n\n\n" );
	}

	__declspec(dllexport) const char*		GetDllVersion( int &nMajorVersion, int &nMinorVersion )
	{
		static int		s_nMajorVer = 1;
		static int		s_nMinorVer = 1000;
		static char		pszBuf[255] = { 0 };

		nMajorVersion = s_nMajorVer;
		nMinorVersion = s_nMinorVer;
		_snprintf( pszBuf, 254, "V%.02d B%.02d", s_nMajorVer, s_nMinorVer );

		return pszBuf;
	}

///< ---------------------------------------------------------------------------------------
	MPrimeClient				Global_PrimeClient;
	bool						Global_bInit = false;
	MDataClient					Global_Client;
	QuotationAdaptor			Global_CBAdaptor;
	QuoteClientSpi*				Global_pSpi = NULL;

	__declspec(dllexport) QuoteClientApi*	CreateQuoteApi( const char* pszDebugPath )
	{
		if (!Global_bInit)
		{
			Global_bInit = true;

			if( 0 != StartWork( &Global_CBAdaptor ) )
			{
				return NULL;
			}
		}

		return &Global_Client;
	}

	__declspec(dllexport) QuotePrimeApi* CreatePrimeApi()
	{
		return &Global_PrimeClient;
	}

	__declspec(dllexport) int	GetSettingInfo( tagQuoteSettingInfo* pArrMarket, int nCount )
	{
		if( 0 != Configuration::GetConfigObj().Load() )
		{
			return -1;
		}

		if( NULL == pArrMarket )
		{
			return -2;
		}

		DllPathTable&		refDllTable = Configuration::GetConfigObj().GetDCPathTable();
		int					nDllNum = min( refDllTable.GetCount(), nCount );

		for( int n = 0; n < nDllNum; n++ )
		{
			::strcpy( pArrMarket[n].cAddress, refDllTable.GetPathByPos( n ).c_str() );

/*	char			szDll[255];
	int				nMarketID;
	int				nSleep;
	char			cMarketChn[64];
	char			cAddress[128];*/
		}

/*
		tagKeyFileInfo oInfo;
		for (int i=0; i<ncopycount; i++)
		{
		Global_Option.GetKeyFileInfo(i, oInfo);
		pArrMarket[i].cMarketID = oInfo.nMarketID;
		strcpy(pArrMarket[i].cMarketChn, oInfo.cMarketChn);
		strcpy(pArrMarket[i].cAddress, oInfo.cAddress);
		pArrMarket[i].nStatus = Global_DllMgr.GetMarketStat(oInfo.nMarketID);
		}*/

		return nDllNum;
	}
}




