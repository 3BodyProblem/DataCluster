#ifndef _DATACLIENT_DEFINE_H
#define _DATACLIENT_DEFINE_H


#include "../Interface.h"
#include "../QuoteClientApi.h"


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
	

protected:

private:

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











