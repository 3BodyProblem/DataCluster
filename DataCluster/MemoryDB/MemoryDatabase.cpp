#include <time.h>
#include <stdio.h>
#include "MemoryDatabase.h"
#include "../DataCenterEngine/DataCenterEngine.h"


typedef IDBFactory& __stdcall		TFunc_GetFactoryObject();
typedef void						(__stdcall *T_Func_DBUnitTest)();


DatabaseIO::DatabaseIO()
: m_pIDBFactoryPtr( NULL ), m_pIDatabase( NULL ), m_bBuilded( false )
{
	m_nUpdateTimeT = ::time( NULL );
}

DatabaseIO::~DatabaseIO()
{
	Release();
}

bool DatabaseIO::IsBuilded()
{
	return m_bBuilded;
}

time_t DatabaseIO::GetLastUpdateTime()
{
	return m_nUpdateTimeT;
}

unsigned int DatabaseIO::GetTableCount()
{
	if( false == IsBuilded() )
	{
		return 0;
	}

	return m_mapTableID.size();
}

unsigned int DatabaseIO::GetTablesID( unsigned int* pIDList, unsigned int nMaxListSize, unsigned int* pWidthList, unsigned int nMaxWidthlistLen )
{
	unsigned int			nIndex = 0;
	unsigned int*			pIDListPtr = pIDList;
	CriticalLock			lock( m_oLock );

	for( TMAP_DATAID2WIDTH::iterator it = m_mapTableID.begin(); it != m_mapTableID.end() && nIndex < nMaxListSize; it++ )
	{
		*(pIDListPtr+nIndex) = it->first;

		if( NULL != pWidthList && 0 != nMaxWidthlistLen )
		{
			*(pWidthList+nIndex) = it->second;
		}

		nIndex++;
	}

	return nIndex;
}

int DatabaseIO::FetchRecordsByID( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo )
{
	unsigned __int64		nTmpVal = 0;
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	if( NULL == pBuffer )
	{
		nSerialNo = 0;
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::FetchRecordsByID() : invalid buffer pointer(NULL)" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		nSerialNo = 0;
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::FetchRecordsByID() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	nTmpVal = m_pIDatabase->GetUpdateSequence();
	nAffectNum = pTable->CopyToBuffer( pBuffer, nBufferSize, nSerialNo );
	if( nAffectNum < 0 )
	{
		nSerialNo = 0;
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::FetchRecordsByID() : failed 2 copy data from table, errorcode = %d", nAffectNum );
		return -3;
	}

	nSerialNo = nTmpVal;
	return nAffectNum;
}

int DatabaseIO::BuildMessageTable( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo )
{
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	nDbSerialNo = 0;
	m_bBuilded = bLastFlag;
	if( false == m_pIDatabase->CreateTable( nDataID, nDataLen, MAX_CODE_LENGTH ) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 create data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->InsertRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::BuildMessageTable() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	m_mapTableID.insert( std::make_pair(nDataID, nDataLen) );		///< ���ݱ�ID���ϣ����

	return 0;
}

int DatabaseIO::DeleteRecord( unsigned int nDataID, char* pData, unsigned int nDataLen )
{
	I_Table*			pTable = NULL;
	int					nAffectNum = 0;
	unsigned __int64	nDbSerialNo = 0;

	if( false == m_bBuilded )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 delete record before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->DeleteRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 delete record from table, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	return nAffectNum;
}

int DatabaseIO::UpdateQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->UpdateRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::UpdateQuotation() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -3;
	}

	m_nUpdateTimeT = ::time( NULL );

	return nAffectNum;
}

int DatabaseIO::QueryQuotation( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( false == m_bBuilded )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::QueryQuotation() : failed 2 update quotation before initialization, message id=%d" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::QueryQuotation() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	RecordBlock	oRecord = pTable->SelectRecord( pData, ::strlen(pData) );
	if( true == oRecord.IsNone() )
	{
		return 0;
	}

	::memcpy( pData, oRecord.GetPtr(), oRecord.Length() );

	return oRecord.Length();
}

int DatabaseIO::RecoverDatabase()
{
	try
	{
		if( m_pIDatabase )
		{
			int				nDBLoadDate = 0;		///< ���������ļ�����

			DataIOEngine::GetEngineObj().WriteInfo( "DatabaseIO::RecoverDatabase() : recovering ......" );
			m_mapTableID.clear();
			m_nUpdateTimeT = ::time( NULL );

			m_bBuilded = false;
			if( 0 != m_pIDatabase->DeleteTables() )
			{
				DataIOEngine::GetEngineObj().WriteWarning( "DatabaseIO::RecoverDatabase() : failed 2 clean mem-database" );
				return -1;
			}

//			m_bBuilded = true;
			return 0;
		}

		DataIOEngine::GetEngineObj().WriteWarning( "DatabaseIO::RecoverDatabase() : invalid database pointer(NULL)" );

		return -3;
	}
	catch( std::exception& err )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "DatabaseIO::RecoverDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "DatabaseIO::RecoverDatabase() : unknow exception" );
	}

	return -3;
}

void DatabaseIO::UnitTest()
{
	T_Func_DBUnitTest		funcUnitTest = NULL;
	int						nErrorCode = m_oDllPlugin.LoadDll( Configuration::GetConfigObj().GetMemPluginPath() );

	if( 0 != nErrorCode )
	{
		::printf( "DatabaseIO::Initialize() : failed 2 load memoryplugin module, errorcode=%d", nErrorCode );
		return;
	}

	funcUnitTest = (T_Func_DBUnitTest)m_oDllPlugin.GetDllFunction( "ExecuteUnitTest" );
	funcUnitTest();

	m_oDllPlugin.CloseDll();
}

int DatabaseIO::Initialize()
{
	Release();

	DataIOEngine::GetEngineObj().WriteInfo( "DatabaseIO::Initialize() : initializing memory database plugin ......" );

	TFunc_GetFactoryObject*	m_funcFactory = NULL;
	int						nErrorCode = m_oDllPlugin.LoadDll( Configuration::GetConfigObj().GetMemPluginPath() );

	if( 0 != nErrorCode )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::Initialize() : failed 2 load memoryplugin module, errorcode=%d", nErrorCode );
		return nErrorCode;
	}

	m_funcFactory = (TFunc_GetFactoryObject*)m_oDllPlugin.GetDllFunction( "GetFactoryObject" );
	m_pIDBFactoryPtr = &(m_funcFactory());
	m_pIDatabase = m_pIDBFactoryPtr->GrapDatabaseInterface();
	if( NULL == m_pIDatabase )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::Initialize() : invalid database interface pointer(NULL)" );
		Release();
		return -100;
	}

	DataIOEngine::GetEngineObj().WriteInfo( "DatabaseIO::Initialize() : memory database plugin is initialized ......" );

	return 0;
}

void DatabaseIO::Release()
{
	if( NULL != m_pIDatabase || NULL != m_pIDBFactoryPtr )
	{
		DataIOEngine::GetEngineObj().WriteInfo( "DatabaseIO::Release() : releasing memory database plugin ......" );

		m_mapTableID.clear();				///< ������ݱ�ID����
		m_pIDatabase->DeleteTables();		///< �����ڴ����е����ݱ�
		m_pIDatabase = NULL;				///< �����ڴ������ݿ�ָ��

		if( m_pIDBFactoryPtr )				///< �������ڲ���е��������ݿ�
		{
			m_pIDBFactoryPtr->ReleaseAllDatabase();
			m_pIDBFactoryPtr = NULL;		///< �����ڴ����Ĺ�������ָ��
		}

		m_oDllPlugin.CloseDll();			///< ж���ڴ������DLL

		DataIOEngine::GetEngineObj().WriteInfo( "DatabaseIO::Release() : memory database plugin is released ......" );
	}
}












