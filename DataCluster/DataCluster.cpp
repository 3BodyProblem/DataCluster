#include "targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCluster.h"
#include "UnitTest/UnitTest.h"
#include "DataCenterEngine/DataCenterEngine.h"


extern "C"
{
	__declspec(dllexport) int __stdcall	Activate( I_QuotationCallBack* pIDataHandle )
	{
		return 0;//QuoCollector::GetCollector().Initialize( pIDataHandle );
	}

	__declspec(dllexport) void __stdcall Destroy()
	{
		//QuoCollector::GetCollector().Release();
	}

	__declspec(dllexport) int __stdcall	GetStatus( char* pszStatusDesc, unsigned int& nStrLen )
	{
		return 0;//QuoCollector::GetCollector().GetCollectorStatus( pszStatusDesc, nStrLen );
	}

	__declspec(dllexport) void __stdcall	ExecuteUnitTest()
	{
		::printf( "\n\n---------------------- [Begin] -------------------------\n" );
		//ExecuteTestCase();
		::printf( "----------------------  [End]  -------------------------\n\n\n" );
	}

}




