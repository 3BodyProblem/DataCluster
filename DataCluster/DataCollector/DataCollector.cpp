#include "DataCollector.h"
#include "../QuoteCltDef.h"
#include "../DataCenterEngine/DataCenterEngine.h"


CollectorStatus::CollectorStatus()
: m_eStatus( ET_SS_UNACTIVE ), m_eMkStatus( QUO_STATUS_NONE ), m_nMarketID( 0 )
{
}

void CollectorStatus::SetMkID( unsigned int nMkID )
{
	m_nMarketID = nMkID;
}

unsigned int CollectorStatus::GetMkID()
{
	return m_nMarketID;
}

enum E_SS_Status CollectorStatus::Get() const
{
	CriticalLock			lock( m_oCSLock );

	return m_eStatus;
}

bool CollectorStatus::Set( enum E_SS_Status eNewStatus )
{
	enum QUO_MARKET_STATUS	eNewMkStatus = QUO_STATUS_INIT;
	CriticalLock			lock( m_oCSLock );

	switch( eNewStatus )
	{
	case ET_SS_UNACTIVE:				///< 未激活:	需要对Session调用Initialize()
	case ET_SS_DISCONNECTED:			///< 断开状态
		{
			eNewMkStatus = QUO_STATUS_NONE;
			break;
		}
	case ET_SS_CONNECTED:				///< 连通状态
	case ET_SS_LOGIN:					///< 登录成功
	case ET_SS_INITIALIZING:			///< 初始化码表/快照中
	case ET_SS_INITIALIZED:				///< 初始化完成
		{
			eNewMkStatus = QUO_STATUS_INIT;
			break;
		}
	case ET_SS_WORKING:
		{
			eNewMkStatus = QUO_STATUS_NORMAL;
			break;
		}
	}

	if( m_eMkStatus != eNewMkStatus && NULL != DataIOEngine::GetEngineObj().GetCallBackPtr() && QUO_MARKET_UNKNOW != m_nMarketID )
	{
		DataIOEngine::GetEngineObj().GetCallBackPtr()->OnStatus( (enum QUO_MARKET_ID)m_nMarketID, eNewMkStatus );
		m_eMkStatus = eNewMkStatus;
	}

	m_eStatus = eNewStatus;

	return true;
}


DataCollector::DataCollector()
 : m_pFuncInitialize( NULL ), m_pFuncRelease( NULL ), m_pFuncIsProxy( NULL )
 , m_pFuncRecoverQuotation( NULL ), m_pFuncGetStatus( NULL )
 , m_bActivated( false ), m_bIsProxyPlugin( false )
{
}

DataCollector::~DataCollector()
{
	Release();
}

unsigned int DataCollector::GetMarketID()
{
	return m_oCollectorStatus.GetMkID();
}

bool DataCollector::IsProxy()
{
	return m_bIsProxyPlugin;
}

int DataCollector::Initialize( I_DataHandle* pIDataCallBack, std::string sDllPath )
{
	Release();

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Initialize() : Initializing DataCollector [%s] ......", sDllPath.c_str() );

	int				nErrorCode = m_oDllPlugin.LoadDll( sDllPath );

	if( 0 != nErrorCode )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::Initialize() : failed 2 load data collector [%s], errorcode=%d", sDllPath.c_str(), nErrorCode );
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
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::Initialize() : invalid fuction pointer(NULL) : %s", sDllPath.c_str() );
		Release();
		return -10;
	}

	if( 0 != (nErrorCode = m_pFuncInitialize( pIDataCallBack )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollector::Initialize() : failed 2 initialize data collector [%s], errorcode=%d", sDllPath.c_str(), nErrorCode );
		Release();
		return nErrorCode;
	}

	m_oCollectorStatus.SetMkID( m_pFuncGetMarketID() );
	m_bIsProxyPlugin = m_pFuncIsProxy();

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Initialize() : DataCollector [%s] is Initialized! ......", sDllPath.c_str() );

	return 0;
}

void DataCollector::Release()
{
	if( NULL != m_pFuncRelease )
	{
		DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Release() : releasing DataCollector plugin, MarketID[%u] ......", m_oCollectorStatus.GetMkID() );
		m_pFuncHaltQuotation();
		m_pFuncHaltQuotation = NULL;
		m_pFuncRelease();
		m_pFuncRelease = NULL;
		m_bActivated = false;
		DataIOEngine::GetEngineObj().WriteInfo( "DataCollector::Release() : DataCollector plugin is released, MarketID[%u] ......", m_oCollectorStatus.GetMkID() );
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
	m_oCollectorStatus.SetMkID( m_pFuncGetMarketID() );
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
	std::vector<DataCollector>::reserve( 128 );
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

	m_mapMkID2Index.clear();
}

int DataCollectorPool::Initialize( I_DataHandle* pIDataCallBack )
{
	CriticalLock			lock( m_oLock );
	Configuration&			refCnf = Configuration::GetConfigObj();
	DllPathTable&			refDcDllTable = refCnf.GetDCPathTable();
	unsigned int			nDllCount = refDcDllTable.GetCount();

	DataIOEngine::GetEngineObj().WriteInfo( "DataCollectorPool::Initialize() : initializing ... " );

	if( nDllCount >= 64 || nDllCount == 0 )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataCollectorPool::Initialize() : invalid data collector configuration, dll count = %d (>=64)", nDllCount );
		return -1;
	}

	if( GetCount() > 0 )							///< 避免重复初始化
	{
		return GetCount();
	}

	m_mapMkID2Index.clear();
	for( unsigned int n = 0; n < nDllCount; n++ )
	{
		std::string		sDcDllPath = refDcDllTable.GetPathByPos( n );

		std::vector<DataCollector>::push_back( DataCollector() );
		if( this->operator []( n ).Initialize( pIDataCallBack, sDcDllPath ) < 0 )
		{
			DataIOEngine::GetEngineObj().WriteError( "DataCollectorPool::Initialize() : failed 2 initialize data collector, %s", sDcDllPath.c_str() );
			this->operator []( n ).Release();
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
		CriticalLock		lock( m_oLock );
		static char			s_pszTmp[2048] = { 0 };
		unsigned int		nBufLen = sizeof(s_pszTmp);
		DataCollector&		refDataCollector = this->operator []( n );
		enum E_SS_Status	eStatus = refDataCollector.InquireDataCollectorStatus( s_pszTmp, nBufLen );

		if( ET_SS_DISCONNECTED == eStatus )			///< 在传输断开的时，需要重新连接
		{
			DataIOEngine::GetEngineObj().WriteWarning( "DataCollectorPool::PreserveAllConnection() : initializing DataCollector plugin ..." );

			refDataCollector.HaltDataCollector();	///< 停止插件
			int		nErrorCode = refDataCollector.RecoverDataCollector();
			if( 0 == nErrorCode )
			{
				DataIOEngine::GetEngineObj().WriteInfo( "DataCollectorPool::PreserveAllConnection() : DataCollector Recovered Successfully! MarketID[%u] --> Index[%d] !!! ", refDataCollector.GetMarketID(), n );
				nAffectNum++;
				m_mapMkID2Index[refDataCollector.GetMarketID()] = n;
			}
			else
			{
				DataIOEngine::GetEngineObj().WriteWarning( "DataCollectorPool::PreserveAllConnection() : failed 2 initialize DataCollector, errorcode=%d", nErrorCode );
			}

			if( (n+1) == GetCount() )
			{
				DataIOEngine::GetEngineObj().WriteInfo( "DataCollectorPool::PreserveAllConnection() : All Connections had been established! Num=[%u] .......!!! ", GetCount() );
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

int DataCollectorPool::MkIDCast( unsigned int nOldMkID )
{
	switch( nOldMkID )
	{
	case XDF_SH:
		return QUO_MARKET_SSE;
	case XDF_SZ:
		return QUO_MARKET_SZSE;
	case XDF_CF:
		return QUO_MARKET_CFFEX;
	case XDF_CNF:
		return QUO_MARKET_DCE;//QUO_MARKET_CZCE//QUO_MARKET_SHFE
	case XDF_SHOPT:
		return QUO_MARKET_SSEOPT;
	case XDF_ZJOPT:
		return QUO_MARKET_CFFEXOPT;
	case XDF_SZOPT:
		return QUO_MARKET_SZSEOPT;
	case XDF_CNFOPT:
		return QUO_MARKET_DCEOPT;//QUO_MARKET_CZCEOPT//QUO_MARKET_SHFEOPT
	default:
		return -1;
	}
}

int DataCollectorPool::Cast2OldMkID( unsigned int nNewMkID )
{
	switch( nNewMkID )
	{
	case QUO_MARKET_SSE:
		return XDF_SH;
	case QUO_MARKET_SZSE:
		return XDF_SZ;
	case QUO_MARKET_CFFEX:
		return XDF_CF;
	case QUO_MARKET_DCE:
	case QUO_MARKET_CZCE:
	case QUO_MARKET_SHFE:
		return XDF_CNF;
	case QUO_MARKET_SSEOPT:
		return XDF_SHOPT;
	case QUO_MARKET_CFFEXOPT:
		return XDF_ZJOPT;
	case QUO_MARKET_SZSEOPT:
		return XDF_SZOPT;
	case QUO_MARKET_DCEOPT:
	case QUO_MARKET_CZCEOPT:
	case QUO_MARKET_SHFEOPT:
		return XDF_CNFOPT;
	default:
		return -1;
	}

	return -100;
}

DataCollector* DataCollectorPool::GetCollectorByMkID( unsigned int nMkID )
{
	int						nInnerMkID = DataCollectorPool::MkIDCast( nMkID );
	CriticalLock			lock( m_oLock );

	if( nInnerMkID < 0 )
	{
		return NULL;
	}

	std::map<int,int>::iterator		it = m_mapMkID2Index.find( nInnerMkID );

	if( it != m_mapMkID2Index.end() )
	{
		DataCollector&				refCollector = this->operator []( it->second );

		return &refCollector;
	}

	return NULL;
}






