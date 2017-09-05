#ifndef _DATACLIENT_DEFINE_H
#define _DATACLIENT_DEFINE_H


#include "../Interface.h"
#include "../QuoteClientApi.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"


/**
 * @brief			�������͹���
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
	static void* STDCALL	DataThreadFunc(void *pParam);	//���������ɷ�
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
	void			Detach();	//����MsgHead,Last�����
	//������(1)����ninbytes, (2)����ushort��MsgLen�������͵ȴ���,
	//�������ͣ��Ƿ�Ҫ����һ�������У��������65535�ı�����Χ��Ҫ�����һ��,LastType=0xFFFF?


	void			PutSingleMsg(unsigned int ntype, char* pStruct, int nStructSize);	//������Ϣ�����ɺϲ�
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

private:
	CriticalObject		m_oLock;						///< ��
	char*				m_pQueryBuffer;
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











