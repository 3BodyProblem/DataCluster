#ifndef __DATA_NODE_SVRCONFIG_H__
#define	__DATA_NODE_SVRCONFIG_H__


#include <string>
#include <vector>
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


extern "C" HMODULE			g_oModule;


std::string GetModulePath( void* hModule );


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
	 * @brief							行情存储/恢复文件的目录
	 */
	const std::string&					GetRecoveryFolderPath() const;

	/**
	 * @brief							内存插件模块路径
	 */
	const std::string&					GetMemPluginPath() const;

	/**
	 * @brief							数据采集插件路径
	 */
	const std::string&					GetDataCollectorPluginPath() const;

protected:
	std::string							m_sMemPluginPath;				///< 数据内存块插件所在路径
	std::string							m_sDataCollectorPluginPath;		///< 数据采集插件所在路径
	std::string							m_sRecoveryFolder;				///< 行情数据存储/恢复目录
};


#endif








