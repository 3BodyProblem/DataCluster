#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


#include "Interface.h"
#include "QuoteCltDef.h"
#include "QuoteClientApi.h"
#include "DataClient/DataClient.h"
#include "../../DataNode/DataNode/Interface.h"


/**
 * @brief						DLL导出接口
 * @author						barry
 * @date						2017/4/1
 */
extern "C"
{
	/**
	 * @brief								获取版本号
	 */
	__declspec(dllexport) int __stdcall		GetVersionNo();

	/**
	 * @brief								初始化数据采集模块
	 * @param[in]							pIDataHandle				行情功能回调
	 * @return								==0							初始化成功
											!=							出错
	 */
	__declspec(dllexport) int __stdcall		StartWork( I_QuotationCallBack* pIDataHandle );

	/**
	 * @brief								释放数据采集模块
	 */
	__declspec(dllexport) void __stdcall	EndWork();

	/**
	 * @brief								获取行情接口支持的市场信息
	 * @param[out]							lpOut						市场编号数组
	 * @param[in]							uiSize						市场编号数值数量
	 * @return								输出：返回>=0表示行情接口支持的市场数量，返回<0表示失败，具体错误信息通过回调接口输出
	 */
	__declspec(dllexport) int  __stdcall	GetMarketIDTable( QUO_MARKET_ID* lpOut, unsigned int uiSize );

	/**
	 * @brief								获取指定市场的市场信息
	 * @param[in]							eMarketID					市场编号
	 * @param[out]							lpOut						当个市场信息
	 * @return								返回>=0表示成功，返回<0表示失败
	 */
	__declspec(dllexport) int  __stdcall	GetMarketInfo( QUO_MARKET_ID eMarketID, tagQUO_MarketInfo* lpOut );

	/**
	 * @brief								获取指定市场参考数据（静态数据或码表数据）
	 * @param[in]							eMarketID					市场编号
	 * @param[in]							uiOffset					起始序号
	 * @param[out]							lpOut						参考数据数组指针
	 * @param[in]							uiSize						参考数据数组支持的数量
	 * @return								返回>=0表示参考数据实际的数量，返回<0表示失败，具体错误信息通过回调接口输出
	 * @note								如果要获取全市场的参考数据，请分配市场信息中的商品数量作为参数传入
	 */
	__declspec(dllexport) int  __stdcall	GetAllReferenceData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_ReferenceData* lpOut, unsigned int uiSize );

	/**
	 * @brief								获取指定市场参考数据（静态数据或码表数据）
	 * @param[in]							eMarketID					市场编号
	 * @param[in]							szCode						商品代码
	 * @param[out]							lpOut						单个商品参考数据
	 * @return								返回>=0表示成功，返回<0表示失败
	 */
	__declspec(dllexport) int  __stdcall	GetReferenceData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_ReferenceData* lpOut );

	/**
	 * @brief								获取指定市场参考数据（静态数据或码表数据）
	 * @param[in]							eMarketID					市场编号
	 * @param[in]							uiOffset					起始序号
	 * @param[out]							lpOut						快照数据数组指针
	 * @param[in]							uiSize						快照数据数组支持的数量
	 * @return								返回>=0表示参考数据实际的数量，返回<0表示失败，具体错误信息通过回调接口输出
	 * @note								如果要获取全市场的快照数据，请分配市场信息中的商品数量作为参数传入
	 */
	__declspec(dllexport) int  __stdcall	GetAllSnapData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_SnapData* lpOut, unsigned int uiSize );

	/**
	 * @brief								获取指定市场参考数据（静态数据或码表数据）
	 * @param[in]							eMarketID					市场编号
	 * @param[in]							szCode						商品代码
	 * @param[out]							lpOut						单个商品快照数据
	 * @return								返回>=0表示成功，返回<0表示失败
	 */
	__declspec(dllexport) int  __stdcall	GetSnapData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_SnapData* lpOut );

	/**
	 * @brief								单元测试导出函数
	 */
	__declspec(dllexport) void __stdcall	ExecuteUnitTest();


	///< -------------------- 以下为被DataNode.exe调用的接口 --------------------------------------

	/**
	 * @brief								初始化数据采集模块
	 * @param[in]							pIDataHandle				行情功能回调
	 * @return								==0							初始化成功
											!=							出错
	 */
	__declspec(dllexport) int __stdcall		Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief								释放数据采集模块
	 */
	__declspec(dllexport) void __stdcall	Release();

	/**
	 * @brief								重新初始化并加载行情数据
	 * @note								是一个同步的函数，在行情初始化完成后才会返回
 	 * @return								==0							成功
											!=0							出错
	 */
	__declspec(dllexport) int __stdcall		RecoverQuotation();

	/**
	 * @brief								暂时数据采集
	 */
	__declspec(dllexport) void __stdcall	HaltQuotation();

	/**
	 * @brief								获取模块的当前状态
	 * @param[out]							pszStatusDesc				返回出状态描述串
	 * @param[in,out]						nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return								返回模块当前状态值
	 */
	__declspec(dllexport) int __stdcall		GetStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief								获取市场编号
	 * @return								市场ID
	 */
	__declspec(dllexport) int __stdcall		GetMarketID();

	/**
	 * @brief								是否为行情传输的采集器
	 * @return								true						是传输模块的行情采集插件
											false						顶层源的行情采集插件
	 */
	__declspec(dllexport) bool __stdcall	IsProxy();

	/**
	 * @brief								落盘数据分析回显
	 */
	__declspec(dllexport) void __stdcall	Echo();


	///< -------------------- 兼容老接口QuoClientApi.Dll ------------------------------------------
	extern MPrimeClient						Global_PrimeClient;
	extern bool								Global_bInit;
	extern MDataClient						Global_Client;
	extern QuotationAdaptor					Global_CBAdaptor;
	extern QuoteClientSpi*					Global_pSpi;
	__declspec(dllexport) const char*		__stdcall GetDllVersion( int &nMajorVersion, int &nMinorVersion );
	__declspec(dllexport) QuoteClientApi*	__stdcall CreateQuoteApi( const char* pszDebugPath );
	__declspec(dllexport) QuotePrimeApi*	__stdcall CreatePrimeApi();
	__declspec(dllexport) int				__stdcall GetSettingInfo( tagQuoteSettingInfo* pArrMarket, int nCount );
}




#endif





