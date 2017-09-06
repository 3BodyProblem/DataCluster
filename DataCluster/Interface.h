#ifndef __DATE_CLUSTER_INTERFACE_H__
#define	__DATE_CLUSTER_INTERFACE_H__


#include "Protocal/DataCluster_Protocal.h"


/**
 * @class					I_QuotationCallBack
 * @brief					数据回调接口
 * @date					2017/6/28
 * @author					barry
 */
class I_QuotationCallBack
{
public:
	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			eMarketID			市场ID
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bPushFlag			推送标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual void			OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen ) = 0;

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID				消息ID
	 * @param[in,out]		pData				数据内容(包含查询主键)
	 * @param[in]			nDataLen			长度
	 * @return				>0					成功,返回数据结构的大小
							==0					没查到结果
							!=0					错误
	 */
	virtual void			OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus ) = 0;

	/**
	 * @brief				日志函数
	 * @param[in]			nLogLevel			日志类型[0=信息、1=信息报告、2=警告日志、3=错误日志、4=详细日志]
	 * @param[in]			pszFormat			字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf ) = 0;
};


///< -------------------------- 数据访问插件导出接口 ----------------------------------


/**
 * @brief					初始化数据采集模块
 * @param[in]				pIDataHandle				行情功能回调
 * @return					==0							初始化成功
							!=							出错
 */
typedef int					(__stdcall *T_Func_Activate)( I_QuotationCallBack* pIDataHandle );

/**
 * @brief					释放数据采集模块
 */
typedef void				(__stdcall *T_Func_Destroy)();

/**
 * @brief					获取模块的当前状态
 * @param[out]				pszStatusDesc				返回出状态描述串
 * @param[in,out]			nStrLen						输入描述串缓存长度，输出描述串有效内容长度
 * @return					返回模块当前状态值
 */
typedef int					(__stdcall *T_Func_Query)( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );

/**
 * @brief					释放数据采集模块
 */
typedef void				(__stdcall *T_Func_ExecuteUnitTest)();









//-----------------------------------------------------------------------------------------------------------------------------
//获取当前行情接口版本
//参数：无
//返回：当前版本号
typedef int  __stdcall tagQUOFun_GetVersion(void);
//.............................................................................................................................
//启动行情接口
//参数：数据、状态、日志回调接口（输入）
//返回：返回>=0表示成功，<0表示失败，具体错误信息通过回调接口输出
typedef int  __stdcall tagQUOFun_StartWork(I_QuotationCallBack * lpSpi);
//.............................................................................................................................
//停止行情接口
//参数：无
//返回：无
typedef void __stdcall tagQUOFun_EndWork(void);
//.............................................................................................................................
//获取行情接口支持的市场信息
//参数：市场编号数组（输出），市场编号数值数量（输入）
//输出：返回>=0表示行情接口支持的市场数量，返回<=0表示失败，具体错误信息通过回调接口输出
typedef int  __stdcall tagQUOFun_GetMarketID(QUO_MARKET_ID * lpOut,unsigned int uiSize);
//.............................................................................................................................
//获取指定市场的市场信息
//参数：市场编号（输入），当个市场信息（输出）
//返回：返回>=0表示成功，返回<0表示失败
typedef int  __stdcall tagQUOFun_GetMarketInfo(QUO_MARKET_ID eMarketID,tagQUO_MarketInfo * lpOut);
//.............................................................................................................................
//获取指定市场参考数据（静态数据或码表数据）
//参数：市场编号（输入），起始序号（输入），参考数据数组指针（输出），参考数据数组支持的数量（输入），如果要获取全市场的参考数据，请分配市场信息中的商品数量作为参数传入
//返回：返回>=0表示参考数据实际的数量，返回<0表示失败，具体错误信息通过回调接口输出
typedef int  __stdcall tagQUOFun_GetAllReferenceData(QUO_MARKET_ID eMarketID,unsigned int uiOffset,tagQUO_ReferenceData * lpOut,unsigned int uiSize);
//.............................................................................................................................
//获取指定市场指定商品的参考数据（静态数据或码表数据）
//参数：市场编号（输入），商品代码（输入），单个商品参考数据（输出）
//返回：返回>=0表示成功，返回<0表示失败
typedef int  __stdcall tagQUOFun_GetReferenceData(QUO_MARKET_ID eMarketID,const char * szCode,tagQUO_ReferenceData * lpOut);
//.............................................................................................................................
//获取指定市场快照数据
//参数：市场编号（输入），起始序号（输入），快照数据数组指针（输出），快照数据数组支持的数量（出入），如果要获取全市场的快照数据，请分配市场信息中的商品数量作为参数传入
//返回：返回>=0表示快照数据实际的数量，返回<0表示失败，具体错误信息通过回调接口输出
typedef int  __stdcall tagQUOFun_GetAllSnapData(QUO_MARKET_ID eMarketID,unsigned int uiOffset,tagQUO_SnapData * lpOut,unsigned int uiSize);
//.............................................................................................................................
//获取指定市场指定商品的快照数据
//参数：市场编号（输入），商品代码（输入），单个商品快照数据（输出）
//返回：返回>=0表示成功，返回<0表示失败
typedef int  __stdcall tagQUOFun_GetSnapData(QUO_MARKET_ID eMarketID,const char * szCode,tagQUO_SnapData * lpOut);
//.............................................................................................................................
/*
【调用说明】

1、装载动态链接库，然后映射以上函数
2、调用者准备好相关MQUO_Spi接口，包括数据处理、日志输出等
3、调用tagQUOFun_StartWork启动行情接口（注意：调用启动函数结束后，就可以确定本接口所支持哪些市场）
4、调用tagQUOFun_GetMarketID获取行情接口所支持的市场数量
	QUO_MARKET_ID	marketid[32];
	tagQUOFun_GetMarketID(marketid,32);	
5、等待MQUO_Spi接口回调函数OnStatus，通知某市场已经初始化结束
6、调用tagQUOFun_GetMarketInfo获取该市场的信息（包括日期、时间、类别、商品数量等信息）
	tagQUO_MarketInfo	marketinfo;
	tagQUOFun_GetMarketInfo(QUO_MARKET_SSE,&marketinfo);
7、调用tagQUOFun_GetReferenceData或tagQUOFun_GetSnapData获取批量参考数据或快照数据
	tagQUO_ReferenceData * lpreferencedata = new tagQUO_ReferenceData[marketinfo.uiWareCount];
	tagQUOFun_GetReferenceData(QUO_MARKET_SSE,0,lpreferencedata,marketinfo.uiWareCount);
8、调用tagQUOFun_GetReferenceData或tagQUOFun_GetSnapData获取单个参考数据或快照数据
	tagQUO_ReferenceData	referencedata;
	tagQUOFun_GetReferenceData(QUO_MARKET_SSE,"600000",referencedata);
9、等待MQUO_Spi接口回调函数OnQuotation，处理快照数据
10、使用完毕后调用tagQUOFun_EndWork停止行情接口

注意：本行情接口中本来就存储一份实时的行情快照，需要时直接调用接口进行获取，应用层不需要再另外保存一份相同的数据（应用层额外计算的数据除外）
*/














#endif









