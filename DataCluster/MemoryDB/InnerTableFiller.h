#ifndef __INNER_TABLE_FILLER_H__
#define	__INNER_TABLE_FILLER_H__


#include <vector>


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
	/**
	 * @brief					��ȡ����¼��ID��
	 */
	virtual unsigned int		GetInnerTableID() = 0;

	/**
	 * @brief					��ȡ��д�Ĵ���¼�ṹ��ַ
	 */
	virtual char*				GetInnerRecordPtr() = 0;

	/**
	 * @brief					��ȡ����¼�ṹ����
	 */
	virtual unsigned int		GetInnerRecordLength() = 0;

	/**
	 * @brief					��Э����Ϣ�ṹ���������õ�����¼
								ÿ��������ϵ���Message��Ҫʵ��һ��������
	 */
	virtual void				FillMessage2InnerRecord() = 0;

private:
	char*						m_pMsgPtr;				///< ��������ϵת���Э����Ϣ��ַ
	unsigned int				m_nMsgLen;				///< ��������ϵ�����Э����Ϣ����
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









