#ifndef _DATACLIENT_DEFINE_H
#define _DATACLIENT_DEFINE_H


#include <map>
#include "../Interface.h"
#include "../QuoteClientApi.h"
#include "../Infrastructure/Lock.h"
#include "../Infrastructure/Thread.h"
#include "../Infrastructure/LoopBuffer.h"


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
	SimpleThread			m_threadClient;
	MLoopBufferMt<char>		m_PushBuffer;
	CriticalObject			m_oSection;
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
};


typedef std::map<int,std::map<int,XDFAPI_MarketKindInfo>>	T_KIND_MAP;		///< [�г�ID,[���,�����Ϣ]]
extern MDataIO				g_oDataIO;


/**
 * @class	MDataClient
 * @brief	QuoClientApi.DLL�Ľӿڵ�ʵ��
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

public:
	static int			GetRate( int nMarketID, unsigned int nKind );

	/**
	 * @brief			�Ƿ�Ϊ�����ϰ�quoteclientapi.dll��ģʽ
	 * @return			true							Ϊ���ݰ�
	 */
	bool				InQuoteClientApiMode();

private:
	CriticalObject		m_oLock;						///< ��
	char*				m_pQueryBuffer;
	static T_KIND_MAP	m_mapMarketKind;				///< ���г��������Ϣ
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
	virtual void			OnQuotation( QUO_MARKET_ID eMarketID, unsigned int nMessageID, char* pDataPtr, unsigned int nDataLen );
	virtual void			OnStatus( QUO_MARKET_ID eMarketID, QUO_MARKET_STATUS eMarketStatus );
	virtual void			OnLog( unsigned char nLogLevel, const char* pszLogBuf );
};



#endif











