#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
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
	__declspec(dllexport) int __stdcall		Initialize( I_QuotationCallBack* pIDataHandle );

	/**
	 * @brief								�ͷ����ݲɼ�ģ��
	 */
	__declspec(dllexport) void __stdcall	Release();

	/**
	 * @brief								��ȡģ��ĵ�ǰ״̬
	 * @param[out]							pszStatusDesc				���س�״̬������
	 * @param[in,out]						nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return								����ģ�鵱ǰ״ֵ̬
	 */
	__declspec(dllexport) int __stdcall		GetStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief								��Ԫ���Ե�������
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





