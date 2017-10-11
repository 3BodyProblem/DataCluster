#ifndef __NODE_WRAPPER__H__
#define __NODE_WRAPPER__H__


#include "../DataCenterEngine/DataCenterEngine.h"


/**
 * @class				InterfaceWrapper4DataNode
 * @brief				��DataNode.exe���õĽӿڵ������ܷ�װ��
 * @author				barry
 * @date				2017-10-10
 */
class InterfaceWrapper4DataNode : public I_QuotationCallBack
{
private:
	InterfaceWrapper4DataNode();

public:
	static InterfaceWrapper4DataNode&	GetObj();

	/**
	 * @brief							�Ƿ�DataNode.exe����
	 */
	bool								IsUsed();

public:
	/**
	 * @brief							��ʼ�����ݲɼ���
	 * @detail							�������� + ������Ϣ�ص� + ������ϵ�ͨѶģ�� + �����Ͽ������߳�
	 * @param[in]						pIDataHandle				����ص��ӿ�
	 * @return							==0							��ʼ���ɹ�
										!=0							����
	 */
	int									Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief							�ͷ���Դ
	 */
	void								Release();

	/**
	 * @brief							����socket��������������պ�����
	 * @note							��һ��ͬ���ĺ������������ʼ����ɺ�Ż᷵��
	 * @return							==0							�ɹ�
										!=0							����
	 */
	int									RecoverQuotation();

	/**
	 * @brief							�Ͽ�����Դsocket����
	 */
	void								Halt();

	/**
	 * @brief							ȡ�òɼ�ģ��ĵ�ǰ״̬
 	 * @param[out]						pszStatusDesc				���س�״̬������
	 * @param[in,out]					nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return							����ģ�鵱ǰ״ֵ̬
	 */
	enum E_SS_Status					GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

protected:
	/**
	 * @brief							ʵ���������ݻص�
	 * @note							���������ڴ�飬������
	 * @param[in]						eMarketID					�г�ID
	 * @param[in]						nDataID						��ϢID
	 * @param[in]						pData						��������
	 * @param[in]						nDataLen					����
	 * @param[in]						bPushFlag					���ͱ�ʶ
	 * @return							==0							�ɹ�
										!=0							����
	 */
	virtual void						OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );

	/**
	 * @brief							�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]						nDataID						��ϢID
	 * @param[in,out]					pData						��������(������ѯ����)
	 * @param[in]						nDataLen					����
	 * @return							>0							�ɹ�,�������ݽṹ�Ĵ�С
										==0							û�鵽���
										!=0							����
	 */
	virtual void						OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );

	/**
	 * @brief							��־����
	 * @param[in]						nLogLevel					��־����[0=��Ϣ��1=��Ϣ���桢2=������־��3=������־��4=��ϸ��־]
	 * @param[in]						pszFormat					�ַ�����ʽ����
	 */
	virtual void						OnLog( unsigned char nLogLevel, const char* pszLogBuf );

protected:
	I_DataHandle*						m_pDataHandle;				///< ����ص��ӿ�

};

#endif











