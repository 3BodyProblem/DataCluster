#ifndef __INNER_TABLE_FILLER_H__
#define	__INNER_TABLE_FILLER_H__


#include <vector>
#include "../Protocal/DataCluster_Protocal.h"


#pragma pack(1)
typedef struct													///< 名称代码表信息
{
	char						szCode[QUO_MAX_CODE];			///< 商品代码
	tagQUO_MarketInfo			objData;
}	T_Inner_MarketInfo;
#pragma pack()


typedef union {
	T_Inner_MarketInfo		MarketData_1;
	tagQUO_ReferenceData	ReferenceData_2;
	tagQUO_SnapData			SnapData_3;
}	T_BIGTABLE_RECORD;											///< 大表数据结构联合体


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
	InnerRecord( unsigned int nMsgID, unsigned int nMsgLen, unsigned int nBigTableID, bool bIsSingleton = false )
		 : m_nMessageID( nMsgID ), m_nMessageLength( nMsgLen ), m_nBigTableID( nBigTableID ), m_bIsSingleton( bIsSingleton )
	{}

	/**
	 * @brief					是否为数据表中的唯一记录（主键为空，不赋值）
	 */
	bool						IsSingleton()	{
		return m_bIsSingleton;
	}

	/**
	 * @brief					获取消息记录的ID号
	 */
	unsigned int				GetMessageID()	{
		return m_nMessageID;
	}

	/**
	 * @brief					获取消息记录结构长度
	 */
	unsigned int				GetMessageLength()	{
		return m_nMessageLength;
	}

	/**
	 * @brief					获取大表ID
	 */
	unsigned int				GetBigTableID()	{
		return m_nBigTableID;
	}

	/**
	 * @brief					获取大表的数据结构宽度
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
	 * @brief					获取可写的大表记录结构地址
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
	 * @brief					将协议消息结构的数据设置到大表记录
								每个传输体系里的Message都要实现一个本方法
	 * @param[in]				pMessagePtr				消息指针
	 * @param[in]				bNewRecord				是否为新记录
	 */
	virtual void				FillMessage2BigTableRecord( char* pMessagePtr, bool bNewRecord = false ) = 0;

protected:
	unsigned int				m_nMessageID;			///< 消息ID
	unsigned int				m_nMessageLength;		///< 消息长度
	unsigned int				m_nBigTableID;			///< 数据大表的ID
	bool						m_bIsSingleton;			///< 数据表中的唯一记录（主键为空，不赋值）
protected:
	T_BIGTABLE_RECORD			m_objUnionData;			///< 大表数据结构联合体
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
	TableFillerRegister()
	{
		s_vctRegisterTable.resize( s_nRegisterTableSize );		///< 预留2048个messageid映射器地址
	}

public:
	/**
	 * @brief					获取单键引用
	 */
	static TableFillerRegister&	GetRegister()
	{
		static TableFillerRegister		obj;
		return obj;
	}

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
	InnerRecord*				PrepareRecordBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen );

private:
	static const unsigned int			s_nRegisterTableSize;		///< 注册表长度
	static std::vector<InnerRecord*>	s_vctRegisterTable;			///< 数据表转换填充器注册表
};



#endif









