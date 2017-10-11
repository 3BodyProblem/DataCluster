#include "NodeWrapper.h"
#include "../DataCluster.h"


InterfaceWrapper4DataNode::InterfaceWrapper4DataNode()
 : m_pDataHandle( NULL )
{
}

InterfaceWrapper4DataNode& InterfaceWrapper4DataNode::GetObj()
{
	static InterfaceWrapper4DataNode	obj;

	return obj;
}

bool InterfaceWrapper4DataNode::IsUsed()
{
	return NULL != m_pDataHandle;
}

int InterfaceWrapper4DataNode::Initialize( I_DataHandle* pIDataHandle )
{
	m_pDataHandle = pIDataHandle;
	if( NULL == pIDataHandle )
	{
		DataIOEngine::GetEngineObj().WriteError( "InterfaceWrapper4DataNode::Initialize() : invalid arguments (I_DataHandle* ptr, NULL)\n" );
		return -1;
	}

	return StartWork( (I_QuotationCallBack*)this );
}

void InterfaceWrapper4DataNode::Release()
{
	EndWork();
}

int InterfaceWrapper4DataNode::RecoverQuotation()
{
	unsigned int	nSec = 0;
	int				nErrorCode = 0;

	if( 0 != (nErrorCode=StartWork( (I_QuotationCallBack*)this )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "InterfaceWrapper4DataNode::RecoverQuotation() : failed 2 subscript quotation, errorcode=%d", nErrorCode );
		return -1;
	}

	for( nSec = 0; nSec < 60 && false == DataIOEngine::GetEngineObj().GetCollectorPool().IsServiceWorking(); nSec++ )
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

void InterfaceWrapper4DataNode::Halt()
{
	DataIOEngine::GetEngineObj().GetCollectorPool().Release();
}

enum E_SS_Status InterfaceWrapper4DataNode::GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen )
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

void InterfaceWrapper4DataNode::OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen )
{

}

void InterfaceWrapper4DataNode::OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus )
{
//	::printf( "DataClusterPlugin::OnStatusChg() : MarketID=%u, Status=%u \n", eMarketID, eMarketStatus );
/*
	if( eMarketStatus == 2 )
	{
		tagQUO_MarketInfo	tagMk;
		m_funcGetMarketInfo( eMarketID, &tagMk );
	}*/
}

void InterfaceWrapper4DataNode::OnLog( unsigned char nLogLevel, const char* pszLogBuf )
{
	unsigned int	nLevel = nLogLevel;

	::printf( "[DataCluster.dll] : LogLevel(%u), %s \n", nLevel, pszLogBuf );
}





