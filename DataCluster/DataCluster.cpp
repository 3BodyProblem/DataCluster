#include "targetver.h"
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCluster.h"
#include "UnitTest/UnitTest.h"
#include "Infrastructure/Lock.h"
#include "DataCenterEngine/DataCenterEngine.h"
#include "QuoteClientApi.h"


/**
 * @class			RecordsFilter
 * @brief			全量数据记录集部分提取类
 */
class RecordsFilter
{
public:
	static const unsigned int	MAX_BUF_SIZE = 1024*1024*50;
	RecordsFilter()
		: m_pBuff( NULL )
	{}

	~RecordsFilter()
	{
		if( NULL != m_pBuff ) {
			delete [] m_pBuff;
			m_pBuff = NULL;
		}
	}

	/**
	 * @brief				初始化提取对象
	 * @return				==0					初始化成功
							!=0					出错
	 */
	int	Initialize() {
		if( NULL == m_pBuff ) {
			m_pBuff = new char[MAX_BUF_SIZE];
		}

		if( NULL != m_pBuff ) {
			return 0;
		}

		return -1;
	}

	/**
	 * @brief				将查询出的全部数据表内容进行部分提取到输出缓存
	 * @param[in]			nMessageID			消息ID
	 * @param[in]			nMessageSize		消息长度
	 * @param[in]			uiOffset			从第n条记录开始提取直到最后一条
	 * @param[out]			lpOut				提取结果输出缓存
	 * @param[in]			uiSize				缓存长度
	 * @return				返回结果的长度
	 */
	int	ExtraRecords( unsigned int nMessageID, unsigned int nMessageSize, unsigned int uiOffset, char* lpOut, unsigned int uiSize )
	{
		unsigned int			nCopySize = 0;
		unsigned int			nItemNumber = 0;
		CriticalLock			lock( m_oLock );

		if( DataIOEngine::GetEngineObj().OnQuery( nMessageID, m_pBuff, MAX_BUF_SIZE ) <= 0 )
		{
			return -1;
		}

		for( unsigned int nOffset = 0; nOffset < MAX_BUF_SIZE && nOffset < uiSize; nOffset += nMessageSize, nItemNumber++ )
		{
			if( nItemNumber >= uiOffset )
			{
				::memcpy( lpOut + (nItemNumber-uiOffset)*nMessageSize, m_pBuff + nOffset, nMessageSize );
				nCopySize += nMessageSize;
			}
		}

		return 0;
	}

private:
	char*						m_pBuff;						///< 数据缓存
	CriticalObject				m_oLock;						///< 锁
};


static RecordsFilter		objRecordsFilter;


extern "C"
{
	__declspec(dllexport) int __stdcall		GetVersionNo()
	{
		unsigned int	nMajor = 1;
		unsigned int	nRelease = 1;
		unsigned int	nBuild = 1;

		return 1000000 * nMajor + 100000 * nRelease + 1000 * nBuild;
	}

	__declspec(dllexport) int __stdcall		StartWork( I_QuotationCallBack* pIDataHandle )
	{
		if( objRecordsFilter.Initialize() < 0 )
		{
			::printf( "%s\n", "DataIOEngine::Initialize() : cannot initialize RecordsFilter class." );
			return -1;
		}

		return DataIOEngine::GetEngineObj().Initialize( pIDataHandle );
	}

	__declspec(dllexport) void __stdcall	EndWork()
	{
		DataIOEngine::GetEngineObj().Release();
	}

	__declspec(dllexport) int  __stdcall	GetMarketID( QUO_MARKET_ID* lpOut, unsigned int uiSize )
	{
		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetMarketInfo( QUO_MARKET_ID eMarketID, tagQUO_MarketInfo* lpOut )
	{
		T_Inner_MarketInfo		tagMarketinfo = { 0 };

		if( QUO_MARKET_UNKNOW == eMarketID || NULL == lpOut )
		{
			return -1;
		}

		if( DataIOEngine::GetEngineObj().OnQuery( eMarketID*100+1, (char*)&tagMarketinfo, sizeof(T_Inner_MarketInfo) ) <= 0 )
		{
			return -2;
		}

		::memcpy( lpOut, &(tagMarketinfo.objData), sizeof(tagQUO_MarketInfo) );

		return 0;
	}

	__declspec(dllexport) int  __stdcall	GetAllReferenceData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_ReferenceData* lpOut, unsigned int uiSize )
	{
		return objRecordsFilter.ExtraRecords( eMarketID*100+2, sizeof(tagQUO_ReferenceData), uiOffset, (char*)lpOut, uiSize );
	}

	__declspec(dllexport) int  __stdcall	GetReferenceData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_ReferenceData* lpOut )
	{
		::memset( lpOut, 0, sizeof(tagQUO_ReferenceData) );
		::strcpy( (char*)lpOut, szCode );

		return DataIOEngine::GetEngineObj().OnQuery( eMarketID*100+2, (char*)lpOut, sizeof(tagQUO_ReferenceData) );
	}

	__declspec(dllexport) int  __stdcall	GetAllSnapData( QUO_MARKET_ID eMarketID, unsigned int uiOffset, tagQUO_SnapData* lpOut, unsigned int uiSize )
	{
		return objRecordsFilter.ExtraRecords( eMarketID*100+3, sizeof(tagQUO_SnapData), uiOffset, (char*)lpOut, uiSize );
	}

	__declspec(dllexport) int  __stdcall	GetSnapData( QUO_MARKET_ID eMarketID, const char* szCode, tagQUO_SnapData* lpOut )
	{
		::memset( lpOut, 0, sizeof(tagQUO_SnapData) );
		::strcpy( (char*)lpOut, szCode );

		return DataIOEngine::GetEngineObj().OnQuery( eMarketID*100+3, (char*)lpOut, sizeof(tagQUO_SnapData) );
	}

	__declspec(dllexport) void __stdcall	ExecuteUnitTest()
	{
		::printf( "\n\n---------------------- [Begin] -------------------------\n" );
		//ExecuteTestCase();
		::printf( "----------------------  [End]  -------------------------\n\n\n" );
	}

	__declspec(dllexport) const char*		GetDllVersion( int &nMajorVersion, int &nMinorVersion )
	{
		static int		s_nMajorVer = 1;
		static int		s_nMinorVer = 1000;
		static char		pszBuf[255] = { 0 };

		nMajorVersion = s_nMajorVer;
		nMinorVersion = s_nMinorVer;
		_snprintf( pszBuf, 254, "V%.02d B%.02d", s_nMajorVer, s_nMinorVer );

		return pszBuf;
	}

///< ------------------------ 以下为兼容老版的接口 ----------------------------------------------------
	MPrimeClient				Global_PrimeClient;
	bool						Global_bInit = false;
	MDataClient					Global_Client;
	QuotationAdaptor			Global_CBAdaptor;
	QuoteClientSpi*				Global_pSpi = NULL;

	__declspec(dllexport) QuoteClientApi*	CreateQuoteApi( const char* pszDebugPath )
	{
		if (!Global_bInit)
		{
			Global_bInit = true;

			if( 0 != StartWork( &Global_CBAdaptor ) )
			{
				return NULL;
			}
		}

		return &Global_Client;
	}

	__declspec(dllexport) QuotePrimeApi* CreatePrimeApi()
	{
		return &Global_PrimeClient;
	}

	__declspec(dllexport) int	GetSettingInfo( tagQuoteSettingInfo* pArrMarket, int nCount )
	{
		if( 0 != Configuration::GetConfigObj().Load() )
		{
			return -1;
		}

		if( NULL == pArrMarket )
		{
			return -2;
		}

		DllPathTable&		refDllTable = Configuration::GetConfigObj().GetDCPathTable();
		int					nDllNum = min( refDllTable.GetCount(), nCount );

		for( int n = 0; n < nDllNum; n++ )
		{
			::strcpy( pArrMarket[n].cAddress, refDllTable.GetPathByPos( n ).c_str() );

/*	char			szDll[255];
	int				nMarketID;
	int				nSleep;
	char			cMarketChn[64];
	char			cAddress[128];*/
		}

/*
		tagKeyFileInfo oInfo;
		for (int i=0; i<ncopycount; i++)
		{
		Global_Option.GetKeyFileInfo(i, oInfo);
		pArrMarket[i].cMarketID = oInfo.nMarketID;
		strcpy(pArrMarket[i].cMarketChn, oInfo.cMarketChn);
		strcpy(pArrMarket[i].cAddress, oInfo.cAddress);
		pArrMarket[i].nStatus = Global_DllMgr.GetMarketStat(oInfo.nMarketID);
		}*/

		return nDllNum;
	}
}




