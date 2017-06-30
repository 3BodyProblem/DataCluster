#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "DataCollector/Interface.h"


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
	__declspec(dllexport) int __stdcall		Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief								�ͷ����ݲɼ�ģ��
	 */
	__declspec(dllexport) void __stdcall	Release();

	/**
	 * @brief								���³�ʼ����������������
	 * @note								��һ��ͬ���ĺ������������ʼ����ɺ�Ż᷵��
 	 * @return								==0							�ɹ�
											!=0							����
	 */
	__declspec(dllexport) int __stdcall		RecoverQuotation();

	/**
	 * @brief								��ʱ���ݲɼ�
	 */
	__declspec(dllexport) void __stdcall	HaltQuotation();

	/**
	 * @brief								��ȡģ��ĵ�ǰ״̬
	 * @param[out]							pszStatusDesc				���س�״̬������
	 * @param[in,out]						nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return								����ģ�鵱ǰ״ֵ̬
	 */
	__declspec(dllexport) int __stdcall		GetStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief								��ȡ�г����
	 * @return								�г�ID
	 */
	__declspec(dllexport) int __stdcall		GetMarketID();

	/**
	 * @brief								�Ƿ�Ϊ���鴫��Ĳɼ���
	 * @return								true						�Ǵ���ģ�������ɼ����
											false						����Դ������ɼ����
	 */
	__declspec(dllexport) bool __stdcall	IsProxy();

	/**
	 * @brief								��Ԫ���Ե�������
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





