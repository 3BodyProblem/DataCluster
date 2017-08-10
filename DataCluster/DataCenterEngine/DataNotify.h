#ifndef __DATA_NOTIFY_H__
#define __DATA_NOTIFY_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


#pragma pack(1)
typedef struct
{
	unsigned short		MsgID;		///< ��ϢID
	unsigned short		MsgLen;		///< ��Ϣ����
} tagMsgHead;
#pragma pack()


/**
 * @class							PackagesLoopBuffer
 * @brief							���ݰ����л���
 * @detail							MessageID1 + data block1 + data block2 + data block3 + ...
 * @author							barry
 */
class PackagesLoopBuffer
{
public:
	PackagesLoopBuffer();
	~PackagesLoopBuffer();

public:
	/**
	 * @brief						��ʼ���������
	 * @param[in]					nMaxBufSize				������Ļ����С
	 * @return						==0						�ɹ�
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						�ͷŻ���ռ�
	 */
	void							Release();
 
public:
	/**
	 * @brief						�洢����
	 * @param[in]					nDataID					����ID
	 * @param[in]					pData					����ָ��
	 * @param[in]					nDataSize				���ݳ���
	 * @return						==0						�ɹ�
	 * @note						��nDataID������ǰһ������nDataIDʱ����������һ��Package��װ
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize );

	/**
	 * @brief						��ȡһ�����ݰ�
	 * @param[out]					pBuff					������ݻ����ַ
	 * @param[in]					nBuffSize				���ݻ��泤��
	 * @param[out]					nMsgID					������ϢID
	 * @return						>0						���ݳ���
									==0						������
									<0						����
	 */
	int								GetBlock( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID );

	/**
	 * @brief						�Ƿ�Ϊ��
	 * @return						true					Ϊ��
	 */
	bool							IsEmpty();

protected:
	CriticalObject					m_oLock;				///< ��
	char*							m_pPkgBuffer;			///< ���ݰ������ַ
	unsigned int					m_nMaxPkgBufSize;		///< ���ݰ������С
	unsigned long					m_nFirstRecord;			///< ������ʼλ��
	unsigned long					m_nLastRecord;			///< ���ݽ���λ��
};


/**
 * @class						QuotationNotify
 * @brief						������ʵʱ���ͻ���
 * @author						barry
 */
class QuotationNotify : public SimpleTask
{
public:
	/**
	 * @brief					���캯��
	 */
	QuotationNotify();
	~QuotationNotify();

	/**
	 * @brief					��ʼ�����������ͻ���
	 * @param[in]				pIQuotation				��������ص�֪ͨ�Ľӿ�
	 * @param[in]				nNewBuffSize			Ҫ����Ļ����С
	 * @return					!= 0					ʧ��
	 */
	int							Initialize( I_QuotationCallBack* pIQuotation, unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					�ͷŸ���Դ
	 */
	void						Release();

protected:
	/**
	 * @brief					������(��ѭ��)
	 * @return					==0						�ɹ�
								!=0						ʧ��
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					����������ѹ������л���
	 * @param[in]				nMsgID					Message ID
	 * @param[in]				pData					��Ϣ���ݵ�ַ
	 * @param[in]				nLen					��Ϣ����
	 * @return					> 0						�ɹ����������ε����ۻ������л��ĳ���
								<= 0					ʧ��
	 */
	int							PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen );

	/**
	 * @brief					�ӻ���ȡ����Ϣ���ݻص����ⲿ������
	 */
	void						NotifyMessage();

protected:
	I_QuotationCallBack*		m_pQuotationCallBack;	///< ����ص�
	WaitEvent					m_oWaitEvent;			///< �����ȴ�
	PackagesLoopBuffer			m_oDataBuffer;			///< �������ݻ������
};





#endif





