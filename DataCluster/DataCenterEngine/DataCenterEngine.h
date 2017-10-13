#ifndef __DATA_NODE_SERVER_H__
#define	__DATA_NODE_SERVER_H__


#include <map>
#include <string>
#include "DataNotify.h"
#include "Configuration.h"
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../MemoryDB/MemoryDatabase.h"
#include "../MemoryDB/InnerTableFiller.h"
#include "../DataCollector/DataCollector.h"
#include "../../../DataCollector4CTPDL/DataCollector4CTPDL/CTP_DL_QuoProtocal.h"
#include "../../../DataCollector4CTPSH/DataCollector4CTPSH/CTP_SH_QuoProtocal.h"
#include "../../../DataCollector4CTPZZ/DataCollector4CTPZZ/CTP_ZZ_QuoProtocal.h"
#include "../../../DataCollector4CTPEC/DataCollector4CTPEC/CTP_EC_QuoProtocal.h"
#include "../../../DataCollector4CTPDLOPT/DataCollector4CTPDLOPT/CTP_DLOPT_QuoProtocal.h"
#include "../../../DataCollector4CTPSHOPT/DataCollector4CTPSHOPT/CTP_SHOPT_QuoProtocal.h"
#include "../../../DataCollector4CTPZZOPT/DataCollector4CTPZZOPT/CTP_ZZOPT_QuoProtocal.h"
#include "../../../DataCollector4Tran2ndCFF/DataCollector4Tran2ndCFF/Tran2nd_CFF_QuoProtocal.h"
#include "../../../DataCollector4Tran2ndSHL1/DataCollector4Tran2ndSHL1/Tran2nd_SHL1_QuoProtocal.h"
#include "../../../DataCollector4Tran2ndSHOPT/DataCollector4Tran2ndSHOPT/Tran2nd_SHOPT_QuoProtocal.h"
#include "../../../DataCollector4Tran2ndSZL1/DataCollector4Tran2ndSZL1/Tran2nd_SZL1_QuoProtocal.h"


/**
 * @class					DataIOEngine
 * @brief					行情接收插件的管理 + 行情回调的策略指派
 * @detail					在client模式下 或 在datanode.exe的服务模式下， 通过不同的wrapper类策略进行初始化启动
 * @date					2017/5/3
 * @author					barry
 */
class DataIOEngine : public SimpleTask
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
	 * @param[in]			pIDataHandle				用于处理转发行情回调给DataNode包装类的指针
	 * @return				==0							成功
							!=0							失败
	 */
	int						Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief				释放行情模块各资源
	 */
	void					Release();

	/**
	 * @brief				获取数据采集集合
	 */
	DataCollectorPool&		GetCollectorPool();

public:///< 日志接口
	void					WriteInfo( const char * szFormat,... );
	void					WriteWarning( const char * szFormat,... );
	void					WriteError( const char * szFormat,... );
	void					WriteDetail( const char * szFormat,... );

protected:///< 线程任务相关函数
	/**
	 * @brief				任务函数(内循环)
	 * @return				==0							成功
							!=0							失败
	 */
	virtual int				Execute();

protected:
	DataCollectorPool		m_oDataCollectorPool;			///< 行情采集模块资源池
};


#endif








