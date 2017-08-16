#ifndef __INNER_TABLE_FILLER_H__
#define	__INNER_TABLE_FILLER_H__


#include <vector>
#include "../Protocal/DataCluster_Protocal.h"


/**
 * @class				InnerRecord
 * @brief				���ݴ���������Ϣ֮��Ľṹת�����������
 * @author				barry
 * @date				2017/8/8
 */
class InnerRecord
{
friend class TableFillerRegister;
public:
	InnerRecord( unsigned int nMsgID, unsigned int nMsgLen, unsigned int nBigTableID );

	/**
	 * @brief					��ȡ��Ϣ��¼��ID��
	 */
	unsigned int				GetMessageID();

	/**
	 * @brief					��ȡ��Ϣ��¼�ṹ����
	 */
	unsigned int				GetMessageLength();

	/**
	 * @brief					��ȡ���ID
	 */
	unsigned int				GetBigTableID();

	/**
	 * @brief					��ȡ�������ݽṹ���
	 */
	unsigned int				GetBigTableWidth();

	/**
	 * @brief					��ȡ��д�Ĵ���¼�ṹ��ַ
	 */
	char*						GetBigTableRecordPtr();

public:
	/**
	 * @brief					��Э����Ϣ�ṹ���������õ�����¼
								ÿ��������ϵ���Message��Ҫʵ��һ��������
	 */
	virtual void				FillMessage2BigTableRecord( char* pMessagePtr ) = 0;

protected:
	unsigned int				m_nMessageID;			///< ��ϢID
	unsigned int				m_nMessageLength;		///< ��Ϣ����
	unsigned int				m_nBigTableID;			///< ���ݴ���ID
protected:
	union BigTableRecord {
		tagQuoMarketInfo		MarketData_1;
		tagQuoCategory			CategoryData_2;
		tagQuoReferenceData		ReferenceData_3;
		tagQuoSnapData			SnapData_4;
	}							m_objUnionData;			///< ������ݽṹ������
};


///< -----------------------------------------------------------------------------


/**
 * @class				TableFillerRegister
 * @brief				���ݱ�ӳ�������ע���
 * @author				barry
 * @date				2017/8/8
 */
class TableFillerRegister
{
private:
	TableFillerRegister();

public:
	/**
	 * @brief					��ȡ��������
	 */
	static TableFillerRegister&	GetRegister();

	/**
	 * @brief					��ʼ��
	 * @return					==0					�ɹ�
	 */
	int							Initialize();

	/**
	 * @brief					����Ϣ�ṹת��������ע��
	 * @param[in]				refRecordFiller		��Ϣ�ṹת����
	 * @return					==0					�ɹ�
								-1					�Ѿ�ע�����
								-2					����
	 */
	int							Register( InnerRecord& refRecordFiller );

	/**
	 * @brief					�����������Ϣ��Ϣ������ֻ�������ƷCode���µĶ�Ӧ�Ĵ����ݱ�ӳ�����ĵ�ַ
	 * @param[in]				nMessageID			��ϢID
	 * @param[in]				pMsgPtr				��Ҫ��ת������Ϣ��ַ
	 * @param[in]				nMsgLen				��Ϣ����
	 * @return					���ض�Ӧ�Ĵ���ת�������ַ
								NULL				δ�ҵ���Ӧ��ת������
	 */
	InnerRecord*				PrepareNewTableBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen );

private:
	static const unsigned int			s_nRegisterTableSize;		///< ע�����
	static std::vector<InnerRecord*>	s_vctRegisterTable;			///< ���ݱ�ת�������ע���
};



#endif









