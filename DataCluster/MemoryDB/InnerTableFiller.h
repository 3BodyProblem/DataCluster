#ifndef __INNER_TABLE_FILLER_H__
#define	__INNER_TABLE_FILLER_H__


#include <vector>


/**
 * @class				InnerRecord
 * @brief				数据大表和行情消息之间的结构转换数据填充类
 * @author				barry
 * @date				2017/8/8
 */
class InnerRecord
{
friend class TableFillerRegister;
public:
	/**
	 * @brief					获取大表记录的ID号
	 */
	virtual unsigned int		GetInnerTableID() = 0;

	/**
	 * @brief					获取可写的大表记录结构地址
	 */
	virtual char*				GetInnerRecordPtr() = 0;

	/**
	 * @brief					获取大表记录结构长度
	 */
	virtual unsigned int		GetInnerRecordLength() = 0;

	/**
	 * @brief					将协议消息结构的数据设置到大表记录
								每个传输体系里的Message都要实现一个本方法
	 */
	virtual void				FillMessage2InnerRecord() = 0;

private:
	char*						m_pMsgPtr;				///< 从行情体系转入的协议消息地址
	unsigned int				m_nMsgLen;				///< 从行情体系传入的协议消息长度
};


///< -----------------------------------------------------------------------------


/**
 * @class				TableFillerRegister
 * @brief				数据表映射填充器注册机
 * @author				barry
 * @date				2017/8/8
 */
class TableFillerRegister
{
private:
	TableFillerRegister();

public:
	/**
	 * @brief					获取单键引用
	 */
	static TableFillerRegister&	GetRegister();

	/**
	 * @brief					根据行情的消息信息，生成只填充了商品Code的新的对应的大数据表映射对象的地址
	 * @param[in]				nMessageID			消息ID
	 * @param[in]				pMsgPtr				需要被转换的消息地址
	 * @param[in]				nMsgLen				消息长度
	 * @return					返回对应的大表的转换对象地址
								NULL				未找到对应的转换对象
	 */
	InnerRecord*				PrepareNewTableBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen );

private:
	static const unsigned int			s_nRegisterTableSize;		///< 注册表长度
	static std::vector<InnerRecord*>	s_vctRegisterTable;			///< 数据表转换填充器注册表
};



#endif









