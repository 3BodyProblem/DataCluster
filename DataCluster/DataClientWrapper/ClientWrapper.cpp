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

	DataIOEngine::GetEngineObj().WriteInfo( "EngineWrapper4DataClient::Initialize() : DataNode Engine is initializing ......" );
	if( 0 != (nErrorCode = m_oDB4ClientMode.Initialize()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataClient::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_oDB4ClientMode.RecoverDatabase();
	if( 0 != (nErrorCode = m_oQuoNotify.Initialize( pIQuotation )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "EngineWrapper4DataClient::Initialize() : failed 2 initialize quotation notify, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	return DataIOEngine::GetEngineObj().Initialize( this );
}

void EngineWrapper4DataClient::Release()
{
	DataIOEngine::GetEngineObj().Release();
	SimpleTask::StopAllThread();
	m_pQuotationCallBack = NULL;
	m_oDB4ClientMode.Release();
	m_oQuoNotify.Release();
}

int EngineWrapper4DataClient::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	unsigned __int64		nSerialNo = 0;
	static	const char		s_pszZeroBuff[128] = { 0 };

	if( 0 == strncmp( pData, s_pszZeroBuff, min(nDataLen, sizeof(s_pszZeroBuff)) ) )
	{
		return m_oDB4ClientMode.QueryBatchRecords( nDataID, pData, nDataLen, nSerialNo );
	}
	else
	{
		return m_oDB4ClientMode.QueryRecord( nDataID, pData, nDataLen, nSerialNo );
	}
}

int EngineWrapper4DataClient::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	unsigned __int64		nSerialNo = 0;
	int						nAffectNum = 0;
	InnerRecord*			pRecord = TableFillerRegister::GetRegister().PrepareRecordBlock( nDataID, pData, nDataLen );

	if( NULL == pRecord )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::NewRecord() : MessageID is invalid, id=%d", nDataID );
		return -1;
	}

	///< 只有Code有内容填充，其他字段都为空, 所以再从内存查询一把，取得其他字段的内容
	nAffectNum = m_oDB4ClientMode.QueryRecord( pRecord->GetBigTableID(), pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );

	if( nAffectNum <= 0 )
	{
		pRecord->FillMessage2BigTableRecord( pData, true );
		nAffectNum = m_oDB4ClientMode.NewRecord( pRecord->GetBigTableID(), pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), bLastFlag, nSerialNo );
	}
	else
	{
		pRecord->FillMessage2BigTableRecord( pData );
		nAffectNum = m_oDB4ClientMode.UpdateRecord( pRecord->GetBigTableID(), pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );
	}

	return nAffectNum;
}

int EngineWrapper4DataClient::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
{
	unsigned __int64		nSerialNo = 0;
	int						nAffectNum = 0;
	InnerRecord*			pRecord = TableFillerRegister::GetRegister().PrepareRecordBlock( nDataID, pData, nDataLen );

	if( NULL == pRecord )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::NewRecord() : MessageID is invalid, id=%d", nDataID );
		return -1;
	}

	unsigned int			nBigTableID = pRecord->GetBigTableID();
	///< 只有Code有内容填充，其他字段都为空, 所以再从内存查询一把，取得其他字段的内容
	nAffectNum = m_oDB4ClientMode.QueryRecord( nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );
	if( nAffectNum < 0 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::UpdateRecord() : error occur in UpdateRecord(), MessageID=%d", nDataID );
		return -2;
	}
	else if( nAffectNum == 0 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::UpdateRecord() : MessageID isn\'t exist, id=%d", nDataID );
		return -3;
	}
	else
	{
		pRecord->FillMessage2BigTableRecord( pData );
		nAffectNum = m_oDB4ClientMode.UpdateRecord( nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth(), nSerialNo );
	}

	m_oQuoNotify.PutMessage( nBigTableID/100, nBigTableID, pRecord->GetBigTableRecordPtr(), pRecord->GetBigTableWidth() );

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

I_QuotationCallBack* EngineWrapper4DataClient::GetCallBackPtr()
{
	return m_pQuotationCallBack;
}

void EngineWrapper4DataClient::OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus )
{
	if( NULL != m_pQuotationCallBack )
	{
		m_pQuotationCallBack->OnStatus( eMarketID, eMarketStatus );
	}
}



