#ifndef __NODE_WRAPPER__H__
#define __NODE_WRAPPER__H__


#include "../DataCenterEngine/DataCenterEngine.h"


/**
 * @class				ClusterCBAdaptor
 * @brief				行情数据族的回调转换类
 * @detail				忽略大结构行情的回调，但转发状态和日志回调
 * @author				barry
 * @date				2017-10-10
 */
class ClusterCBAdaptor : public I_QuotationCallBack
{
public:
	ClusterCBAdaptor();

	/**
	 * @brief							初始化数据采集器
	 * @detail							加载配置 + 设置消息回调 + 激活对上的通讯模块 + 启动断开重连线程
	 * @param[in]						pIDataHandle				行情回调接口
	 * @return							==0							初始化成功
										!=0							出错
	 */
	int									Initialize( I_DataHandle* pIDataHandle );

public:
	/**
	 * @brief							实行行情数据回调
	 * @note							更新行情内存块，并推送
	 * @param[in]						eMarketID					市场ID
	 * @param[in]						nDataID						消息ID
	 * @param[in]						pData						数据内容
	 * @param[in]						nDataLen					长度
	 * @param[in]						bPushFlag					推送标识
	 * @return							==0							成功
										!=0							错误
	 */
	virtual void						OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );

	/**
	 * @brief							内存数据查询接口
	 * @param[in]						nDataID						消息ID
	 * @param[in,out]					pData						数据内容(包含查询主键)
	 * @param[in]						nDataLen					长度
	 * @return							>0							成功,返回数据结构的大小
										==0							没查到结果
										!=0							错误
	 */
	virtual void						OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );

	/**
	 * @brief							日志函数
	 * @param[in]						nLogLevel					日志类型[0=信息、1=信息报告、2=警告日志、3=错误日志、4=详细日志]
	 * @param[in]						pszFormat					字符串格式化串
	 */
	virtual void						OnLog( unsigned char nLogLevel, const char* pszLogBuf );

protected:
	I_DataHandle*						m_pDataHandle;				///< 行情回调接口
};


/**
 * @class				EngineWrapper4DataNode
 * @brief				给DataNode.exe调用的接口导出功能封装类
 * @author				barry
 * @date				2017-10-10
 */
class EngineWrapper4DataNode : public I_DataHandle
{
private:
	EngineWrapper4DataNode();

public:
	static EngineWrapper4DataNode&		GetObj();

	/**
	 * @brief							是否被DataNode.exe调用
	 */
	bool								IsUsed();

public:
	/**
	 * @brief							初始化数据采集器
	 * @detail							加载配置 + 设置消息回调 + 激活对上的通讯模块 + 启动断开重连线程
	 * @param[in]						pIDataHandle				行情回调接口
	 * @return							==0							初始化成功
										!=0							出错
	 */
	int									Initialize( I_DataHandle* pIDataHandle );

	/**
	 * @brief							释放资源
	 */
	void								Release();

	/**
	 * @brief							重新socket连接请求行情快照和推送
	 * @note							是一个同步的函数，在行情初始化完成后才会返回
	 * @return							==0							成功
										!=0							出错
	 */
	int									RecoverQuotation();

	/**
	 * @brief							断开行情源socket连接
	 */
	void								Halt();

	/**
	 * @brief							取得采集模块的当前状态
 	 * @param[out]						pszStatusDesc				返回出状态描述串
	 * @param[in,out]					nStrLen						输入描述串缓存长度，输出描述串有效内容长度
	 * @return							返回模块当前状态值
	 */
	enum E_SS_Status					GetCollectorStatus( char* pszStatusDesc, unsigned int& nStrLen );

public:///< I_DataHandle接口实现: 用于给数据采集模块提供行情数据的回调方法
	/**
 	 * @brief							初始化性质的行情数据回调
	 * @note							只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]						nDataID						消息ID
	 * @param[in]						pData						数据内容
	 * @param[in]						nDataLen					长度
	 * @param[in]						bLastFlag					是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return							==0							成功
										!=0							错误
	 */
	virtual int							OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag );

	/**
	 * @brief							实行行情数据回调
	 * @note							更新行情内存块，并推送
	 * @param[in]						nDataID						消息ID
	 * @param[in]						pData						数据内容
	 * @param[in]						nDataLen					长度
	 * @param[in]						bPushFlag					推送标识
	 * @return							==0							成功
										!=0							错误
	 */
	virtual int							OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag );

	/**
	 * @brief							内存数据查询接口
	 * @param[in]						nDataID						消息ID
	 * @param[in,out]					pData						数据内容(包含查询主键)
	 * @param[in]						nDataLen					长度
	 * @return							>0							成功,返回数据结构的大小
										==0							没查到结果
										!=0							错误
	 * @note							如果pData的缓存为“全零”缓存，则返回表内的所有数据
	 */
	virtual int							OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen );

	/**
	 * @brief							日志函数
	 * @param[in]						nLogLevel					日志类型[0=信息、1=警告日志、2=错误日志、3=详细日志]
	 * @param[in]						pszFormat					字符串格式化串
	 */
	virtual void						OnLog( unsigned char nLogLevel, const char* pszFormat, ... );

protected:
	ClusterCBAdaptor					m_oClusterCBAdaptor;		///< 行情回调适配
	I_DataHandle*						m_pDataHandle;				///< 行情回调接口

};

#endif











