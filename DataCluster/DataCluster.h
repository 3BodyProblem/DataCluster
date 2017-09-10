#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
#include "DataCollector/Interface.h"
#include "QuoteCltDef.h"
#include "QuoteClientApi.h"
#include "DataClient/DataClient.h"


/**
 * @brief						DLL�����ӿ�
 * @author						barry
 * @date						2017/4/1
 */
extern "C"
{
	/**
	 * @brief								��ȡ�汾��
	 */
	__declspec(dllexport) int __stdcall		GetVersionNo();

	/**
	 * @brief								��ʼ�����ݲɼ�ģ��
	 * @param[in]							pIDataHandle				���鹦�ܻص�
	 * @return								==0							��ʼ���ɹ�
											!=							����
	 */
	__declspec(dllexport) int __stdcall		StartWork( I_QuotationCallBack* pIDataHandle );

	/**
	 * @brief								�ͷ����ݲɼ�ģ��
	 */
	__declspec(dllexport) void __stdcall	EndWork();

	/**
	 * @brief								��ȡ����ӿ�֧�ֵ��г���Ϣ
	 * @param[out]							lpOut						�г��������
	 * @param[in]							uiSize						�г������ֵ����
	 * @return								���������>=0��ʾ����ӿ�֧�ֵ��г�����������<0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
	 */
	__declspec(dllexport) int  __stdcall	GetMarketID( QUO_MARKET_ID* lpOut, unsigned int uiSize );

	/**
	 * @brief								��ȡָ���г����г���Ϣ
	 * @param[in]							eMarketID					�г����
	 * @param[out]							lpOut						�����г���Ϣ
	 * @return								����>=0��ʾ�ɹ�������<0��ʾʧ��
	 */
	__declspec(dllexport) int  __stdcall	GetMarketInfo( QUO_MARKET_ID eMarketID, tagQUO_MarketInfo* lpOut );

	/**
	 * @brief								��ȡָ���г��ο����ݣ���̬���ݻ�������ݣ�
	 * @param[in]							eMarketID					�г����
	 * @param[in]							uiOffset					��ʼ���
	 * @param[out]							lpOut						�ο���������ָ��
	 * @param[in]							uiSize						�ο���������֧�ֵ�����
	 * @return								����>=0��ʾ�ο�����ʵ�ʵ�����������<0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
	 * @note								���Ҫ��ȡȫ�г��Ĳο����ݣ�������г���Ϣ�е���Ʒ������Ϊ��������
	 */
	__declspec(dllexport) int  __stdcall	GetAllReferenceData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_ReferenceData* lpOut, unsigned int uiSize );

	/**
	 * @brief								��ȡָ���г��ο����ݣ���̬���ݻ�������ݣ�
	 * @param[in]							eMarketID					�г����
	 * @param[in]							szCode						��Ʒ����
	 * @param[out]							lpOut						������Ʒ�ο�����
	 * @return								����>=0��ʾ�ɹ�������<0��ʾʧ��
	 */
	__declspec(dllexport) int  __stdcall	GetReferenceData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_ReferenceData* lpOut );

	/**
	 * @brief								��ȡָ���г��ο����ݣ���̬���ݻ�������ݣ�
	 * @param[in]							eMarketID					�г����
	 * @param[in]							uiOffset					��ʼ���
	 * @param[out]							lpOut						������������ָ��
	 * @param[in]							uiSize						������������֧�ֵ�����
	 * @return								����>=0��ʾ�ο�����ʵ�ʵ�����������<0��ʾʧ�ܣ����������Ϣͨ���ص��ӿ����
	 * @note								���Ҫ��ȡȫ�г��Ŀ������ݣ�������г���Ϣ�е���Ʒ������Ϊ��������
	 */
	__declspec(dllexport) int  __stdcall	GetAllSnapData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_SnapData* lpOut, unsigned int uiSize );

	/**
	 * @brief								��ȡָ���г��ο����ݣ���̬���ݻ�������ݣ�
	 * @param[in]							eMarketID					�г����
	 * @param[in]							szCode						��Ʒ����
	 * @param[out]							lpOut						������Ʒ��������
	 * @return								����>=0��ʾ�ɹ�������<0��ʾʧ��
	 */
	__declspec(dllexport) int  __stdcall	GetSnapData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_SnapData* lpOut );

	/**
	 * @brief								��Ԫ���Ե�������
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();

	///< -------------------- �����Ͻӿ�QuoClientApi.Dll ------------------------------------------
	extern MPrimeClient						Global_PrimeClient;
	extern bool								Global_bInit;
	extern MDataClient						Global_Client;
	extern QuotationAdaptor					Global_CBAdaptor;
	extern QuoteClientSpi*					Global_pSpi;
	__declspec(dllexport) const char*		GetDllVersion( int &nMajorVersion, int &nMinorVersion );
	__declspec(dllexport) QuoteClientApi*	CreateQuoteApi( const char* pszDebugPath );
	__declspec(dllexport) QuotePrimeApi*	CreatePrimeApi();
	__declspec(dllexport) int				GetSettingInfo( tagQuoteSettingInfo* pArrMarket, int nCount );
}




#endif





