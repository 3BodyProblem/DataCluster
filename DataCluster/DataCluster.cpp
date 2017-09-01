#include "targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCluster.h"
#include "UnitTest/UnitTest.h"
#include "DataCenterEngine/DataCenterEngine.h"


extern "C"
{
	__declspec(dllexport) int __stdcall		Activate( I_QuotationCallBack* pIDataHandle )
	{
		return DataIOEngine::GetEngineObj().Initialize( pIDataHandle );
	}

	__declspec(dllexport) void __stdcall	Destroy()
	{
		DataIOEngine::GetEngineObj().Release();
	}

	__declspec(dllexport) int __stdcall	Query( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
	{
		return DataIOEngine::GetEngineObj().OnQuery( nMessageID, pDataPtr, nDataLen );
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

	__declspec(dllexport) QuoteClientApi*	CreateQuoteApi( const char* pszDebugPath )
	{
/*		if (!Global_bInit)
		{
			Global_bInit = true;
			Global_StartWork();
		}
		return &Global_Client;*/
		return NULL;
	}
}




