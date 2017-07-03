#include "DataCollector.h"
#include "../DataCenterEngine/DataCenterEngine.h"


CollectorStatus::CollectorStatus()
: m_eStatus( ET_SS_UNACTIVE )
{
}

enum E_SS_Status CollectorStatus::Get() const
{
	CriticalLock			lock( m_oCSLock );

	return m_eStatus;
}

bool CollectorStatus::Set( enum E_SS_Status eNewStatus )
{
	CriticalLock			lock( m_oCSLock );

	m_eStatus = eNewStatus;

	return true;
}


DataCollector::DataCollector()
 : m_pFuncInitialize( NULL ), m_pFuncRelease( NULL ), m_pFuncIsProxy( NULL )
 , m_pFuncRecoverQuotation( NULL ), m_pFuncGetStatus( NULL )
 , m_nMarketID( 0 ), m_bActivated( false ), m_bIsProxyPlugin( false )
{
}

DataCollector::~DataCollector()
{
	Release();
}

unsigned int DataCollector::GetMarketID()
{
	return m_nMarketID;
}

bool DataCollector::IsProxy()
{
	return m_bIsProxyPlugin;
}

int DataCollector::Initialize( I_DataHandle* pIDataCallBack, std::string sDllPath )
{
	Release();

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Initialize() : initializing data collector plugin ......" );

	std::string		sModulePath = GetModulePath(NULL) + sDllPath;
	int				nErrorCode = m_oDllPlugin.LoadDll( sModulePath );

	if( 0 != nErrorCode )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::Initialize() : failed 2 load data collector module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_pFuncInitialize = (T_Func_Initialize)m_oDllPlugin.GetDllFunction( "Initialize" );
	m_pFuncRelease = (T_Func_Release)m_oDllPlugin.GetDllFunction( "Release" );
	m_pFuncRecoverQuotation = (T_Func_RecoverQuotation)m_oDllPlugin.GetDllFunction( "RecoverQuotation" );
	m_pFuncHaltQuotation = (T_Func_HaltQuotation)m_oDllPlugin.GetDllFunction( "HaltQuotation" );
	m_pFuncGetStatus = (T_Func_GetStatus)m_oDllPlugin.GetDllFunction( "GetStatus" );
	m_pFuncGetMarketID = (T_Func_GetMarketID)m_oDllPlugin.GetDllFunction( "GetMarketID" );
	m_pFuncIsProxy = (T_Func_IsProxy)m_oDllPlugin.GetDllFunction( "IsProxy" );

	if( NULL == m_pFuncInitialize || NULL == m_pFuncRelease || NULL == m_pFuncRecoverQuotation || NULL == m_pFuncGetStatus || NULL == m_pFuncGetMarketID || NULL == m_pFuncHaltQuotation || NULL == m_pFuncIsProxy )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::Initialize() : invalid fuction pointer(NULL)" );
		return -10;
	}

	if( 0 != (nErrorCode = m_pFuncInitialize( pIDataCallBack )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::Initialize() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_nMarketID = m_pFuncGetMarketID();
	m_bIsProxyPlugin = m_pFuncIsProxy();

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Initialize() : data collector plugin is initialized ......" );

	return 0;
}

void DataCollector::Release()
{
	if( NULL != m_pFuncRelease )
	{
		DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Release() : releasing memory database plugin ......" );
		m_pFuncHaltQuotation();
		m_pFuncHaltQuotation = NULL;
		m_pFuncRelease();
		m_pFuncRelease = NULL;
		m_bActivated = false;
		DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Release() : memory database plugin is released ......" );
	}

	m_pFuncGetStatus = NULL;
	m_pFuncInitialize = NULL;
	m_pFuncRecoverQuotation = NULL;
	m_oDllPlugin.CloseDll();
}

bool DataCollector::IsAlive()
{
	return m_bActivated;
}

void DataCollector::HaltDataCollector()
{
	if( NULL != m_pFuncRelease && true == m_bActivated )
	{
		DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::HaltDataCollector() : [NOTICE] data collector is Halting ......" );

		m_pFuncHaltQuotation();
		m_bActivated = false;

		DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::HaltDataCollector() : [NOTICE] data collector Halted ......" );
	}
}

int DataCollector::RecoverDataCollector()
{
	DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::RecoverDataCollector() : recovering data collector ......" );

	if( NULL == m_pFuncRecoverQuotation )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::RecoverDataCollector() : invalid fuction pointer(NULL)" );
		return -1;
	}

	int		nErrorCode = m_pFuncRecoverQuotation();

	if( 0 != nErrorCode )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::RecoverDataCollector() : failed 2 recover quotation" );
		return nErrorCode;
	}

	m_bActivated = true;
	m_nMarketID = m_pFuncGetMarketID();
	DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::RecoverDataCollector() : data collector recovered ......" );

	return nErrorCode;
}

enum E_SS_Status DataCollector::InquireDataCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen )
{
	if( NULL == m_pFuncGetStatus )
	{
		return ET_SS_UNACTIVE;
	}

	m_oCollectorStatus.Set( (enum E_SS_Status)m_pFuncGetStatus( pszStatusDesc, nStrLen ) );

	return m_oCollectorStatus.Get();
}


///< ------------------------------------------------------------------------------------


DataCollectorPool::DataCollectorPool()
{
	std::vector<DataCollector>::reserve( 64 );
}

DataCollectorPool::~DataCollectorPool()
{
	Release();
}

void DataCollectorPool::Release()
{
	for( unsigned int n = 0; n < GetCount(); n++ )
	{
		this->operator []( n ).Release();
	}
}

int DataCollectorPool::Initialize( I_DataHandle* pIDataCallBack )
{
	Configuration&			refCnf = Configuration::GetConfigObj();
	DllPathTable&			refDcDllTable = refCnf.GetDCPathTable();
	unsigned int			nDllCount = refDcDllTable.GetCount();

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollectorPool::Initialize() : initializing ... " );

	if( nDllCount <= 0 )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollectorPool::Initialize() : data collector table is empty" );
		return -1;
	}

	if( nDllCount >= 64 )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollectorPool::Initialize() : 2 many data collector configuration, dll count = %d (>=64)", nDllCount );
		return -2;
	}

	if( GetCount() > 0 )							///< 避免重复初始化
	{
		return GetCount();
	}

	for( unsigned int n = 0; n < nDllCount; n++ )
	{
		std::string		sDcDllPath = refDcDllTable.GetPathByPos( n );

		std::vector<DataCollector>::push_back( DataCollector() );
		if( this->operator []( n ).Initialize( pIDataCallBack, sDcDllPath ) < 0 )
		{
			DataIOEngine::GetEngineObj().WriteError( "DataCollectorPool::Initialize() : failed 2 initialize data collector, %s", sDcDllPath.c_str() );
			std::vector<DataCollector>::clear();	///< 清空已经初始化成功的插件
			return -1000 - n;
		}
	}

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollectorPool::Initialize() : initialized ... (num=%d)", GetCount() );

	return GetCount();
}

int DataCollectorPool::PreserveAllConnection()
{
	int			nAffectNum = 0;

	for( unsigned int n = 0; n < GetCount(); n++ )
	{
		static char			s_pszTmp[2048] = { 0 };
		unsigned int		nBufLen = sizeof(s_pszTmp);
		DataCollector&		refDataCollector = this->operator []( n );
		enum E_SS_Status	eStatus = refDataCollector.InquireDataCollectorStatus( s_pszTmp, nBufLen );

		if( ET_SS_DISCONNECTED == eStatus )			///< 在传输断开的时，需要重新连接
		{
			DataIOEngine::GetEngineObj().WriteWarning( "DataCollectorPool::PreserveAllConnection() : [Plugin] connection disconnected, %s", s_pszTmp );

			refDataCollector.HaltDataCollector();	///< 停止插件
			int		nErrorCode = refDataCollector.RecoverDataCollector();
			if( 0 == nErrorCode )
			{
				nAffectNum++;
			}
			else
			{
				DataIOEngine::GetEngineObj().WriteWarning( "DataCollectorPool::PreserveAllConnection() : failed 2 recover data collector module, errorcode=%d", nErrorCode );
			}
		}
	}

	return nAffectNum;
}

unsigned int DataCollectorPool::GetCount()
{
	CriticalLock			lock( m_oLock );

	return std::vector<DataCollector>::size();
}









