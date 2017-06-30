#include "UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "../DataCluster.h"
#include "../DataCollector/DataCollector.h"
#include "../DataCenterEngine/Configuration.h"


///< --------------------- ��Ԫ�����ඨ�� --------------------------------



void TestLogic::SetUpTestCase()
{
}

void TestLogic::TearDownTestCase()
{
}

void TestLogic::SetUp()
{
}

void TestLogic::TearDown()
{
}


///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------
///< ------------------------ ������������ ----------------------------------------------------

DataCollector		oDataCollector;

TEST_F( TestLogic, CheckConfiguration )
{
	///< �����ļ���Ϣ���ز���
	int						n = 0;
	bool					bInitPoint = false;
	Configuration&			refCnf = Configuration::GetConfigObj();
	int						nErrCode = refCnf.Load();

	::printf( "\n------------ ������Ϣ�б� ---------------\n" );


	::printf( "\n\n" );
}



///< ------------ ��Ԫ���Գ�ʼ���ඨ�� ------------------------------------


void UnitTestEnv::SetUp()
{
	///< ����һ�����ݿ�
//	ASSERT_NE( m_pIDatabase, (I_Database*)NULL );
}

void UnitTestEnv::TearDown()
{
	///< �ͷ�һ�����ݿ���Դ
//	for( int n = 0; n < 10; n++ )	{
//		ASSERT_EQ( GetFactoryObject().ReleaseAllDatabase(), true );
//	}
}


///< ---------------- ��Ԫ���Ե����������� -------------------------------


void ExecuteTestCase()
{
	static	bool	s_bInit = false;

	if( false == s_bInit )	{
		int			nArgc = 1;
		char*		pszArgv[32] = { "DataNode.EXE", };

		s_bInit = true;
		testing::AddGlobalTestEnvironment( new UnitTestEnv() );
		testing::InitGoogleTest( &nArgc, pszArgv );
		RUN_ALL_TESTS();
	}
}











