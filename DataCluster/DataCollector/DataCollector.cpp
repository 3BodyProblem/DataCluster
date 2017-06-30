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

unsigned int DataCollector::GetMarketID()
{
	return m_nMarketID;
}

bool DataCollector::IsProxy()
{
	return m_bIsProxyPlugin;
}

int DataCollector::Initialize( I_DataHandle* pIDataCallBack )
{
	Release();

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::Initialize() : initializing data collector plugin ......" );

	std::string		sModulePath = GetModulePath(NULL) + Configuration::GetConfigObj().GetDataCollectorPluginPath();
	int				nErrorCode = m_oDllPlugin.LoadDll( sModulePath );

	if( 0 != nErrorCode )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataCollector::Initialize() : failed 2 load data collector module, errorcode=%d", nErrorCode );
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
		DataCenterEngine::GetSerivceObj().WriteError( "DataCollector::Initialize() : invalid fuction pointer(NULL)" );
		return -10;
	}

	if( 0 != (nErrorCode = m_pFuncInitialize( pIDataCallBack )) )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataCollector::Initialize() : failed 2 initialize data collector module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_nMarketID = m_pFuncGetMarketID();
	m_bIsProxyPlugin = m_pFuncIsProxy();

	DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::Initialize() : data collector plugin is initialized ......" );

	return 0;
}

void DataCollector::Release()
{
	if( NULL != m_pFuncRelease )
	{
		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::Release() : releasing memory database plugin ......" );
		m_pFuncHaltQuotation();
		m_pFuncHaltQuotation = NULL;
		m_pFuncRelease();
		m_pFuncRelease = NULL;
		m_bActivated = false;
		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::Release() : memory database plugin is released ......" );
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
		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::HaltDataCollector() : [NOTICE] data collector is Halting ......" );

		m_pFuncHaltQuotation();
		m_bActivated = false;

		DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::HaltDataCollector() : [NOTICE] data collector Halted ......" );
	}
}

int DataCollector::RecoverDataCollector()
{
	DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::RecoverDataCollector() : recovering data collector ......" );

	if( NULL == m_pFuncRecoverQuotation )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataCollector::RecoverDataCollector() : invalid fuction pointer(NULL)" );
		return -1;
	}

	int		nErrorCode = m_pFuncRecoverQuotation();

	if( 0 != nErrorCode )
	{
		DataCenterEngine::GetSerivceObj().WriteError( "DataCollector::RecoverDataCollector() : failed 2 recover quotation" );
		return nErrorCode;
	}

	m_bActivated = true;
	m_nMarketID = m_pFuncGetMarketID();
	DataCenterEngine::GetSerivceObj().WriteInfo( "DataCollector::RecoverDataCollector() : data collector recovered ......" );

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














