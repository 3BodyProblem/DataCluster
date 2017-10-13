#ifndef __CLIENT_WRAPPER__H__
#define __CLIENT_WRAPPER__H__


#include "../DataCenterEngine/DataCenterEngine.h"


/**
 * @class				EngineWrapper4DataClient
 * @brief				给终端调的行情查询和实时推送接口
 * @author				barry
 * @date				2017-10-10
 */
class EngineWrapper4DataClient : public I_DataHandle
{
private:
	EngineWrapper4DataClient();

public:
	static EngineWrapper4DataClient&	GetObj();

	/**
	 * @brief							是否以Client终端模式被调用
	 */
	bool								IsUsed();

public:
	/**
	 * @brief							初始化数据采集器
	 * @detail							加载配置 + 设置消息回调 + 激活对上的通讯模块 + 启动断开重连线程
	 * @param[in]						pIQuotation					用于行情回调通知的接口
	 * @return							==0							初始化成功
										!=0							出错
	 */
	int									Initialize( I_QuotationCallBack* pIQuotation );

	/**
	 * @brief							释放资源
	 */
	void								Release();

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

public:
	/**
	 * @brief							获取数据库对象
	 */
	BigTableDatabase&					GetDatabaseObj();

	/**
	 * @brief							内存数据查询接口
	 * @param[in]						nDataID				消息ID
	 * @param[in,out]					pData				数据内容(包含查询主键)
	 * @param[in]						nDataLen			长度
	 * @return							>0					成功,返回数据结构的大小
										==0					没查到结果
										!=0					错误
	 */
	void								OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );

protected:
	I_QuotationCallBack*				m_pQuotationCallBack;		///< 大数据表的行情回调
	BigTableDatabase					m_oDB4ClientMode;			///< 内存数据插件管理
	QuotationNotify						m_oQuoNotify;				///< 行情数据通知回调
};

#endif











