#pragma warning(disable:4996)
#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCenterEngine.h"


DataIOEngine::DataIOEngine()
 : SimpleTask( "DataIOEngine::Thread" )
{
}

int DataIOEngine::Initialize()
{
	int			nErrorCode = 0;

	Release();
	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initializing ......" );

	if( 0 != (nErrorCode = m_oDatabaseIO.Initialize()) )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_oDatabaseIO.RecoverDatabase();
	if( 0 != (nErrorCode = m_oDataCollectorPool.Initialize( this )) )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = SimpleTask::Activate()) )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize task thread, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initialized ......" );

	return nErrorCode;
}

void DataIOEngine::Release()
{
	m_oDataCollectorPool.Release();
	m_oDatabaseIO.Release();
	SimpleTask::StopThread();
}

bool DataIOEngine::PrepareQuotation()
{
	int			nErrorCode = 0;

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::PrepareQuotation() : reloading quotation........" );

//	m_oDataCollectorPool.HaltDataCollector();											///< 1) 先事先停止数据采集模块

//	if( 0 != (nErrorCode=m_oDataCollectorPool.RecoverDataCollector()) )					///< 3) 重新初始化行情采集模块
	{
		DataCenterEngine::GetSerivceObj().WriteWarning( "DataIOEngine::PrepareQuotation() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return false;;
	}

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::PrepareQuotation() : quotation reloaded ........" );

	return true;
}

int DataIOEngine::Execute()
{
	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : enter into thread func ..." );

	while( true == IsAlive() )
	{
		try
		{
			SimpleTask::Sleep( 1000*3 );	///< 一秒循环一次

			m_oDataCollectorPool.PreserveAllConnection();
		}
		catch( std::exception& err )
		{
			DataCenterEngine::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : exception : %s", err.what() );
		}
		catch( ... )
		{
			DataCenterEngine::GetSerivceObj().WriteWarning( "DataIOEngine::Execute() : unknow exception" );
		}
	}

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : exit thread func ..." );

	return 0;
}

int DataIOEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	unsigned __int64		nSerialNo = 0;
	static	const char		s_pszZeroBuff[128] = { 0 };

	if( 0 == strncmp( pData, s_pszZeroBuff, sizeof(s_pszZeroBuff) ) )
	{
		return m_oDatabaseIO.FetchRecordsByID( nDataID, pData, nDataLen, nSerialNo );
	}
	else
	{
		return m_oDatabaseIO.QueryQuotation( nDataID, pData, nDataLen, nSerialNo );
	}
}

int DataIOEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	unsigned __int64		nPushSerialNo = 0;				///< 实时行情更新流水

	return m_oDatabaseIO.BuildMessageTable( nDataID, pData, nDataLen, bLastFlag, nPushSerialNo );
}

int DataIOEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	unsigned __int64	nPushSerialNo = 0;				///< 实时行情更新流水
	int					nErrorCode = m_oDatabaseIO.UpdateQuotation( nDataID, pData, nDataLen, nPushSerialNo );

	if( 0 >= nErrorCode )
	{
		return nErrorCode;
	}

	return nErrorCode;
}

void DataIOEngine::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
    va_list		valist;
    char		pszLogBuf[8000] = { 0 };

    va_start( valist, pszFormat );
    _vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
    va_end( valist );

	switch( nLogLevel )	///< 日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	{
	case 0:
//		MServicePlug::WriteInfo( "[Plugin] %s", pszLogBuf );
		break;
	case 1:
//		MServicePlug::WriteWarning( "[Plugin] %s", pszLogBuf );
		break;
	case 2:
//		MServicePlug::WriteError( "[Plugin] %s", pszLogBuf );
		break;
	case 3:
//		MServicePlug::WriteDetail( "[Plugin] %s", pszLogBuf );
		break;
	default:
		::printf( "[Plugin] unknow log level [%d] \n", nLogLevel );
		break;
	}
}


///< ----------------------------------------------------------------------------


DataCenterEngine::DataCenterEngine()
 : m_bActivated( false )
{
}

DataCenterEngine::~DataCenterEngine()
{
	Destroy();
}

DataCenterEngine& DataCenterEngine::GetSerivceObj()
{
	static DataCenterEngine	obj;

	return obj;
}

int DataCenterEngine::Activate()
{
	try
	{
		m_bActivated = true;
		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCenterEngine::Activate() : activating service.............." );

		static	char						pszErrorDesc[8192] = { 0 };
		int									nErrorCode = Configuration::GetConfigObj().Load();	///< 加载配置信息

		if( 0 != nErrorCode )	{
			DataCenterEngine::GetSerivceObj().WriteWarning( "DataCenterEngine::Activate() : invalid configuration file, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		///< ........................ 开始启动本节点引擎 .............................
		if( 0 != (nErrorCode = DataIOEngine::Initialize()) )
		{
			DataCenterEngine::GetSerivceObj().WriteWarning( "DataCenterEngine::Activate() : failed 2 initialize service engine, errorcode=%d", nErrorCode );
			return nErrorCode;
		}

		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCenterEngine::Activate() : service activated.............." );

		return 0;
	}
	catch( std::exception& err )
	{
		DataCenterEngine::GetSerivceObj().WriteWarning( "DataCenterEngine::Activate() : exception : %s\n", err.what() );
	}
	catch( ... )
	{
		DataCenterEngine::GetSerivceObj().WriteWarning( "DataCenterEngine::Activate() : unknow exception" );
	}

	return -100;
}

void DataCenterEngine::Destroy()
{
	try
	{
		DataIOEngine::Release();
	}
	catch( std::exception& err )
	{
		DataCenterEngine::GetSerivceObj().WriteWarning( "DataCenterEngine::Destroy() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataCenterEngine::GetSerivceObj().WriteWarning( "DataCenterEngine::Destroy() : unknow exception" );
	}
}

bool DataCenterEngine::IsServiceAlive()
{
	if( true == SimpleThread::IsAlive() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DataCenterEngine::WriteInfo( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
//		MServicePlug::WriteInfo( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataCenterEngine::WriteWarning( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
//		MServicePlug::WriteWarning( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataCenterEngine::WriteError( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
//		MServicePlug::WriteError( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataCenterEngine::WriteDetail( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( true == m_bActivated ) {
//		MServicePlug::WriteDetail( "%s", tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}






