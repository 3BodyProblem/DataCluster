#ifndef __DATA_NODE_SVRCONFIG_H__
#define	__DATA_NODE_SVRCONFIG_H__


#include <string>
#include <vector>
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


extern "C" HMODULE			g_oModule;


std::string GetModulePath( void* hModule );


class DllPathTable : protected std::vector<std::string>
{
public:
	/**
	 * @brief							新添dll路径
	 */
	void								AddPath( std::string sDllPath );

	/**
	 * @brief							获取配置的路径数量
	 */
	unsigned int						GetCount();

	/**
	 * @brief							根据位置索引获取加载地址
	 * @param[in]						nPos				位置索引
	 * @return							模块加载地址
	 */
	std::string							GetPathByPos( unsigned int nPos );

protected:
	CriticalObject						m_oLock;						///< 锁
};


/**
 * @class								Configuration
 * @brief								节点服务器的配置信息管理类
 * @date								2017/5/4
 * @author								barry
 */
class Configuration
{
private:
	Configuration();

public:
	/**
	 * @brief							取得配置对象的单键引用
	 */
	static Configuration&				GetConfigObj();

	/**
	 * @brief							初始化加载配置
	 * @return							==0				成功
										!=				失败
	 */
	int									Load();

public:
	/**
	 * @brief							内存插件模块路径
	 */
	const std::string&					GetMemPluginPath() const;

	/**
	 * @brief							数据采集插件路径
	 */
	const std::string&					GetDataCollectorPluginPath() const;

protected:
	DllPathTable						m_oDCPathTable;					///< 数据采集器加载地址表
	std::string							m_sMemPluginPath;				///< 数据内存块插件所在路径
	std::string							m_sDataCollectorPluginPath;		///< 数据采集插件所在路径
};


#endif








