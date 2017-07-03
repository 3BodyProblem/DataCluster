#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
#include "DataCollector/Interface.h"


/**
 * @brief						DLL导出接口
 * @author						barry
 * @date						2017/4/1
 */
extern "C"
{
	/**
	 * @brief								初始化数据采集模块
	 * @param[in]							pIDataHandle				行情功能回调
	 * @return								==0							初始化成功
											!=							出错
	 */
	__declspec(dllexport) int __stdcall		Activate( I_QuotationCallBack* pIDataHandle );

	/**
	 * @brief								释放数据采集模块
	 */
	__declspec(dllexport) void __stdcall	Destroy();

	/**
	 * @brief								获取模块的当前状态
	 * @param[out]							nMessageID					消息ID
	 * @param[in,out]						pDataPtr					数据地址,如果传入的数据缓存首部带有商品代码，则为主键查询操作
	 * @param[in]							nDataLen					数据长度
	 * @return								>=0							返回查询出来的message个数
											<0							出错
	 */
	__declspec(dllexport) int __stdcall		Query( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );

	/**
	 * @brief								单元测试导出函数
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();
}




#endif





