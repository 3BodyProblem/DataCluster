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
 * @brief								�ڵ��������������Ϣ������
 * @date								2017/5/4
 * @author								barry
 */
class Configuration
{
private:
	Configuration();

public:
	/**
	 * @brief							ȡ�����ö���ĵ�������
	 */
	static Configuration&				GetConfigObj();

	/**
	 * @brief							��ʼ����������
	 * @return							==0				�ɹ�
										!=				ʧ��
	 */
	int									Load();

public:
	/**
	 * @brief							����洢/�ָ��ļ���Ŀ¼
	 */
	const std::string&					GetRecoveryFolderPath() const;

	/**
	 * @brief							�ڴ���ģ��·��
	 */
	const std::string&					GetMemPluginPath() const;

	/**
	 * @brief							���ݲɼ����·��
	 */
	const std::string&					GetDataCollectorPluginPath() const;

protected:
	std::string							m_sMemPluginPath;				///< �����ڴ��������·��
	std::string							m_sDataCollectorPluginPath;		///< ���ݲɼ��������·��
	std::string							m_sRecoveryFolder;				///< �������ݴ洢/�ָ�Ŀ¼
};


#endif








