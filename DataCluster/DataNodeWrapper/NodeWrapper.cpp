#include "NodeWrapper.h"
#include "../DataCluster.h"


ClusterCBAdaptor::ClusterCBAdaptor()
 : m_pDataHandle( NULL )
{
}

int ClusterCBAdaptor::Initialize( I_DataHandle* pIDataHandle )
{
	m_pDataHandle = pIDataHandle;
	if( NULL == pIDataHandle )
	{
		DataIOEngine::GetEngineObj().WriteError( "ClusterCBAdaptor::Initialize() : invalid arguments (I_DataHandle* ptr, NULL)\n" );
		return -1;
	}

	return 0;
}

void ClusterCBAdaptor::OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{
}

void ClusterCBAdaptor::OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus )
{
}

void ClusterCBAdaptor::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{
}


EngineWrapper4DataNode::EngineWrapper4DataNode()
 : m_pDataHandle( NULL )
{
}

EngineWrapper4DataNode& EngineWrapper4DataNode::GetObj()
{
	static EngineWrapper4DataNode	obj;

	return obj;
}

bool EngineWrapper4DataNode::IsUsed()
{
	return NULL != m_pDataHandle;
}

int EngineWrapper4DataNode::Initialize( I_DataHandle* pIDataHandle )
{
	m_pDataHandle = pIDataHandle;
	if( NULL == pIDataHandle )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataNode::Initialize() : invalid arguments (I_DataHandle* ptr, NULL)\n" );
		return -1;
	}

	if( 0 != Configuration::GetConfigObj().Load() )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataNode::Initialize() : invalid configuration file" );
		return -2;
	}

	if( m_oClusterCBAdaptor.Initialize( pIDataHandle ) )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataNode::Initialize() : failed 2 initialize ClusterAdatpor.\n" );
		return -3;
	}

	return DataIOEngine::GetEngineObj().Initialize( this );
}

void EngineWrapper4DataNode::Release()
{
	DataIOEngine::GetEngineObj().Release();
	m_pDataHandle = NULL;
}

int EngineWrapper4DataNode::RecoverQuotation()
{
	unsigned int	nSec = 0;
	int				nErrorCode = 0;

	if( 0 != (nErrorCode=StartWork( (I_QuotationCallBack*)this )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataNode::RecoverQuotation() : failed 2 subscript quotation, errorcode=%d", nErrorCode );
		return -1;
	}

	for( nSec = 0; nSec < 16 && false == DataIOEngine::GetEngineObj().GetCollectorPool().IsServiceWorking(); nSec++ )
	{
		SimpleTask::Sleep( 1000 * 1 );
	}

	if( true == DataIOEngine::GetEngineObj().GetCollectorPool().IsServiceWorking() )
	{
		return 0;
	}
	else
	{
		DataIOEngine::GetEngineObj().WriteError( "QuoCollector::RecoverQuotation() : overtime [> %d sec.], errorcode=%d", nSec, nErrorCode );
		return -2;
	}
}

void EngineWrapper4DataNode::Halt()
{
	DataIOEngine::GetEngineObj().GetCollectorPool().Release();
}

enum E_SS_Status EngineWrapper4DataNode::GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen )
{
	unsigned int			nVer = GetVersionNo();

	nStrLen = ::sprintf( pszStatusDesc, "模块名=全市场行情簇集,Version=%d.%d.%d,市场编号=%u"
		, nVer/1000000, nVer%1000000/1000, nVer%1000, GetMarketID() );

	if( true == DataIOEngine::GetEngineObj().GetCollectorPool().IsServiceWorking() )
	{
		return ET_SS_WORKING;
	}
	else
	{
		return ET_SS_INITIALIZING;
	}
}

int EngineWrapper4DataNode::OnQuery( unsigned int nDataID, const char* pData, unsigned int nDataLen )
{
	if( NULL == m_pDataHandle )
	{
		return -1;
	}

	return m_pDataHandle->OnQuery( nDataID, pData, nDataLen );
}

int EngineWrapper4DataNode::OnImage( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag )
{
	if( NULL == m_pDataHandle )
	{
		return -1;
	}

	return m_pDataHandle->OnImage( nDataID, pData, nDataLen, false );
}

int EngineWrapper4DataNode::OnData( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag, bool bPushFlag )
{
	int						nAffectNum = 0;

	if( NULL == m_pDataHandle )
	{
		return -1;
	}

	return m_pDataHandle->OnData( nDataID, pData, nDataLen, bLastFlag, bPushFlag );
}

int EngineWrapper4DataNode::OnStream( unsigned int nDataID, const char* pData, unsigned int nDataLen )
{
	if( NULL == m_pDataHandle )
	{
		return -1;
	}

	return m_pDataHandle->OnStream( nDataID, (char*)pData, nDataLen );
}

void EngineWrapper4DataNode::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
	va_list		valist;
	char		pszLogBuf[8000] = { 0 };

	va_start( valist, pszFormat );
	_vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
	va_end( valist );

	if( NULL != m_pDataHandle )
	{
		m_pDataHandle->OnLog( nLogLevel, pszLogBuf );
	}
	else
	{
		::printf( "%s\n", pszLogBuf );
	}
}






