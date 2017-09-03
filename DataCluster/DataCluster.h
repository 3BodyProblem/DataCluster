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
	 * @brief								��ʼ�����ݲɼ�ģ��
	 * @param[in]							pIDataHandle				���鹦�ܻص�
	 * @return								==0							��ʼ���ɹ�
											!=							����
	 */
	__declspec(dllexport) int __stdcall		Activate( I_QuotationCallBack* pIDataHandle );

	/**
	 * @brief								�ͷ����ݲɼ�ģ��
	 */
	__declspec(dllexport) void __stdcall	Destroy();

	/**
	 * @brief								��ȡģ��ĵ�ǰ״̬
	 * @param[out]							nMessageID					��ϢID
	 * @param[in,out]						pDataPtr					���ݵ�ַ,�����������ݻ����ײ�������Ʒ���룬��Ϊ������ѯ����
	 * @param[in]							nDataLen					���ݳ���
	 * @return								>=0							���ز�ѯ������message����
											<0							����
	 */
	__declspec(dllexport) int __stdcall		Query( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );

	/**
	 * @brief								��Ԫ���Ե�������
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();

	///< -------------------- �����Ͻӿ�QuoClientApi.Dll ------------------------------------------
	extern MPrimeClient						Global_PrimeClient;
	extern bool								Global_bInit;
	extern MDataClient						Global_Client;
	__declspec(dllexport) const char*		GetDllVersion( int &nMajorVersion, int &nMinorVersion );
	__declspec(dllexport) QuoteClientApi*	CreateQuoteApi( const char* pszDebugPath );
	__declspec(dllexport) QuotePrimeApi*	CreatePrimeApi();
	__declspec(dllexport) int				GetSettingInfo( tagQuoteSettingInfo* pArrMarket, int nCount );
}




#endif





