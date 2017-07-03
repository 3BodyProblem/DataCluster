#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <map>
#include <string>
#include "Configuration.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../DataCollector/Interface.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../DataCollector/DataCollector.h"


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
public:///< ���湹��ͳ�ʼ����ع���
	DataIOEngine();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize();

	/**
	 * @brief				�ͷ�����ģ�����Դ
	 */
	void					Release();

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

	/**
	 * @brief				���¼���/��ʼ������(�ڴ��������ݲɼ���)
	 * @detail				��ʼ�����ֵ�����ҵ�����̶��������������
	 * @return				true						��ʼ���ɹ�
							false						ʧ��
	 */
	bool					PrepareQuotation();

protected:
	DatabaseIO				m_oDatabaseIO;					///< �ڴ����ݲ������
	DataCollectorPool		m_oDataCollectorPool;			///< ����ɼ�ģ����Դ��
};


/**
 * @class					DataCenterEngine
 * @brief					�������������	(������)
 * @detail					��չ��Ϊ���������Ҫ��һЩ���ݸ��¹���������߼����ܣ�
							a) ��������/ͣ����
							b) ��������������ݶ�ʱ���̱���
							c) ���������״̬��ʱͨ��
 * @note					���ڹ����˴�������������������ԭ���ǳ��˳�ʼ��������ʱ�䶼һֱ�������Ӳ��Ͽ�!
 * @date					2017/5/3
 * @author					barry
 */
class DataCenterEngine : public DataIOEngine
{
private:
	DataCenterEngine();
public:
	~DataCenterEngine();

	/**
	 * @brief				ȡ�÷������ĵ�������
	 */
	static DataCenterEngine&	GetSerivceObj();

	/**
	 * @brief				�߳��Ƿ�����
	 * @return				true					���ڹ�����
							false					�ǹ���״̬
	 */
	bool					IsServiceAlive();

public:
	/**
	 * @brief				��ʼ��&�����������
	 * @return				==0						�����ɹ�
							!=0						��������
	 */
	int						Activate();

	/**
	 * @brief				�����������
	 */
	void					Destroy();

public:
	virtual void			WriteInfo( const char * szFormat,... );
	virtual void			WriteWarning( const char * szFormat,... );
	virtual void			WriteError( const char * szFormat,... );
	virtual void			WriteDetail( const char * szFormat,... );

protected:
	bool					m_bActivated;					///< ���񼤻��ʶ
};





#endif








