#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <map>
#include <string>
#include "DataNotify.h"
#include "Configuration.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../DataCollector/Interface.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../MemoryDB/InnerTableFiller.h"
#include "../DataCollector/DataCollector.h"
#include "../../../DataCollector4CTPDL/DataCollector4CTPDL/CTP_DL_QuoProtocal.h"
#include "../../../DataCollector4CTPSH/DataCollector4CTPSH/CTP_SH_QuoProtocal.h"
#include "../../../DataCollector4CTPZZ/DataCollector4CTPZZ/CTP_ZZ_QuoProtocal.h"


/**
 * @class					DataIOEngine
 * @brief					�������ݸ��¹�������(��Ҫ��װ���ݳ�ʼ���͸���/���͵�ҵ��)
 * @detail					����/Э������ģ��(���ݲɼ����+�����ڴ���+����ѹ�����)
 * @note					��Ҫ�ṩ��������������صĻ�������: �ɼ�������������µ��ڴ� + �������ݳ�ʼ�������߼� + �������ݶ��¼��������ܷ�װ
							&
							���������ÿ�ճ�ʼ���Ѿ������˽ڼ��յ����
 * @date					2017/5/3
 * @author					barry
 */
class DataIOEngine : public I_DataHandle, public SimpleTask
{
private:
	DataIOEngine();
public:///< ���湹��ͳ�ʼ����ع���
	~DataIOEngine();

	/**
	 * @brief				��ȡ����
	 */
	static DataIOEngine&	GetEngineObj();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @param[in]			pIQuotation					��������ص�֪ͨ�Ľӿ�
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize( I_QuotationCallBack* pIQuotation );

	/**
	 * @brief				�ͷ�����ģ�����Դ
	 */
	void					Release();

public:///< ��־�ӿ�
	virtual void			WriteInfo( const char * szFormat,... );
	virtual void			WriteWarning( const char * szFormat,... );
	virtual void			WriteError( const char * szFormat,... );
	virtual void			WriteDetail( const char * szFormat,... );

public:///< I_DataHandle�ӿ�ʵ��: ���ڸ����ݲɼ�ģ���ṩ�������ݵĻص�����
	/**
 	 * @brief				��ʼ�����ʵ��������ݻص�
	 * @note				ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]			nDataID						��ϢID
	 * @param[in]			pData						��������
	 * @param[in]			nDataLen					����
	 * @param[in]			bLastFlag					�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return				==0							�ɹ�
							!=0							����
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				ʵ���������ݻص�
	 * @note				���������ڴ�飬������
	 * @param[in]			nDataID						��ϢID
	 * @param[in]			pData						��������
	 * @param[in]			nDataLen					����
	 * @param[in]			bPushFlag					���ͱ�ʶ
	 * @return				==0							�ɹ�
							!=0							����
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief				�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]			nDataID						��ϢID
	 * @param[in,out]		pData						��������(������ѯ����)
	 * @param[in]			nDataLen					����
	 * @return				>0							�ɹ�,�������ݽṹ�Ĵ�С
							==0							û�鵽���
							!=0							����
	 * @note				���pData�Ļ���Ϊ��ȫ�㡱���棬�򷵻ر��ڵ���������
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief				��־����
	 * @param[in]			nLogLevel					��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	 * @param[in]			pszFormat					�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

protected:///< �߳�������غ���
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	virtual int				Execute();

protected:
	QuotationNotify			m_oQuoNotify;					///< ��������֪ͨ�ص�
	DatabaseAdaptor			m_oDatabaseIO;					///< �ڴ����ݲ������
	DataCollectorPool		m_oDataCollectorPool;			///< ����ɼ�ģ����Դ��
	I_QuotationCallBack*	m_pQuotationCallBack;			///< ����ص�
};


#endif








