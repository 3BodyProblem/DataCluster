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
 * @brief				��ǰ����Ự��״̬
 * @detail				��������Ҫͨ������жϣ���ϳ�ʼ������ʵ�������ж��Ƿ���Ҫ���³�ʼ���ȶ���
 * @note				״̬�仯��ʱ�򣬻�֪ͨ�ص��ӿ�
 * @author				barry
 */
class CollectorStatus
{
public:
	CollectorStatus();

	CollectorStatus( const CollectorStatus& obj );

public:
	enum E_SS_Status		Get() const;

	bool					Set( enum E_SS_Status eNewStatus );

	void					SetMkID( unsigned int nMkID );

	unsigned int			GetMkID();

private:
	mutable CriticalObject	m_oCSLock;
	enum QUO_MARKET_STATUS	m_eMarketStatus;			///< �г�״̬
	enum E_SS_Status		m_eDataCollectorStatus;		///< ��ǰ�����߼�״̬�������жϵ�ǰ����ʲô������
	unsigned int			m_nMarketID;				///< ���ݲɼ�����Ӧ���г�ID
};


/**
 * @class					DataCollector
 * @brief					���ݲɼ�ģ�����ע��ӿ�
 * @note					�ɼ�ģ��ֻ�ṩ������ʽ�Ļص�֪ͨ( I_DataHandle: ��ʼ��ӳ�����ݣ� ʵʱ�������ݣ� ��ʼ����ɱ�ʶ ) + ���³�ʼ����������
 * @date					2017/5/3
 * @author					barry
 */
class DataCollector : public SimpleTask
{
public:
	DataCollector();
	~DataCollector();

	/**
	 * @brief				���ݲɼ�ģ���ʼ��
	 * @param[in]			pIDataCallBack				����ص��ӿ�
	 * @param[in]			sDllPath					���ݲɼ�ģ��ļ���·��
	 * @param[in]			sMkName						�г�����
	 * @param[in]			sTCPAddr					TCP������Ϣ
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						Initialize( I_DataHandle* pIDataCallBack, std::string sDllPath, std::string sMkName, std::string sTCPAddr );

	/**
	 * @breif				���ݲɼ�ģ���ͷ��˳�
	 */
	void					Release();

public:///< ���ݲɼ�ģ���¼�����
	/**
 	 * @brief				��ʼ��/���³�ʼ���ص�
	 * @note				ͬ�����������������غ󣬼���ʼ�������Ѿ����꣬�����ж�ִ�н���Ƿ�Ϊ���ɹ���
	 * @return				==0							�ɹ�
							!=0							����
	 */
	int						RecoverDataCollector();

	/**
	 * @brief				��ͣ���ݲɼ���
	 */
	void					HaltDataCollector();

	/**
	 * @biref				ȡ�õ�ǰ���ݲɼ�ģ��״̬
	 * @param[out]			pszStatusDesc				���س�״̬������
	 * @param[in,out]		nStrLen						�������������泤�ȣ������������Ч���ݳ���
	 * @return				E_SS_Status״ֵ̬
	 */
	enum E_SS_Status		InquireDataCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

	/**
	 * @brief				��ȡ�г����
	 */
	unsigned int			GetMarketID();

	/**
	 * @brief				�Ƿ�Ϊ���鴫��Ĳɼ����
	 */
	bool					IsProxy();

	/**
	 * @brief				�Ƿ��ڻ��
	 */
	bool					IsAlive();

	/**
	 * @brief				��ȡDLL·��
	 */
	const std::string&		GetDllPath();

	/**
	 * @brief				��ȡ�г�����
	 */
	const std::string&		GetMkName();

	/**
	 * @brief				��ȡ�г�������Ϣ
	 */
	const std::string&		GetTCPAddr();

protected:///< �߳�������غ���
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	virtual int				Execute();

protected:
	std::string				m_sMkName;						///< �г�����
	std::string				m_sDllPath;						///< DLL·����Ϣ
	std::string				m_sTCPAddr;						///< ������Ϣ
	bool					m_bActivated;					///< �Ƿ��Ѿ�����
	bool					m_bIsProxyPlugin;				///< �Ƿ�Ϊ���������
	CollectorStatus			m_oCollectorStatus;				///< ���ݲɼ�ģ���״̬
protected:
	Dll						m_oDllPlugin;					///< ���������
	T_Func_Initialize		m_pFuncInitialize;				///< ���ݲɼ�����ʼ���ӿ�
	T_Func_Release			m_pFuncRelease;					///< ���ݲɼ����ͷŽӿ�
	T_Func_RecoverQuotation	m_pFuncRecoverQuotation;		///< ���ݲɼ��������������³�ʼ���ӿ�
	T_Func_HaltQuotation	m_pFuncHaltQuotation;			///< ���ݲɼ�����ͣ�ӿ�
	T_Func_GetStatus		m_pFuncGetStatus;				///< ���ݲɼ���״̬��ȡ�ӿ�
	T_Func_GetMarketID		m_pFuncGetMarketID;				///< ���ݲɼ�����Ӧ���г�ID��ȡ�ӿ�
	T_Func_IsProxy			m_pFuncIsProxy;					///< ���ݲɼ�����Ӧ��ģ�����ͻ�ȡ�ӿ�
};


/**
 * @class					DataCollectorPool
 * @brief					���ݲɼ�ģ���
 * @author					barry
 */
class DataCollectorPool : protected std::vector<DataCollector>
{
public:
	DataCollectorPool();
	~DataCollectorPool();

	/**
	 * @brief				���������ļ��е����ݲɼ���·���б����γ�ʼ�������
	 * @param[in]			pIDataCallBack					����ص��ӿ�
	 * @return				>=0								��ʼ�������ݲɼ��������
							<0								����
	 * @note				�����ܳɹ����غͳ�ʼ�������Ӳ��
	 */
	int						Initialize( I_DataHandle* pIDataCallBack );

	/**
	 * @brief				�ͷ���Դ
	 */
	void					Release();

public:
	/**
	 * @brief				ά�ָ��������ݲɼ���������(24hr)
	 * @return				true							�ɹ�
	 */
	bool					PreserveAllConnection();

	/**
	 * @brief				�ж��Ƿ��Ѿ�ȫ���������Է���
	 * @return				true							ȫ���ɷ���
	 */
	bool					IsServiceWorking();

	/**
	 * @brief				��Ч���ݲɼ���������
	 */
	unsigned int			GetCount();

	/**
	 * @brief				��ȡ��Ч�Ŀɷ���ĻỰ����
	 */
	unsigned int			GetValidSessionCount();

public:
	/**
	 * @brief				�����г����ȡ�����ݲɼ�ģ��
	 * @param[in]			nMkID							�ɵ��г����ID
	 */
	DataCollector*			GetCollectorByMkID( unsigned int nMkID );

	static int				MkIDCast( unsigned int nOldMkID );
	static int				Cast2OldMkID( unsigned int nNewMkID );

protected:
	CriticalObject			m_oLock;						///< ��
	unsigned int			m_nValidCollector;				///< �����еĻỰ����
};





#endif








