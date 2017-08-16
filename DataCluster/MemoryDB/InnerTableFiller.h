#ifndef __INNER_TABLE_FILLER_H__
#define	__INNER_TABLE_FILLER_H__


#include <vector>
#include "../Protocal/DataCluster_Protocal.h"


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
	InnerRecord( unsigned int nMsgID, unsigned int nMsgLen, unsigned int nBigTableID );

	/**
	 * @brief					获取消息记录的ID号
	 */
	unsigned int				GetMessageID();

	/**
	 * @brief					获取消息记录结构长度
	 */
	unsigned int				GetMessageLength();

	/**
	 * @brief					获取大表ID
	 */
	unsigned int				GetBigTableID();

	/**
	 * @brief					获取大表的数据结构宽度
	 */
	unsigned int				GetBigTableWidth();

	/**
	 * @brief					获取可写的大表记录结构地址
	 */
	char*						GetBigTableRecordPtr();

public:
	/**
	 * @brief					将协议消息结构的数据设置到大表记录
								每个传输体系里的Message都要实现一个本方法
	 */
	virtual void				FillMessage2BigTableRecord( char* pMessagePtr ) = 0;

protected:
	unsigned int				m_nMessageID;			///< 消息ID
	unsigned int				m_nMessageLength;		///< 消息长度
	unsigned int				m_nBigTableID;			///< 数据大表的ID
protected:
	union BigTableRecord {
		tagQuoMarketInfo		MarketData_1;
		tagQuoCategory			CategoryData_2;
		tagQuoReferenceData		ReferenceData_3;
		tagQuoSnapData			SnapData_4;
	}							m_objUnionData;			///< 大表数据结构联合体
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
	 * @brief					初始化
	 * @return					==0					成功
	 */
	int							Initialize();

	/**
	 * @brief					将消息结构转换器进行注册
	 * @param[in]				refRecordFiller		消息结构转换器
	 * @return					==0					成功
								-1					已经注册过了
								-2					出错
	 */
	int							Register( InnerRecord& refRecordFiller );

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









