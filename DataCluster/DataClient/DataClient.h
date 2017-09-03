#ifndef _DATACLIENT_DEFINE_H
#define _DATACLIENT_DEFINE_H


#include "../Interface.h"
#include "../QuoteClientApi.h"


/**
 * @class	MDataClient
 * @brief	api�ӿڵ�ʵ��
 */
class MDataClient : public QuoteClientApi
{
public:
	MDataClient();
	virtual ~MDataClient();

public:
	/**
	 * @brief			ֻ��ģ���ʼ�������������г�����
	 */
	int STDCALL			Init();

	/**
	 * @brief			�ͷ���Դ
	 */
	void STDCALL		Release();

	/**
	 * @brief			ע��ص�ʵ��
	 */
	void STDCALL		RegisterSpi(QuoteClientSpi * pspi);

	/**
	 * @brief			�������г���������
	 */
	int STDCALL			BeginWork();

	/**
	 * @brief			�رո��г���������
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
 * @brief		���ݲ�ѯ��
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











