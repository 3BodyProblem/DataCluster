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
		return DataIOEngine::GetEngineObj().Initialize( pIDataHandle );
	}

	__declspec(dllexport) void __stdcall Destroy()
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

}




