#ifndef _DATACLIENT_DEFINE_H
#define _DATACLIENT_DEFINE_H


#include "../Interface.h"
#include "../QuoteClientApi.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


/**
 * @brief			数据推送管理
 */
class MDataIO
{
public:
	MDataIO();
	virtual ~MDataIO();

public:
	int					Instance();
	void				Release();

public:
	int					PutData(XDFAPI_PkgHead* pHead,const char* pszInBuf, int	nInBytes);

protected:
	static void* STDCALL	DataThreadFunc(void *pParam);	//处理数据派发
	void				inner_CheckData();

protected:
	char*					m_pPkgBuf;
	WaitEvent				m_oWEvent;
	MThread					m_threadClient;
	MLoopBufferMt<char>		m_PushBuffer;
	CriticalObject			m_oSection;
	MCounter				m_oCounter;
};


class MStreamWrite
{
public:
	MStreamWrite();
	virtual ~MStreamWrite();
	MStreamWrite(char* pszinbuf, int ninbytes);

public:
	void			Attach(char* pszinbuf, int ninbytes);
	void			Detach();	//处理MsgHead,Last缓冲等
	//负责处理(1)超过ninbytes, (2)超过ushort的MsgLen复合类型等处理,
	//复合类型，是否要缓冲一个，还有，如果超出65535的表述范围，要另外分一个,LastType=0xFFFF?


	void			PutSingleMsg(unsigned int ntype, char* pStruct, int nStructSize);	//独立消息，不可合并
	void			PutMsg(unsigned int ntype, char* pStruct, int nStructSize);

	bool			IsError();
	int				GetOffset();
	int				GetCount();

protected:

protected:
	char*			m_pInBuf;
	int				m_nInBytes;
	
	XDFAPI_UniMsgHead*	m_pUniHead;

	unsigned int	m_nLastType;
	int				m_nMsgCount;
	int				m_nOffset;
	bool			m_bError;

private:

};




/**
 * @class	MDataClient
 * @brief	api接口的实现
 */
class MDataClient : public QuoteClientApi
{
public:
	MDataClient();
	virtual ~MDataClient();

public:
	/**
	 * @brief			只作模块初始化，不启动各市场传输
	 */
	int STDCALL			Init();

	/**
	 * @brief			释放资源
	 */
	void STDCALL		Release();

	/**
	 * @brief			注册回调实例
	 */
	void STDCALL		RegisterSpi(QuoteClientSpi * pspi);

	/**
	 * @brief			启动各市场传输驱动
	 */
	int STDCALL			BeginWork();

	/**
	 * @brief			关闭各市场传输驱动
	 */
	void STDCALL		EndWork();

	int	 STDCALL		GetMarketInfo(unsigned char cMarket, char* pszInBuf, int nInBytes);

	int	STDCALL			GetCodeTable(unsigned char cMarket, char* pszInBuf, int nInBytes, int& nCount);
	
	int STDCALL			GetLastMarketDataAll(unsigned char cMarket, char* pszInBuf, int nInBytes);
	
	int STDCALL			GetMarketStatus(unsigned char cMarket,int& nStatus, unsigned int& ulTime, __int64 * pI64Send, __int64 * pI64Recv);

private:
	CriticalObject		m_oLock;						///< 锁
	char*				m_pQueryBuffer;
};


/**
 * @class		MPrimeClient
 * @brief		数据查询类
 */
class MPrimeClient : public QuotePrimeApi
{
public:
	MPrimeClient();
	virtual ~MPrimeClient();

public:
	int		STDCALL		ReqFuncData(int FuncNo, void* wParam, void* lParam);
protected:
private:
};



class QuotationAdaptor : public I_QuotationCallBack
{
public:
	virtual void			OnQuotation( unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnStatusChg( unsigned int nMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf );
};



#endif











