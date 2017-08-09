#include "InnerTableFiller.h"
#include "../DataCenterEngine/DataCenterEngine.h"


const unsigned int TableFillerRegister::s_nRegisterTableSize = 1024;
std::vector<InnerRecord*> TableFillerRegister::s_vctRegisterTable;


TableFillerRegister::TableFillerRegister()
{
	s_vctRegisterTable.resize( s_nRegisterTableSize );		///< Ԥ��1024��messageidӳ������ַ
}

TableFillerRegister&	TableFillerRegister::GetRegister()
{
	static TableFillerRegister		obj;

	return obj;
}

InnerRecord* TableFillerRegister::PrepareNewTableBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen )
{
	unsigned int			nPos = nMessageID % s_nRegisterTableSize;
	InnerRecord*			pInnerRecord = s_vctRegisterTable[nPos];

	if( NULL == pMsgPtr || nMsgLen < 20 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : invalid parameters" );

		return NULL;
	}

	if( NULL != pInnerRecord )
	{
		pInnerRecord->m_nMsgLen = nMsgLen;
		pInnerRecord->m_pMsgPtr = (char*)pMsgPtr;
		::memset( pInnerRecord->GetInnerRecordPtr(), 0, pInnerRecord->GetInnerRecordLength() );	///< ����һ�ѣ��ڳ��ռ�
		::memcpy( pInnerRecord->GetInnerRecordPtr(), pMsgPtr, 20 );								///< �ٰ�Code��copy��ȥ

		return pInnerRecord;
	}

	DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : invalid Message ID" );

	return NULL;
}







