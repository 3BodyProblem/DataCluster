#ifndef __INNER_TABLE_FILLER_H__
#define	__INNER_TABLE_FILLER_H__


#include <vector>
#include "../Protocal/DataCluster_Protocal.h"


#pragma pack(1)
typedef struct													///< ���ƴ������Ϣ
{
	char						szCode[QUO_MAX_CODE];			///< ��Ʒ����
	tagQUO_MarketInfo			objData;
}	T_Inner_MarketInfo;
#pragma pack()


typedef union {
	T_Inner_MarketInfo		MarketData_1;
	tagQUO_ReferenceData	ReferenceData_2;
	tagQUO_SnapData			SnapData_3;
}	T_BIGTABLE_RECORD;											///< ������ݽṹ������


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
	InnerRecord( unsigned int nMsgID, unsigned int nMsgLen, unsigned int nBigTableID, bool bIsSingleton = false )
		 : m_nMessageID( nMsgID ), m_nMessageLength( nMsgLen ), m_nBigTableID( nBigTableID ), m_bIsSingleton( bIsSingleton )
	{}

	/**
	 * @brief					�Ƿ�Ϊ���ݱ��е�Ψһ��¼������Ϊ�գ�����ֵ��
	 */
	bool						IsSingleton()	{
		return m_bIsSingleton;
	}

	/**
	 * @brief					��ȡ��Ϣ��¼��ID��
	 */
	unsigned int				GetMessageID()	{
		return m_nMessageID;
	}

	/**
	 * @brief					��ȡ��Ϣ��¼�ṹ����
	 */
	unsigned int				GetMessageLength()	{
		return m_nMessageLength;
	}

	/**
	 * @brief					��ȡ���ID
	 */
	unsigned int				GetBigTableID()	{
		return m_nBigTableID;
	}

	/**
	 * @brief					��ȡ�������ݽṹ���
	 */
	unsigned int				GetBigTableWidth()	{
		switch( GetBigTableID() % 100 )
		{
		case 1:
			return sizeof( m_objUnionData.MarketData_1 );
		case 2:
			return sizeof( m_objUnionData.ReferenceData_2 );
		case 3:
			return sizeof( m_objUnionData.SnapData_3 );
		}

		return 0;
	}

	/**
	 * @brief					��ȡ��д�Ĵ���¼�ṹ��ַ
	 */
	char*						GetBigTableRecordPtr()	{
		switch( GetBigTableID() % 100 )
		{
		case 1:
			return (char*)&(m_objUnionData.MarketData_1);
		case 2:
			return (char*)&(m_objUnionData.ReferenceData_2);
		case 3:
			return (char*)&(m_objUnionData.SnapData_3);
		}

		return NULL;
	}

public:
	/**
	 * @brief					��Э����Ϣ�ṹ���������õ�����¼
								ÿ��������ϵ���Message��Ҫʵ��һ��������
	 * @param[in]				pMessagePtr				��Ϣָ��
	 * @param[in]				bNewRecord				�Ƿ�Ϊ�¼�¼
	 */
	virtual void				FillMessage2BigTableRecord( char* pMessagePtr, bool bNewRecord = false ) = 0;

protected:
	unsigned int				m_nMessageID;			///< ��ϢID
	unsigned int				m_nMessageLength;		///< ��Ϣ����
	unsigned int				m_nBigTableID;			///< ���ݴ���ID
	bool						m_bIsSingleton;			///< ���ݱ��е�Ψһ��¼������Ϊ�գ�����ֵ��
protected:
	T_BIGTABLE_RECORD			m_objUnionData;			///< ������ݽṹ������
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
	TableFillerRegister()
	{
		s_vctRegisterTable.resize( s_nRegisterTableSize );		///< Ԥ��2048��messageidӳ������ַ
	}

public:
	/**
	 * @brief					��ȡ��������
	 */
	static TableFillerRegister&	GetRegister()
	{
		static TableFillerRegister		obj;
		return obj;
	}

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
	InnerRecord*				PrepareRecordBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen );

private:
	static const unsigned int			s_nRegisterTableSize;		///< ע�����
	static std::vector<InnerRecord*>	s_vctRegisterTable;			///< ���ݱ�ת�������ע���
};



#endif









