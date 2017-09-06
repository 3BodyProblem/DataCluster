#ifndef __DATE_CLUSTER_INTERFACE_H__
#define	__DATE_CLUSTER_INTERFACE_H__


#include "Protocal/DataCluster_Protocal.h"


/**
 * @class					I_QuotationCallBack
 * @brief					���ݻص��ӿ�
 * @date					2017/6/28
 * @author					barry
 */
class I_QuotationCallBack
{
public:
	/**
	 * @brief				ʵ���������ݻص�
	 * @note				���������ڴ�飬������
	 * @param[in]			eMarketID			�г�ID
	 * @param[in]			nDataID				��ϢID
	 * @param[in]			pData				��������
	 * @param[in]			nDataLen			����
	 * @param[in]			bPushFlag			���ͱ�ʶ
	 * @return				==0					�ɹ�
							!=0					����
	 */
	virtual void			OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen ) = 0;

	/**
	 * @brief				�ڴ����ݲ�ѯ�ӿ�
	 * @param[in]			nDataID				��ϢID
	 * @param[in,out]		pData				��������(������ѯ����)
	 * @param[in]			nDataLen			����
	 * @return				>0					�ɹ�,�������ݽṹ�Ĵ�С
							==0					û�鵽���
							!=0					����
	 */
	virtual void			OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus ) = 0;

	/**
	 * @brief				��־����
	 * @param[in]			nLogLevel			��־����[0=��Ϣ��1=��Ϣ���桢2=������־��3=������־��4=��ϸ��־]
	 * @param[in]			pszFormat			�ַ�����ʽ����
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf ) = 0;
};


///< -------------------------- ���ݷ��ʲ�������ӿ� ----------------------------------


/**
 * @brief					��ʼ�����ݲɼ�ģ��
 * @param[in]				pIDataHandle				���鹦�ܻص�
 * @return					==0							��ʼ���ɹ�
							!=							����
 */
typedef int					(__stdcall *T_Func_Activate)( I_QuotationCallBack* pIDataHandle );

/**
 * @brief					�ͷ����ݲɼ�ģ��
 */
typedef void				(__stdcall *T_Func_Destroy)();

/**
 * @brief					��ȡģ��ĵ�ǰ״̬
 * @param[out]				pszStatusDesc				���س�״̬������
 * @param[in,out]			nStrLen						�������������泤�ȣ������������Ч���ݳ���
 * @return					����ģ�鵱ǰ״ֵ̬
 */
typedef int					(__stdcall *T_Func_Query)( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );

/**
 * @brief					�ͷ����ݲɼ�ģ��
 */
typedef void				(__stdcall *T_Func_ExecuteUnitTest)();









//-----------------------------------------------------------------------------------------------------------------------------
//��ȡ��ǰ����ӿڰ汾
//��������
//���أ���ǰ�汾��
typedef int  __stdcall tagQUOFun_GetVersion(void);
//.............................................................................................................................
//��������ӿ�
//���������ݡ�״̬����־�ص��ӿڣ����룩
//���أ�����>=0��ʾ�ɹ���<0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
typedef int  __stdcall tagQUOFun_StartWork(I_QuotationCallBack * lpSpi);
//.............................................................................................................................
//ֹͣ����ӿ�
//��������
//���أ���
typedef void __stdcall tagQUOFun_EndWork(void);
//.............................................................................................................................
//��ȡ����ӿ�֧�ֵ��г���Ϣ
//�������г�������飨��������г������ֵ���������룩
//���������>=0��ʾ����ӿ�֧�ֵ��г�����������<=0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
typedef int  __stdcall tagQUOFun_GetMarketID(QUO_MARKET_ID * lpOut,unsigned int uiSize);
//.............................................................................................................................
//��ȡָ���г����г���Ϣ
//�������г���ţ����룩�������г���Ϣ�������
//���أ�����>=0��ʾ�ɹ�������<0��ʾʧ��
typedef int  __stdcall tagQUOFun_GetMarketInfo(QUO_MARKET_ID eMarketID,tagQUO_MarketInfo * lpOut);
//.............................................................................................................................
//��ȡָ���г��ο����ݣ���̬���ݻ�������ݣ�
//�������г���ţ����룩����ʼ��ţ����룩���ο���������ָ�루��������ο���������֧�ֵ����������룩�����Ҫ��ȡȫ�г��Ĳο����ݣ�������г���Ϣ�е���Ʒ������Ϊ��������
//���أ�����>=0��ʾ�ο�����ʵ�ʵ�����������<0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
typedef int  __stdcall tagQUOFun_GetAllReferenceData(QUO_MARKET_ID eMarketID,unsigned int uiOffset,tagQUO_ReferenceData * lpOut,unsigned int uiSize);
//.............................................................................................................................
//��ȡָ���г�ָ����Ʒ�Ĳο����ݣ���̬���ݻ�������ݣ�
//�������г���ţ����룩����Ʒ���루���룩��������Ʒ�ο����ݣ������
//���أ�����>=0��ʾ�ɹ�������<0��ʾʧ��
typedef int  __stdcall tagQUOFun_GetReferenceData(QUO_MARKET_ID eMarketID,const char * szCode,tagQUO_ReferenceData * lpOut);
//.............................................................................................................................
//��ȡָ���г���������
//�������г���ţ����룩����ʼ��ţ����룩��������������ָ�루�������������������֧�ֵ����������룩�����Ҫ��ȡȫ�г��Ŀ������ݣ�������г���Ϣ�е���Ʒ������Ϊ��������
//���أ�����>=0��ʾ��������ʵ�ʵ�����������<0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
typedef int  __stdcall tagQUOFun_GetAllSnapData(QUO_MARKET_ID eMarketID,unsigned int uiOffset,tagQUO_SnapData * lpOut,unsigned int uiSize);
//.............................................................................................................................
//��ȡָ���г�ָ����Ʒ�Ŀ�������
//�������г���ţ����룩����Ʒ���루���룩��������Ʒ�������ݣ������
//���أ�����>=0��ʾ�ɹ�������<0��ʾʧ��
typedef int  __stdcall tagQUOFun_GetSnapData(QUO_MARKET_ID eMarketID,const char * szCode,tagQUO_SnapData * lpOut);
//.............................................................................................................................
/*
������˵����

1��װ�ض�̬���ӿ⣬Ȼ��ӳ�����Ϻ���
2��������׼�������MQUO_Spi�ӿڣ��������ݴ�����־�����
3������tagQUOFun_StartWork��������ӿڣ�ע�⣺�����������������󣬾Ϳ���ȷ�����ӿ���֧����Щ�г���
4������tagQUOFun_GetMarketID��ȡ����ӿ���֧�ֵ��г�����
	QUO_MARKET_ID	marketid[32];
	tagQUOFun_GetMarketID(marketid,32);	
5���ȴ�MQUO_Spi�ӿڻص�����OnStatus��֪ͨĳ�г��Ѿ���ʼ������
6������tagQUOFun_GetMarketInfo��ȡ���г�����Ϣ���������ڡ�ʱ�䡢�����Ʒ��������Ϣ��
	tagQUO_MarketInfo	marketinfo;
	tagQUOFun_GetMarketInfo(QUO_MARKET_SSE,&marketinfo);
7������tagQUOFun_GetReferenceData��tagQUOFun_GetSnapData��ȡ�����ο����ݻ��������
	tagQUO_ReferenceData * lpreferencedata = new tagQUO_ReferenceData[marketinfo.uiWareCount];
	tagQUOFun_GetReferenceData(QUO_MARKET_SSE,0,lpreferencedata,marketinfo.uiWareCount);
8������tagQUOFun_GetReferenceData��tagQUOFun_GetSnapData��ȡ�����ο����ݻ��������
	tagQUO_ReferenceData	referencedata;
	tagQUOFun_GetReferenceData(QUO_MARKET_SSE,"600000",referencedata);
9���ȴ�MQUO_Spi�ӿڻص�����OnQuotation�������������
10��ʹ����Ϻ����tagQUOFun_EndWorkֹͣ����ӿ�

ע�⣺������ӿ��б����ʹ洢һ��ʵʱ��������գ���Ҫʱֱ�ӵ��ýӿڽ��л�ȡ��Ӧ�ò㲻��Ҫ�����Ᵽ��һ����ͬ�����ݣ�Ӧ�ò�����������ݳ��⣩
*/














#endif









