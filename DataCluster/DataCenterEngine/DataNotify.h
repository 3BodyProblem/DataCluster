#ifndef __DATA_NOTIFY_H__
#define __DATA_NOTIFY_H__


#pragma warning(disable:4018)
#include <set>
#include <string>
#include "../Interface.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


#pragma pack(1)
typedef struct
{
	unsigned short		MsgID;		///< 消息ID
	unsigned short		MsgLen;		///< 消息长度
} tagMsgHead;
#pragma pack()


/**
 * @class							PackagesLoopBuffer
 * @brief							数据包队列缓存
 * @detail							MessageID1 + data block1 + data block2 + data block3 + ...
 * @author							barry
 */
class PackagesLoopBuffer
{
public:
	PackagesLoopBuffer();
	~PackagesLoopBuffer();

public:
	/**
	 * @brief						初始化缓存对象
	 * @param[in]					nMaxBufSize				将分配的缓存大小
	 * @return						==0						成功
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						释放缓存空间
	 */
	void							Release();
 
public:
	/**
	 * @brief						存储数据
	 * @param[in]					nDataID					数据ID
	 * @param[in]					pData					数据指针
	 * @param[in]					nDataSize				数据长度
	 * @return						==0						成功
	 * @note						当nDataID不等于前一个包的nDataID时，将新启用一个Package封装
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize );

	/**
	 * @brief						获取一个数据包
	 * @param[out]					pBuff					输出数据缓存地址
	 * @param[in]					nBuffSize				数据缓存长度
	 * @param[out]					nMsgID					数据消息ID
	 * @return						>0						数据长度
									==0						无数据
									<0						出错
	 */
	int								GetBlock( char* pBuff, unsigned int nBuffSize, unsigned int& nMsgID );

	/**
	 * @brief						是否为空
	 * @return						true					为空
	 */
	bool							IsEmpty();

protected:
	CriticalObject					m_oLock;				///< 锁
	char*							m_pPkgBuffer;			///< 数据包缓存地址
	unsigned int					m_nMaxPkgBufSize;		///< 数据包缓存大小
	unsigned long					m_nFirstRecord;			///< 数据起始位置
	unsigned long					m_nLastRecord;			///< 数据结束位置
};


/**
 * @class						QuotationNotify
 * @brief						行情流实时推送缓存
 * @author						barry
 */
class QuotationNotify : public SimpleTask
{
public:
	/**
	 * @brief					构造函数
	 */
	QuotationNotify();
	~QuotationNotify();

	/**
	 * @brief					初始化行情流推送缓存
	 * @param[in]				pIQuotation				用于行情回调通知的接口
	 * @param[in]				nNewBuffSize			要分配的缓存大小
	 * @return					!= 0					失败
	 */
	int							Initialize( I_QuotationCallBack* pIQuotation, unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					释放各资源
	 */
	void						Release();

protected:
	/**
	 * @brief					任务函数(内循环)
	 * @return					==0						成功
								!=0						失败
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					将行情数据压缩后进行缓存
	 * @param[in]				nMsgID					Message ID
	 * @param[in]				pData					消息数据地址
	 * @param[in]				nLen					消息长度
	 * @return					> 0						成功，返回历次调用累积的序列化的长度
								<= 0					失败
	 */
	int							PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen );

	/**
	 * @brief					从缓存取出消息数据回调给外部调用者
	 */
	void						NotifyMessage();

protected:
	I_QuotationCallBack*		m_pQuotationCallBack;	///< 行情回调
	WaitEvent					m_oWaitEvent;			///< 条件等待
	PackagesLoopBuffer			m_oDataBuffer;			///< 行情数据缓存队列
};





#endif





