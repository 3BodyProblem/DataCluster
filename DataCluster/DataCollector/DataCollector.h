#ifndef __DATA_COLLECTOR_H__
#define	__DATA_COLLECTOR_H__


#include <map>
#include <string>
#include <vector>
#include "../../../DataNode/DataNode/Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Protocal/DataCluster_Protocal.h"


/**
 * @class				CollectorStatus
 * @brief				当前行情会话的状态
 * @detail				服务框架需要通过这个判断（组合初始化策略实例）来判断是否需要重新初始化等动作
 * @note				状态变化的时候，会通知回调接口
 * @author				barry
 */
class CollectorStatus
{
public:
	CollectorStatus();

public:
	enum E_SS_Status		Get() const;

	bool					Set( enum E_SS_Status eNewStatus );

	void					SetMkID( unsigned int nMkID );

	unsigned int			GetMkID();

private:
	mutable CriticalObject	m_oCSLock;
	enum QUO_MARKET_STATUS	m_eMkStatus;		///< 市场状态
	enum E_SS_Status		m_eStatus;			///< 当前行情逻辑状态，用于判断当前该做什么操作了
	unsigned int			m_nMarketID;		///< 数据采集器对应的市场ID
};


/**
 * @class					DataCollector
 * @brief					数据采集模块控制注册接口
 * @note					采集模块只提供三种形式的回调通知( I_DataHandle: 初始化映像数据， 实时行情数据， 初始化完成标识 ) + 重新初始化方法函数
 * @date					2017/5/3
 * @author					barry
 */
class DataCollector : public SimpleTask
{
public:
	DataCollector();
	~DataCollector();

	/**
	 * @brief				数据采集模块初始化
	 * @param[in]			pIDataCallBack				行情回调接口
	 * @param[in]			sDllPath					数据采集模块的加载路径
	 * @param[in]			sMkName						市场名称
	 * @return				==0							成功
							!=0							错误
	 */
	int						Initialize( I_DataHandle* pIDataCallBack, std::string sDllPath, std::string sMkName );

	/**
	 * @breif				数据采集模块释放退出
	 */
	void					Release();

public:///< 数据采集模块事件定义
	/**
 	 * @brief				初始化/重新初始化回调
	 * @note				同步函数，即函数返回后，即初始化操作已经做完，可以判断执行结果是否为“成功”
	 * @return				==0							成功
							!=0							错误
	 */
	int						RecoverDataCollector();

	/**
	 * @brief				暂停数据采集器
	 */
	void					HaltDataCollector();

	/**
	 * @biref				取得当前数据采集模块状态
	 * @param[out]			pszStatusDesc				返回出状态描述串
	 * @param[in,out]		nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return				E_SS_Status状态值
	 */
	enum E_SS_Status		InquireDataCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief				获取市场编号
	 */
	unsigned int			GetMarketID();

	/**
	 * @brief				是否为行情传输的采集插件
	 */
	bool					IsProxy();

	/**
	 * @brief				是否在活动中
	 */
	bool					IsAlive();

	/**
	 * @brief				获取DLL路径
	 */
	const std::string&		GetDllPath();

	/**
	 * @brief				获取市场名称
	 */
	const std::string&		GetMkName();

protected:///< 线程任务相关函数
	/**
	 * @brief				任务函数(内循环)
	 * @return				==0							成功
							!=0							失败
	 */
	virtual int				Execute();

protected:
	std::string				m_sMkName;						///< 市场名称
	std::string				m_sDllPath;						///< DLL路径信息
	bool					m_bActivated;					///< 是否已经激活
	bool					m_bIsProxyPlugin;				///< 是否为传输代理插件
	CollectorStatus			m_oCollectorStatus;				///< 数据采集模块的状态
protected:
	Dll						m_oDllPlugin;					///< 插件加载类
	T_Func_Initialize		m_pFuncInitialize;				///< 数据采集器初始化接口
	T_Func_Release			m_pFuncRelease;					///< 数据采集器释放接口
	T_Func_RecoverQuotation	m_pFuncRecoverQuotation;		///< 数据采集器行情数据重新初始化接口
	T_Func_HaltQuotation	m_pFuncHaltQuotation;			///< 数据采集器暂停接口
	T_Func_GetStatus		m_pFuncGetStatus;				///< 数据采集器状态获取接口
	T_Func_GetMarketID		m_pFuncGetMarketID;				///< 数据采集器对应的市场ID获取接口
	T_Func_IsProxy			m_pFuncIsProxy;					///< 数据采集器对应的模块类型获取接口
};


/**
 * @class					DataCollectorPool
 * @brief					数据采集模块池
 * @author					barry
 */
class DataCollectorPool : protected std::vector<DataCollector>
{
public:
	DataCollectorPool();
	~DataCollectorPool();

	/**
	 * @brief				根据配置文件中的数据采集器路径列表，依次初始化各插件
	 * @param[in]			pIDataCallBack					行情回调接口
	 * @return				>=0								初始化的数据采集插件数量
							<0								出错
	 * @note				必须能成功加载和初始化所有子插件
	 */
	int						Initialize( I_DataHandle* pIDataCallBack );

	/**
	 * @brief				释放资源
	 */
	void					Release();

public:
	/**
	 * @brief				维持各加载数据采集器的连接(24hr)
	 * @return				true							成功
	 */
	bool					PreserveAllConnection();

	/**
	 * @brief				判断是否已经全部启动可以服务
	 * @return				true							全部可服务
	 */
	bool					IsServiceWorking();

	/**
	 * @brief				有效数据采集器的数量
	 */
	unsigned int			GetCount();

public:
	/**
	 * @brief				根据市场编号取得数据采集模块
	 * @param[in]			nMkID							旧的市场编号ID
	 */
	DataCollector*			GetCollectorByMkID( unsigned int nMkID );

	static int				MkIDCast( unsigned int nOldMkID );
	static int				Cast2OldMkID( unsigned int nNewMkID );

protected:
	CriticalObject			m_oLock;						///< 锁
};





#endif








