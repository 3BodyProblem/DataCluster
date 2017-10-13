#ifndef __CLIENT_WRAPPER__H__
#define __CLIENT_WRAPPER__H__


#include "../DataCenterEngine/DataCenterEngine.h"


/**
 * @class				EngineWrapper4DataClient
 * @brief				���ն˵��������ѯ��ʵʱ���ͽӿ�
 * @author				barry
 * @date				2017-10-10
 */
class EngineWrapper4DataClient : public I_DataHandle
{
private:
	EngineWrapper4DataClient();

public:
	static EngineWrapper4DataClient&	GetObj();

	/**
	 * @brief							�Ƿ���Client�ն�ģʽ������
	 */
	bool								IsUsed();

public:
	/**
	 * @brief							��ʼ�����ݲɼ���
	 * @detail							�������� + ������Ϣ�ص� + ������ϵ�ͨѶģ�� + �����Ͽ������߳�
	 * @param[in]						pIQuotation					��������ص�֪ͨ�Ľӿ�
	 * @return							==0							��ʼ���ɹ�
										!=0							����
	 */
	int									Initialize( I_QuotationCallBack* pIQuotation );

	/**
	 * @brief							�ͷ���Դ
	 */
	void								Release();

public:///< I_DataHandle�ӿ�ʵ��: ���ڸ����ݲɼ�ģ���ṩ�������ݵĻص�����
	/**
 	 * @brief							��ʼ�����ʵ��������ݻص�
	 * @note							ֻ�Ǹ��¹�����������ݵ��ڴ��ʼ�ṹ��������
	 * @param[in]						nDataID						��ϢID
	 * @param[in]						pData						��������
	 * @param[in]						nDataLen					����
	 * @param[in]						bLastFlag					�Ƿ����г�ʼ�������Ѿ����꣬����Ϊ���һ���ģ���ʶ
	 * @return							==0							�ɹ�
										!=0							����
	 */
	virtual int							OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief							ʵ���������ݻص�
	 * @note							���������ڴ�飬������
	 * @param[in]						nDataID						��ϢID
	 * @param[in]						pData						��������
	 * @param[in]						nDataLen					����
	 * @param[in]						bPushFlag					���ͱ�ʶ
	 * @return							==0							�ɹ�
										!=0							����
	 */
	virtual int							OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief							�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]						nDataID						��ϢID
	 * @param[in,out]					pData						��������(������ѯ����)
	 * @param[in]						nDataLen					����
	 * @return							>0							�ɹ�,�������ݽṹ�Ĵ�С
										==0							û�鵽���
										!=0							����
	 * @note							���pData�Ļ���Ϊ��ȫ�㡱���棬�򷵻ر��ڵ���������
	 */
	virtual int							OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief							��־����
	 * @param[in]						nLogLevel					��־����[0=��Ϣ��1=������־��2=������־��3=��ϸ��־]
	 * @param[in]						pszFormat					�ַ�����ʽ����
	 */
	virtual void						OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

public:
	/**
	 * @brief							��ȡ���ݿ����
	 */
	BigTableDatabase&					GetDatabaseObj();

	/**
	 * @brief							�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]						nDataID				��ϢID
	 * @param[in,out]					pData				��������(������ѯ����)
	 * @param[in]						nDataLen			����
	 * @return							>0					�ɹ�,�������ݽṹ�Ĵ�С
										==0					û�鵽���
										!=0					����
	 */
	void								OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );

protected:
	I_QuotationCallBack*				m_pQuotationCallBack;		///< �����ݱ������ص�
	BigTableDatabase					m_oDB4ClientMode;			///< �ڴ����ݲ������
	QuotationNotify						m_oQuoNotify;				///< ��������֪ͨ�ص�
};

#endif











