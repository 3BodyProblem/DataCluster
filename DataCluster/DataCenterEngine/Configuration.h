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
	 * @brief							����dll·��
	 */
	void								AddPath( std::string sDllPath, std::string sQuotationName, std::string sAddress );

	/**
	 * @brief							��ȡ���õ�·������
	 */
	unsigned int						GetCount();

	/**
	 * @brief							����λ��������ȡ���ص�ַ
	 * @param[in]						nPos				λ������
	 * @return							ģ����ص�ַ
	 */
	std::string							GetPathByPos( unsigned int nPos );

	/**
	 * @brief							����λ��������ȡ�г�����
	 * @param[in]						nPos				λ������
	 * @return							�г�����
	 */
	std::string							GetMkNameByPos( unsigned int nPos );

	/**
	 * @brief							����λ��������ȡ���ӵ�ַ�Ͷ˿�
	 * @param[in]						nPos				λ������
	 * @return							TCP������Ϣ
	 */
	std::string							GetTCPAddressByPos( unsigned int nPos );

protected:
	CriticalObject						m_oLock;						///< ��
	std::vector<std::string>			m_vctMarketName;				///< ���г�����
	std::vector<std::string>			m_vctTCPAddress;				///< ���ӵ�ַ�Ͷ˿�
};


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
	 * @brief							�ڴ���ģ��·��
	 */
	const std::string&					GetMemPluginPath() const;

	/**
	 * @brief							��ȡ���ݲɼ�ģ��ļ���·���б�
	 */
	DllPathTable&						GetDCPathTable();

	/**
	 * @brief							����洢/�ָ��ļ���Ŀ¼
	 */
	const std::string&					GetRecoveryFolderPath() const;

protected:
	bool								m_bLoaded;						///< ���ر�ʶ
	DllPathTable						m_oDCPathTable;					///< ���ݲɼ������ص�ַ��
	std::string							m_sMemPluginPath;				///< �����ڴ��������·��
	std::string							m_sRecoveryFolder;				///< �������ݴ洢/�ָ�Ŀ¼
};


#endif








