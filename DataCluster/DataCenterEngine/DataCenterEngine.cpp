#pragma warning(disable:4996)
#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "../DataCluster.h"
#include "DataCenterEngine.h"
#include "../DataNodeWrapper/NodeWrapper.h"
#include "../DataClientWrapper/ClientWrapper.h"


DataIOEngine::DataIOEngine()
: SimpleTask( "DataIOEngine::Thread" )
{
}

DataIOEngine::~DataIOEngine()
{
	Release();
}

DataIOEngine& DataIOEngine::GetEngineObj()
{
	static DataIOEngine	obj;

	return obj;
}

DataCollectorPool& DataIOEngine::GetCollectorPool()
{
	return m_oDataCollectorPool;
}

int DataIOEngine::Initialize( I_DataHandle* pIDataHandle )
{
	int			nErrorCode = 0;

	Release();
	if( 0 != (nErrorCode = TableFillerRegister::GetRegister().Initialize()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 register message filler 2 table, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 >= (nErrorCode = m_oDataCollectorPool.Initialize( pIDataHandle )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = SimpleTask::Activate()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize task thread, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	DataIOEngine::GetEngineObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initialized ......" );
	return nErrorCode;
}

void DataIOEngine::Release()
{
	SimpleTask::StopThread();
	SimpleTask::Join( 5000 );
	m_oDataCollectorPool.Release();
}

int DataIOEngine::Execute()
{
	DataIOEngine::GetEngineObj().WriteInfo( "DataIOEngine::Execute() : enter into thread func ..." );

	while( true == IsAlive() )
	{
		try
		{
			SimpleTask::Sleep( 1000*2 );	///< 一秒循环一次

			m_oDataCollectorPool.PreserveAllConnection();
		}
		catch( std::exception& err )
		{
			DataIOEngine::GetEngineObj().WriteWarning( "DataIOEngine::Execute() : exception : %s", err.what() );
		}
		catch( ... )
		{
			DataIOEngine::GetEngineObj().WriteWarning( "DataIOEngine::Execute() : unknow exception" );
		}
	}

	DataIOEngine::GetEngineObj().WriteInfo( "DataIOEngine::Execute() : exit thread func ..." );

	return 0;
}

void DataIOEngine::WriteInfo( const char * szFormat,... )
{
	va_list						stmarker;
	char						tempbuf[8192] = { 0 };

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == EngineWrapper4DataClient::GetObj().IsUsed() )
	{
		EngineWrapper4DataClient::GetObj().OnLog( 0, tempbuf );
	}
	else if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		EngineWrapper4DataNode::GetObj().OnLog( 0, tempbuf );
	}
	else
	{
		::printf( "%s\n", tempbuf );
	}
}

void DataIOEngine::WriteWarning( const char * szFormat,... )
{
	va_list						stmarker;
	char						tempbuf[8192] = { 0 };

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == EngineWrapper4DataClient::GetObj().IsUsed() )
	{
		EngineWrapper4DataClient::GetObj().OnLog( 2, tempbuf );
	}
	else if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		EngineWrapper4DataNode::GetObj().OnLog( 2, tempbuf );
	}
	else
	{
		::printf( "%s\n", tempbuf );
	}
}

void DataIOEngine::WriteError( const char * szFormat,... )
{
	va_list						stmarker;
	char						tempbuf[8192] = { 0 };

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == EngineWrapper4DataClient::GetObj().IsUsed() )
	{
		EngineWrapper4DataClient::GetObj().OnLog( 3, tempbuf );
	}
	else if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		EngineWrapper4DataNode::GetObj().OnLog( 3, tempbuf );
	}
	else
	{
		::printf( "%s\n", tempbuf );
	}
}

void DataIOEngine::WriteDetail( const char * szFormat,... )
{
	va_list						stmarker;
	char						tempbuf[8192] = { 0 };

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == EngineWrapper4DataClient::GetObj().IsUsed() )
	{
		EngineWrapper4DataClient::GetObj().OnLog( 4, tempbuf );
	}
	else if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		EngineWrapper4DataNode::GetObj().OnLog( 4, tempbuf );
	}
	else
	{
		::printf( "%s\n", tempbuf );
	}
}






