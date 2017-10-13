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
 * @brief					������ղ���Ĺ��� + ����ص��Ĳ���ָ��
 * @detail					��clientģʽ�� �� ��datanode.exe�ķ���ģʽ�£� ͨ����ͬ��wrapper����Խ��г�ʼ������
 * @date					2017/5/3
 * @author					barry
 */
class DataIOEngine : public SimpleTask
{
private:
	DataIOEngine();
public:///< ���湹��ͳ�ʼ����ع���
	~DataIOEngine();

	/**
	 * @brief				��ȡ����
	 */
	static DataIOEngine&	GetEngineObj();

	/**
 	 * @brief				��ʼ�������������׼������
	 * @param[in]			pIDataHandle				���ڴ���ת������ص���DataNode��װ���ָ��
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	int						Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief				�ͷ�����ģ�����Դ
	 */
	void					Release();

	/**
	 * @brief				��ȡ���ݲɼ�����
	 */
	DataCollectorPool&		GetCollectorPool();

public:///< ��־�ӿ�
	void					WriteInfo( const char * szFormat,... );
	void					WriteWarning( const char * szFormat,... );
	void					WriteError( const char * szFormat,... );
	void					WriteDetail( const char * szFormat,... );

protected:///< �߳�������غ���
	/**
	 * @brief				������(��ѭ��)
	 * @return				==0							�ɹ�
							!=0							ʧ��
	 */
	virtual int				Execute();

protected:
	DataCollectorPool		m_oDataCollectorPool;			///< ����ɼ�ģ����Դ��
};


#endif








