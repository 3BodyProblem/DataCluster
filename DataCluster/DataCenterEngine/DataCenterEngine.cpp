#pragma warning(disable:4996)
#include <time.h>
#include "../targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "../DataCluster.h"
#include "DataCenterEngine.h"
#include "../DataNodeWrapper/NodeWrapper.h"


DataIOEngine::DataIOEngine()
: SimpleTask( "DataIOEngine::Thread" ), m_pQuotationCallBack( NULL )
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

BigTableDatabase& DataIOEngine::GetDatabaseObj()
{
	return m_oDB4ClientMode;
}

DataCollectorPool& DataIOEngine::GetCollectorPool()
{
	return m_oDataCollectorPool;
}

I_QuotationCallBack* DataIOEngine::GetCallBackPtr()
{
	return m_pQuotationCallBack;
}

int DataIOEngine::Initialize( I_QuotationCallBack* pIQuotation, I_DataHandle* pIDataHandle4DataNode )
{
	int							nErrorCode = 0;

	Release();
	if( NULL == (m_pQuotationCallBack = pIQuotation) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : invalid callback object pointer(Null)" );
		return -1;
	}

	if( 0 != (nErrorCode = Configuration::GetConfigObj().Load()) )	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : invalid configuration file, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = TableFillerRegister::GetRegister().Initialize()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 register message filler 2 table, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	DataIOEngine::GetEngineObj().WriteInfo( "DataIOEngine::Initialize() : DataNode Engine is initializing ......" );
	if( 0 != (nErrorCode = m_oDB4ClientMode.Initialize()) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize memory database plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_oDB4ClientMode.RecoverDatabase();
	if( 0 >= (nErrorCode = m_oDataCollectorPool.Initialize( pIDataHandle4DataNode )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize data collector plugin, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	if( 0 != (nErrorCode = m_oQuoNotify.Initialize( pIQuotation )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DataIOEngine::Initialize() : failed 2 initialize quotation notify, errorcode=%d", nErrorCode );
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
	if( NULL != m_pQuotationCallBack )
	{
		SimpleTask::StopThread();
		SimpleTask::StopAllThread();
		m_pQuotationCallBack = NULL;
		SimpleTask::Join( 5000 );
		m_oDataCollectorPool.Release();
		m_oDB4ClientMode.Release();
		m_oQuoNotify.Release();
	}
}

int DataIOEngine::Execute()
{
	DataIOEngine::GetEngineObj().WriteInfo( "DataIOEngine::Execute() : enter into thread func ..." );

	while( true == IsAlive() )
	{
		try
		{
			SimpleTask::Sleep( 1000*3 );	///< һ��ѭ��һ��

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

int DataIOEngine::OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen )
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

int DataIOEngine::OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag )
{
	unsigned __int64		nSerialNo = 0;
	int						nAffectNum = 0;
	InnerRecord*			pRecord = TableFillerRegister::GetRegister().PrepareRecordBlock( nDataID, pData, nDataLen );

	if( NULL == pRecord )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::NewRecord() : MessageID is invalid, id=%d", nDataID );
		return -1;
	}

	///< ֻ��Code��������䣬�����ֶζ�Ϊ��, �����ٴ��ڴ��ѯһ�ѣ�ȡ�������ֶε�����
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

int DataIOEngine::OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag )
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
	///< ֻ��Code��������䣬�����ֶζ�Ϊ��, �����ٴ��ڴ��ѯһ�ѣ�ȡ�������ֶε�����
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

void DataIOEngine::OnLog( unsigned char nLogLevel, const char* pszFormat, ... )
{
	va_list		valist;
	char		pszLogBuf[8000] = { 0 };

	va_start( valist, pszFormat );
	_vsnprintf( pszLogBuf, sizeof(pszLogBuf)-1, pszFormat, valist );
	va_end( valist );

	switch( nLogLevel )	///< ��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	{
	case 0:
		DataIOEngine::WriteInfo( "[Plugin] %s", pszLogBuf );
		break;
	case 1:
		DataIOEngine::WriteWarning( "[Plugin] %s", pszLogBuf );
		break;
	case 2:
		DataIOEngine::WriteError( "[Plugin] %s", pszLogBuf );
		break;
	case 3:
		DataIOEngine::WriteDetail( "[Plugin] %s", pszLogBuf );
		break;
	default:
		::printf( "[Plugin] unknow log level [%d] \n", nLogLevel );
		break;
	}
}

void DataIOEngine::WriteInfo( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( NULL != m_pQuotationCallBack ) {
		m_pQuotationCallBack->OnLog( 0, tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataIOEngine::WriteWarning( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( NULL != m_pQuotationCallBack ) {
		m_pQuotationCallBack->OnLog( 1, tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataIOEngine::WriteError( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( NULL != m_pQuotationCallBack ) {
		m_pQuotationCallBack->OnLog( 2, tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}

void DataIOEngine::WriteDetail( const char * szFormat,... )
{
	char						tempbuf[8192];
	va_list						stmarker;

	va_start(stmarker,szFormat);
	vsnprintf(tempbuf,sizeof(tempbuf),szFormat,stmarker);
	va_end(stmarker);

	if( NULL != m_pQuotationCallBack ) {
		m_pQuotationCallBack->OnLog( 3, tempbuf );
	} else {
		::printf( "%s\n", tempbuf );
	}
}






