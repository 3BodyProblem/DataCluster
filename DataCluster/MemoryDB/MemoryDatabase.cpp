#include <time.h>
#include <stdio.h>
#include "MemoryDatabase.h"
#include "InnerTableFiller.h"
#include "../DataNodeWrapper/NodeWrapper.h"
#include "../DataCenterEngine/DataCenterEngine.h"


typedef IDBFactory& __stdcall		TFunc_GetFactoryObject();
typedef void						(__stdcall *T_Func_DBUnitTest)();


DatabaseIO::DatabaseIO()
: m_pIDBFactoryPtr( NULL ), m_pIDatabase( NULL ), m_bBuilded( false )
{
}

DatabaseIO::~DatabaseIO()
{
	Release();
}

bool DatabaseIO::IsBuilded()
{
	return m_bBuilded;
}

unsigned int DatabaseIO::GetTableCount()
{
	if( false == IsBuilded() )
	{
		return 0;
	}

	if( NULL == m_pIDatabase )
	{
		return 0;
	}

	return m_pIDatabase->GetTableCount();
}

int DatabaseIO::QueryBatchRecords( unsigned int nDataID, char* pBuffer, unsigned int nBufferSize, unsigned __int64& nSerialNo )
{
	unsigned __int64		nTmpVal = 0;
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	if( NULL == pBuffer )
	{
		nSerialNo = 0;
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::QueryBatchRecords() : invalid buffer pointer(NULL)" );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		nSerialNo = 0;
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::QueryBatchRecords() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	nTmpVal = m_pIDatabase->GetUpdateSequence();
	nAffectNum = pTable->CopyToBuffer( pBuffer, nBufferSize, nSerialNo );
	if( nAffectNum < 0 )
	{
		nSerialNo = 0;
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::QueryBatchRecords() : failed 2 copy data from table, errorcode = %d", nAffectNum );
		return -3;
	}

	nSerialNo = nTmpVal;
	return nAffectNum;
}

int DatabaseIO::NewRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag, unsigned __int64& nDbSerialNo )
{
	I_Table*				pTable = NULL;
	int						nAffectNum = 0;
	CriticalLock			lock( m_oLock );

	nDbSerialNo = 0;
	m_bBuilded = bLastFlag;
	if( false == m_pIDatabase->CreateTable( nDataID, nDataLen, MAX_CODE_LENGTH ) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::NewRecord() : failed 2 create data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::NewRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -2;
	}

	if( 0 > (nAffectNum = pTable->InsertRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::NewRecord() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
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

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( 0 > (nAffectNum = pTable->DeleteRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::DeleteRecord() : failed 2 delete record from table, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -2;
	}

	return nAffectNum;
}

int DatabaseIO::UpdateRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;

	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::UpdateRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -1;
	}

	if( 0 > (nAffectNum = pTable->UpdateRecord( pData, nDataLen, nDbSerialNo )) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::UpdateRecord() : failed 2 insert into data table 4 message, message id=%d, affectnum=%d", nDataID, nAffectNum );
		return -2;
	}

	return nAffectNum;
}

int DatabaseIO::QueryRecord( unsigned int nDataID, char* pData, unsigned int nDataLen, unsigned __int64& nDbSerialNo )
{
	I_Table*		pTable = NULL;
	int				nAffectNum = 0;

	nDbSerialNo = 0;
	if( NULL == ((pTable = m_pIDatabase->QueryTable( nDataID ))) )
	{
		DataIOEngine::GetEngineObj().WriteError( "DatabaseIO::QueryRecord() : failed 2 locate data table 4 message, message id=%d", nDataID );
		return -1;
	}

	RecordBlock	oRecord = pTable->SelectRecord( pData, ::strlen(pData) );
	if( true == oRecord.IsNone() )
	{
		return 0;
	}

	::memcpy( pData, oRecord.GetPtr(), oRecord.Length() );

	return oRecord.Length();
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
	if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		return 0;
	}

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



BigTableDatabase::BigTableDatabase()
{
}

BigTableDatabase::~BigTableDatabase()
{
	BackupDatabase();		///< �ȱ������ݿ�
	Release();				///< ���ͷ�������Դ
}

int BigTableDatabase::Initialize()
{
	int			nErrCode = 0;

	if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		return 0;
	}

	DataIOEngine::GetEngineObj().WriteInfo( "BigTableDatabase::Initialize() : initializing powerfull database object ......" );

	if( (nErrCode=DatabaseIO::Initialize()) < 0 )
	{
		DataIOEngine::GetEngineObj().WriteError( "BigTableDatabase::Initialize() : failed 2 initialize, errorcode = %d", nErrCode );
		return nErrCode;
	}

	DataIOEngine::GetEngineObj().WriteInfo( "BigTableDatabase::Initialize() : powerfull database object initialized! ..." );

	return 0;
}

void BigTableDatabase::Release()
{
	DatabaseIO::Release();				///< �ͷ����ݿ�������Դ
}

int BigTableDatabase::RecoverDatabase()
{
	if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		return 0;
	}

	try
	{
		if( m_pIDatabase )
		{
			int				nDBLoadDate = 0;		///< ���������ļ�����

			DataIOEngine::GetEngineObj().WriteInfo( "BigTableDatabase::RecoverDatabase() : recovering ......" );
			m_mapTableID.clear();
			m_bBuilded = false;

			if( 0 != m_pIDatabase->DeleteTables() )
			{
				DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::RecoverDatabase() : failed 2 clean mem-database" );
				return -1;
			}

			///< ----------------- �Ӵ��ָ̻���ʷ���� --------------------------------------------------
			if( 0 > (nDBLoadDate=m_pIDatabase->LoadFromDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() )) )
			{
				DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::RecoverDatabase() : failed 2 recover mem-database from disk." );
				return 0;
			}

			m_bBuilded = true;
			return 0;
		}

		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::RecoverDatabase() : invalid database pointer(NULL)" );

		return -3;
	}
	catch( std::exception& err )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::RecoverDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::RecoverDatabase() : unknow exception" );
	}

	return -3;
}

int BigTableDatabase::BackupDatabase()
{
	if( true == EngineWrapper4DataNode::GetObj().IsUsed() )
	{
		return 0;
	}

	try
	{
		if( m_pIDatabase )
		{
			DataIOEngine::GetEngineObj().WriteInfo( "BigTableDatabase::BackupDatabase() : making backup ......" );

			if( true == m_pIDatabase->SaveToDisk( Configuration::GetConfigObj().GetRecoveryFolderPath().c_str() ) )
			{
				DataIOEngine::GetEngineObj().WriteInfo( "BigTableDatabase::BackupDatabase() : backup completed ......" );
				return 0;
			}
			else
			{
				DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::BackupDatabase() : miss backup ......" );
				return -2;
			}
		}
	}
	catch( std::exception& err )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::BackupDatabase() : exception : %s", err.what() );
	}
	catch( ... )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "BigTableDatabase::BackupDatabase() : unknow exception" );
	}

	return -1;
}










