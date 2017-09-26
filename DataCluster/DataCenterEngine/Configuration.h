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
	void								AddPath( std::string sDllPath, std::string sQuotationName, std::string sAddress );

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

	/**
	 * @brief							根据位置索引获取市场名称
	 * @param[in]						nPos				位置索引
	 * @return							市场描述
	 */
	std::string							GetMkNameByPos( unsigned int nPos );

	/**
	 * @brief							根据位置索引获取连接地址和端口
	 * @param[in]						nPos				位置索引
	 * @return							TCP连接信息
	 */
	std::string							GetTCPAddressByPos( unsigned int nPos );

protected:
	CriticalObject						m_oLock;						///< 锁
	std::vector<std::string>			m_vctMarketName;				///< 各市场名称
	std::vector<std::string>			m_vctTCPAddress;				///< 连接地址和端口
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
	 * @brief							获取数据采集模块的加载路径列表
	 */
	DllPathTable&						GetDCPathTable();

	/**
	 * @brief							行情存储/恢复文件的目录
	 */
	const std::string&					GetRecoveryFolderPath() const;

protected:
	bool								m_bLoaded;						///< 加载标识
	DllPathTable						m_oDCPathTable;					///< 数据采集器加载地址表
	std::string							m_sMemPluginPath;				///< 数据内存块插件所在路径
	std::string							m_sRecoveryFolder;				///< 行情数据存储/恢复目录
};


#endif








