#pragma warning(disable:4996)
#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCenterEngine.h"


DataIOEngine::DataIOEngine()
 : SimpleTask( "DataIOEngine::Thread" )
 , m_nPushSerialNo( 0 )
{
}

int DataIOEngine::Initialize( const std::string& sDataCollectorPluginPath, const std::string& sMemPluginPath )
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
	if( 0 != (nErrorCode = m_oDataCollector.Initialize( this )) )
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
	m_nPushSerialNo = 0;
	m_oDataCollector.Release();
	m_oDatabaseIO.Release();
	SimpleTask::StopThread();
}

bool DataIOEngine::PrepareQuotation()
{
	int			nErrorCode = 0;

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::PrepareQuotation() : reloading quotation........" );

	m_oDataCollector.HaltDataCollector();												///< 1) 先事先停止数据采集模块

	if( 0 != (nErrorCode=m_oDataCollector.RecoverDataCollector()) )						///< 3) 重新初始化行情采集模块
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
			{
				SimpleTask::Sleep( 1000 );		///< 重新初始化间隔，默认为3秒
				DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : [NOTICE] Enter Service Initializing Time ......" );

				if( false == PrepareQuotation() )		///< 重新加载行情数据
				{
					continue;
				}

				DataCenterEngine::GetSerivceObj().WriteInfo( "DataIOEngine::Execute() : ................. [NOTICE] Service is Available ....................." );
				continue;
			}

			OnIdle();									///< 空闲处理函数
			SimpleTask::Sleep( 1000 );					///< 一秒循环一次
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
	return m_oDatabaseIO.BuildMessageTable( nDataID, pData, nDataLen, bLastFlag, m_nPushSerialNo );
}

int DataIOEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	int					nErrorCode = m_oDatabaseIO.UpdateQuotation( nDataID, pData, nDataLen, m_nPushSerialNo );

	if( 0 >= nErrorCode )
	{
		return nErrorCode;
	}

//	m_oLinkSessions.PushQuotation( nDataID, 0, pData, nDataLen, bPushFlag, m_nPushSerialNo );

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
		if( 0 != (nErrorCode = DataIOEngine::Initialize( Configuration::GetConfigObj().GetDataCollectorPluginPath()
													, Configuration::GetConfigObj().GetMemPluginPath() )) )
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

int DataCenterEngine::OnIdle()
{
	///< 非交易时段，停止源驱动的数据采集模块的工作
	if( false == m_oDataCollector.IsProxy() && true == m_oDataCollector.IsAlive() )
	{
		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCenterEngine::OnIdle() : halting data collector ......" );
		m_oDataCollector.HaltDataCollector();
	}

	return 0;
}

bool DataCenterEngine::OnInquireStatus( char* pszStatusDesc, unsigned int& nStrLen )
{
	bool				bDataBuilded = m_oDatabaseIO.IsBuilded();
	enum E_SS_Status	eStatus = m_oDataCollector.InquireDataCollectorStatus( pszStatusDesc, nStrLen );

	///< 交易时段的工作状态
	if( ET_SS_WORKING == eStatus && true == bDataBuilded )
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






