#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <map>
#include <string>
#include "DataNotify.h"
#include "Configuration.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../DataCollector/Interface.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../MemoryDB/InnerTableFiller.h"
#include "../DataCollector/DataCollector.h"
#include "../../../DataCollector4CTPDL/DataCollector4CTPDL/CTP_DL_QuoProtocal.h"
#include "../../../DataCollector4CTPSH/DataCollector4CTPSH/CTP_SH_QuoProtocal.h"
#include "../../../DataCollector4CTPZZ/DataCollector4CTPZZ/CTP_ZZ_QuoProtocal.h"


/**
 * @class					DataIOEngine
 * @brief					行情数据更新管理引擎(主要封装数据初始化和更新/推送的业务)
 * @detail					集成/协调各子模块(数据采集插件+数据内存插件+数据压缩插件)
 * @note					主要提供三块行情数据相关的基础功能: 采集进来的行情更新到内存 + 行情数据初始化控制逻辑 + 行情数据对下级的网络框架封装
							&
							其中行情的每日初始化已经考虑了节假日的情况
 * @date					2017/5/3
 * @author					barry
 */
class DataIOEngine : public I_DataHandle, public SimpleTask
{
private:
	DataIOEngine();
public:///< 引擎构造和初始化相关功能
	~DataIOEngine();

	/**
	 * @brief				获取单键
	 */
	static DataIOEngine&	GetEngineObj();

	/**
 	 * @brief				初始化行情各参数，准备工作
	 * @param[in]			pIQuotation					用于行情回调通知的接口
	 * @return				==0							成功
							!=0							失败
	 */
	int						Initialize( I_QuotationCallBack* pIQuotation );

	/**
	 * @brief				释放行情模块各资源
	 */
	void					Release();

public:///< 日志接口
	virtual void			WriteInfo( const char * szFormat,... );
	virtual void			WriteWarning( const char * szFormat,... );
	virtual void			WriteError( const char * szFormat,... );
	virtual void			WriteDetail( const char * szFormat,... );

public:///< I_DataHandle接口实现: 用于给数据采集模块提供行情数据的回调方法
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]			nDataID						消息ID
	 * @param[in]			pData						数据内容
	 * @param[in]			nDataLen					长度
	 * @param[in]			bLastFlag					是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0							成功
							!=0							错误
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			nDataID						消息ID
	 * @param[in]			pData						数据内容
	 * @param[in]			nDataLen					长度
	 * @param[in]			bPushFlag					推送标识
	 * @return				==0							成功
							!=0							错误
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID						消息ID
	 * @param[in,out]		pData						数据内容(包含查询主键)
	 * @param[in]			nDataLen					长度
	 * @return				>0							成功,返回数据结构的大小
							==0							没查到结果
							!=0							错误
	 * @note				如果pData的缓存为“全零”缓存，则返回表内的所有数据
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief				日志函数
	 * @param[in]			nLogLevel					日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	 * @param[in]			pszFormat					字符串格式化串
	 */
	virtual void			OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

protected:///< 线程任务相关函数
	/**
	 * @brief				任务函数(内循环)
	 * @return				==0							成功
							!=0							失败
	 */
	virtual int				Execute();

protected:
	QuotationNotify			m_oQuoNotify;					///< 行情数据通知回调
	DatabaseAdaptor			m_oDatabaseIO;					///< 内存数据插件管理
	DataCollectorPool		m_oDataCollectorPool;			///< 行情采集模块资源池
	I_QuotationCallBack*	m_pQuotationCallBack;			///< 行情回调
};


#endif








