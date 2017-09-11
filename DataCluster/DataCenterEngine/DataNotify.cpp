#include "DataNotify.h"
#include "DataCenterEngine.h"
#pragma warning(disable:4244)


PackagesLoopBuffer::PackagesLoopBuffer()
 : m_pPkgBuffer( NULL ), m_nMaxPkgBufSize( 0 )
 , m_nFirstRecord( 0 ), m_nLastRecord( 0 )
{
}

PackagesLoopBuffer::~PackagesLoopBuffer()
{
	Release();
}

int PackagesLoopBuffer::Initialize( unsigned long nMaxBufSize )
{
	Release();

	CriticalLock	guard( m_oLock );

	if( NULL == (m_pPkgBuffer = new char[nMaxBufSize]) )
	{
		DataIOEngine::GetEngineObj().WriteError( "PackagesLoopBuffer::Instance() : failed 2 initialize package buffer, size = %d", nMaxBufSize );
		return -1;
	}

	m_nMaxPkgBufSize = nMaxBufSize;

	return 0;
}

void PackagesLoopBuffer::Release()
{
	if( NULL != m_pPkgBuffer )
	{
		CriticalLock	guard( m_oLock );

		delete [] m_pPkgBuffer;
		m_pPkgBuffer = NULL;
		m_nMaxPkgBufSize = 0;
		m_nFirstRecord = 0;
		m_nLastRecord = 0;
	}
}

int PackagesLoopBuffer::PushBlock( unsigned int nMarketID, unsigned int nDataID, const char* pData, unsigned int nDataSize )
{
	CriticalLock		guard( m_oLock );

	if( NULL == m_pPkgBuffer || nDataSize == 0 || NULL == pData || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	///< �������µĿռ�
	int	nFreeSize = (m_nFirstRecord + m_nMaxPkgBufSize - m_nLastRecord - 1) % m_nMaxPkgBufSize;

	///< ����msgid+msglen+message�ռ�ռ������
	int	nMsgLen = nDataSize + sizeof(tagMsgHead);
	if( nMsgLen > nFreeSize )
	{
		return -2;	///< �ռ䲻��
	}

	///< �����µ����ݿ�
	char				pszDataBlock[sizeof(T_BIGTABLE_RECORD)+128] = { 0 };
	tagMsgHead*			pMsgHead = (tagMsgHead*)pszDataBlock;
	pMsgHead->MkID = nMarketID;
	pMsgHead->MsgID = nDataID;
	pMsgHead->MsgLen = nDataSize;
	::memcpy( pszDataBlock+sizeof(tagMsgHead), pData, nDataSize );

	int				nConsecutiveFreeSize = m_nMaxPkgBufSize - m_nLastRecord;
	if( nConsecutiveFreeSize >= nMsgLen )
	{
		::memcpy( &m_pPkgBuffer[m_nLastRecord], (char*)pszDataBlock, nMsgLen );
	}
	else
	{
		::memcpy( &m_pPkgBuffer[m_nLastRecord], pszDataBlock, nConsecutiveFreeSize );
		::memcpy( &m_pPkgBuffer[0], pszDataBlock + nConsecutiveFreeSize, (nMsgLen - nConsecutiveFreeSize) );
	}

	m_nLastRecord = (m_nLastRecord + nMsgLen) % m_nMaxPkgBufSize;

	return 0;
}

int PackagesLoopBuffer::GetBlock( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID, unsigned int& nMarketID )
{
	CriticalLock		guard( m_oLock );
	if( NULL == pBuff || nBuffSize == 0 || NULL == m_pPkgBuffer || m_nMaxPkgBufSize == 0 )	{
		assert( 0 );
		return -1;
	}

	unsigned int		nDataLen = (m_nLastRecord + m_nMaxPkgBufSize - m_nFirstRecord) % m_nMaxPkgBufSize;

	if( nDataLen == 0 )
	{
		return -2;		///< ����Ϊ��
	}

	if( sizeof(tagMsgHead) > nDataLen )
	{
		return -3;		///< ���ݲ���msghead��size
	}

	tagMsgHead			oMsgHead = { 0 };
	int					nConsecutiveSize = m_nMaxPkgBufSize - m_nFirstRecord;
	if( nConsecutiveSize >= sizeof(tagMsgHead) )
	{
		::memcpy( &oMsgHead, m_pPkgBuffer + m_nFirstRecord, sizeof(tagMsgHead) );
	}
	else
	{
		::memcpy( &oMsgHead, m_pPkgBuffer + m_nFirstRecord, nConsecutiveSize );
		::memcpy( (char*)&oMsgHead + nConsecutiveSize, m_pPkgBuffer+0, sizeof(tagMsgHead)-nConsecutiveSize );
	}

	nMarketID = oMsgHead.MkID;
	nMsgID = oMsgHead.MsgID;
	if( (oMsgHead.MsgLen + sizeof(tagMsgHead)) > nDataLen )
	{
		return -4;		///< ���ݲ��ֳ��Ȳ�ȫ
	}

	m_nFirstRecord = (m_nFirstRecord + sizeof(tagMsgHead)) % m_nMaxPkgBufSize;

	///< ��ȡ�����岿�ֵ�����
	nConsecutiveSize = m_nMaxPkgBufSize - m_nFirstRecord;
	if( nConsecutiveSize >= oMsgHead.MsgLen )
	{
		::memcpy( pBuff, m_pPkgBuffer + m_nFirstRecord, oMsgHead.MsgLen );
	}
	else
	{
		::memcpy( pBuff, m_pPkgBuffer + m_nFirstRecord, nConsecutiveSize );
		::memcpy( pBuff + nConsecutiveSize, m_pPkgBuffer+0, (oMsgHead.MsgLen - nConsecutiveSize) );
	}

	m_nFirstRecord = (m_nFirstRecord + oMsgHead.MsgLen) % m_nMaxPkgBufSize;

	return oMsgHead.MsgLen;
}

bool PackagesLoopBuffer::IsEmpty()
{
	if( m_nLastRecord == m_nFirstRecord )
	{
		return true;
	}
	else
	{
		return false;
	}
}


QuotationNotify::QuotationNotify()
 : m_pQuotationCallBack( NULL )
{
}

QuotationNotify::~QuotationNotify()
{
	Release();
}

void QuotationNotify::Release()
{
	SimpleTask::StopThread();	///< ֹͣ�߳�
	SimpleTask::Join();			///< �˳��ȴ�
	m_oDataBuffer.Release();	///< �ͷ�������Դ
}

int QuotationNotify::Initialize( I_QuotationCallBack* pIQuotation, unsigned int nNewBuffSize )
{
	Release();

	if( 0 != m_oDataBuffer.Initialize( nNewBuffSize ) )		///< ���ڴ������һ�黺��
	{
		DataIOEngine::GetEngineObj().WriteError( "QuotationNotify::Instance() : failed 2 allocate cache, size = %d", nNewBuffSize );
		return -2;
	}

	m_pQuotationCallBack = pIQuotation;

	if( NULL == m_pQuotationCallBack )
	{
		DataIOEngine::GetEngineObj().WriteError( "QuotationNotify::Instance() : callback interface is an invalid ptr (NULL)" );
		return -3;
	}

	return SimpleTask::Activate();
}

int QuotationNotify::Execute()
{
	while( true )
	{
		if( true == m_oDataBuffer.IsEmpty() )
		{
			m_oWaitEvent.Wait( 1000 * 1 );
		}

		NotifyMessage();		///< ѭ�����ͻ����е�����
	}

	return 0;
}

int QuotationNotify::PutMessage( unsigned int nMarketID, unsigned short nMsgID, const char *pData, unsigned int nLen )
{
	if( NULL == pData  )
	{
		return -12345;
	}

	int		nErrorCode = m_oDataBuffer.PushBlock( nMarketID, nMsgID, pData, nLen );	///< ��������
	if( nErrorCode < 0 )
	{
		DataIOEngine::GetEngineObj().WriteError( "QuotationNotify::PutMessage() : failed 2 push message data 2 buffer, errorcode = %d", nErrorCode );
		return nErrorCode;
	}

	if( false == m_oDataBuffer.IsEmpty() )
	{
		m_oWaitEvent.Active();
	}

	return nErrorCode;
}

void QuotationNotify::NotifyMessage()
{
	if( false == m_oDataBuffer.IsEmpty() )
	{
		unsigned int	nMarketID = 0;
		unsigned int	nMsgID = 0;
		char			pszDataBlock[sizeof(T_BIGTABLE_RECORD)+128] = { 0 };
		int				nDataSize = m_oDataBuffer.GetBlock( pszDataBlock, sizeof(pszDataBlock), nMsgID, nMarketID );

		if( nDataSize <= 0 )
		{
			DataIOEngine::GetEngineObj().WriteError( "QuotationNotify::NotifyMessage() : failed 2 fetch package from buffer, errorcode = %d", nDataSize );
			m_oWaitEvent.Wait( 1000 * 1 );
			return;
		}

		m_pQuotationCallBack->OnQuotation( (QUO_MARKET_ID)nMarketID, nMsgID, pszDataBlock, nDataSize );
	}
}












