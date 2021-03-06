#include "ClientWrapper.h"
#include "../DataCluster.h"


EngineWrapper4DataClient::EngineWrapper4DataClient()
 : m_pQuotationCallBack( NULL )
{
}

EngineWrapper4DataClient& EngineWrapper4DataClient::GetObj()
{
	static EngineWrapper4DataClient	obj;

	return obj;
}

bool EngineWrapper4DataClient::IsUsed()
{
	return NULL != m_pQuotationCallBack;
}

int EngineWrapper4DataClient::Initialize( I_QuotationCallBack* pIQuotation )
{
	int		nErrorCode = 0;

	m_pQuotationCallBack = pIQuotation;
	if( NULL == m_pQuotationCallBack )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataClient::Initialize() : invalid arguments (I_DataHandle* ptr, NULL)\n" );
		return -1;
	}

	if( 0 != (nErrorCode = Configuration::GetConfigObj().Load()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataClient::Initialize() : invalid configuration file, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	DataIOEngine::GetEngineObj().WriteInfo( "EngineWrapper4DataClient::Initialize() : DataNode Engine is initializing ......" );
	if( 0 != (nErrorCode = m_oDB4ClientMode.Initialize()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataClient::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_oDB4ClientMode.RecoverDatabase();		///< 从本地恢复历史数据(DataCluster.DLL内部的数据库，独立于DataNode.exe中的数据库)

	if( false == Global_Client.InQuoteClientApiMode() )
	{
		if( 0 != (nErrorCode = m_oQuoNotify.Initialize( pIQuotation )) )
		{
			DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataClient::Initialize() : failed 2 initialize quotation notify, errorcode=%d", nErrorCode );
			return nErrorCode;
		}
	}

	return DataIOEngine::GetEngineObj().Initialize( this );
}

void EngineWrapper4DataClient::Release()
{
	DataIOEngine::GetEngineObj().WriteInfo( "EngineWrapper4DataClient::Release() : Releasing ......" );

	DataIOEngine::GetEngineObj().Release();
	m_pQuotationCallBack = NULL;
	m_oDB4ClientMode.Release();
	m_oQuoNotify.Release();

	DataIOEngine::GetEngineObj().WriteInfo( "EngineWrapper4DataClient::Release() : Released ......" );
}

int EngineWrapper4DataClient::OnQuery( unsigned int nDataID, const char* pData, unsigned int nDataLen )
{
	unsigned __int64		nSerialNo = 0;
	static	const char		s_pszZeroBuff[128] = { 0 };

	if( 0 == strncmp( pData, s_pszZeroBuff, min(nDataLen, sizeof(s_pszZeroBuff)) ) )
	{
		return m_oDB4ClientMode.QueryBatchRecords( nDataID, (char*)pData, nDataLen, nSerialNo );
	}
	else
	{
		return m_oDB4ClientMode.QueryRecord( nDataID, (char*)pData, nDataLen, nSerialNo );
	}
}

int EngineWrapper4DataClient::OnImage( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag )
{
	unsigned __int64		nSerialNo = 0;
	int						nAffectNum = 0;
	InnerRecord*			pRecord = TableFillerRegister::GetRegister().PrepareRecordBlock( nDataID, pData, nDataLen );

	if( NULL == pRecord )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "EngineWrapper4DataClient::NewRecord() : MessageID is invalid, id=%d", nDataID );
		return -1;
	}

	///< 只有Code有内容填充，其他字段都为空, 所以再从内存查询一把，取得其他字段的内容
	nAffectNum = m_oDB4ClientMode.QueryRecord( pRecord->GetBigTableID(), pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );

	if( nAffectNum <= 0 )
	{
		pRecord->FillMessage2BigTableRecord( (char*)pData, true );
		nAffectNum = m_oDB4ClientMode.NewRecord( pRecord->GetBigTableID(), pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), bLastFlag, nSerialNo );
	}
	else
	{
		pRecord->FillMessage2BigTableRecord( (char*)pData );
		nAffectNum = m_oDB4ClientMode.UpdateRecord( pRecord->GetBigTableID(), pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );
	}

	return nAffectNum;
}

int EngineWrapper4DataClient::OnStream( unsigned int nDataID, const char* pData, unsigned int nDataLen )
{
	return 0;
}

int EngineWrapper4DataClient::OnData( unsigned int nDataID, const char* pData, unsigned int nDataLen, bool bLastFlag, bool bPushFlag )
{
	unsigned __int64		nSerialNo = 0;
	int						nAffectNum = 0;
	InnerRecord*			pRecord = TableFillerRegister::GetRegister().PrepareRecordBlock( nDataID, pData, nDataLen );

	if( NULL == pRecord )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "EngineWrapper4DataClient::NewRecord() : MessageID is invalid, id=%d", nDataID );
		return -1;
	}

	unsigned int			nBigTableID = pRecord->GetBigTableID();
	///< 只有Code有内容填充，其他字段都为空, 所以再从内存查询一把，取得其他字段的内容
	nAffectNum = m_oDB4ClientMode.QueryRecord( nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );
	if( nAffectNum < 0 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "EngineWrapper4DataClient::UpdateRecord() : error occur in UpdateRecord(), MessageID=%d", nDataID );
		return -2;
	}
	else if( nAffectNum == 0 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "EngineWrapper4DataClient::UpdateRecord() : MessageID isn\'t exist, id=%d", nDataID );
		return -3;
	}
	else
	{
		pRecord->FillMessage2BigTableRecord( (char*)pData );
		nAffectNum = m_oDB4ClientMode.UpdateRecord( nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );
	}

	if( false == Global_Client.InQuoteClientApiMode() )
	{
		m_oQuoNotify.PutMessage( nBigTableID/100, nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth() );
	}
	else
	{
		m_pQuotationCallBack->OnQuotation( (enum QUO_MARKET_ID)(nBigTableID/100), nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth() );
	}

	return nAffectNum;
}

void EngineWrapper4DataClient::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
	va_list		valist;
	char		pszLogBuf[8000] = { 0 };

	va_start( valist, pszFormat );
	_vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
	va_end( valist );

	if( NULL != m_pQuotationCallBack )
	{
		m_pQuotationCallBack->OnLog( nLogLevel, pszLogBuf );
	}
	else
	{
		::printf( "%s\n", pszLogBuf );
	}
}

BigTableDatabase& EngineWrapper4DataClient::GetDatabaseObj()
{
	return m_oDB4ClientMode;
}

void EngineWrapper4DataClient::OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus )
{
	if( NULL != m_pQuotationCallBack )
	{
		m_pQuotationCallBack->OnStatus( eMarketID, eMarketStatus );
	}
}



