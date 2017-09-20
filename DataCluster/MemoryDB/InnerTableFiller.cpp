#include <map>
#include <math.h>
#include <algorithm>
#include "InnerTableFiller.h"
#include "../DataCenterEngine/DataCenterEngine.h"


///< 大连商品期货
static	std::map<short,short>			s_mapKindID2PriceRate4DL;
static	std::map<std::string,short>		s_mapCode2Kind4DL;
static	unsigned int					s_nTime4DL = 0;
static	unsigned int					s_nDate4DL = 0;

double	CalPriceRate( const char* pszCode, unsigned int& nKindID, std::map<std::string,short>& mapCode2Kind, std::map<short,short>& mapKind2Rate )
{
	double								dRate = 1.;

	if( nKindID == 0xFFffFFff )
	{
		std::map<std::string,short>::iterator it = mapCode2Kind.find( pszCode );

		if( it != mapCode2Kind.end() )
		{
			nKindID = it->second;
		}
		else
		{
			nKindID = 0xFFffFFff;
			return dRate;
		}
	}

	std::map<short,short>::iterator		itKind = mapKind2Rate.find( nKindID );

	if( itKind != mapKind2Rate.end() )
	{
		dRate = ::pow( (double)10, (int)mapKind2Rate[nKindID] );
	}

	return dRate;
}


struct MappingDLFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingDLFuture_MkInfo2QuoMarketInfo() : InnerRecord( 100, sizeof(tagDLFutureMarketInfo_LF100), QUO_MARKET_DCE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureMarketInfo_LF100*	pMkInfo = (tagDLFutureMarketInfo_LF100*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4DL = pMkInfo->MarketDate;
		}
	}
};

struct MappingDLFuture_Kind2QuoCategory : public InnerRecord { MappingDLFuture_Kind2QuoCategory() : InnerRecord( 101, sizeof(tagDLFutureKindDetail_LF101), QUO_MARKET_DCE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureKindDetail_LF101*	pKind = (tagDLFutureKindDetail_LF101*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4DL[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingDLFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingDLFuture_MkStatus2QuoMarketInfo() : InnerRecord( 102, sizeof(tagDLFutureMarketStatus_HF102), QUO_MARKET_DCE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureMarketStatus_HF102*	pMkStatus = (tagDLFutureMarketStatus_HF102*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4DL = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingDLFuture_Reference2QuoReference : public InnerRecord { MappingDLFuture_Reference2QuoReference() : InnerRecord( 103, sizeof(tagDLFutureReferenceData_LF103), QUO_MARKET_DCE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureReferenceData_LF103*	pRefData = (tagDLFutureReferenceData_LF103*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4DL[std::string(pBigTable->szCode)] = pBigTable->uiKindID;

			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DL, s_mapKindID2PriceRate4DL );
			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingDLFuture_SnapLF2QuoSnapData : public InnerRecord { MappingDLFuture_SnapLF2QuoSnapData() : InnerRecord( 104, sizeof(tagDLFutureSnapData_LF104), QUO_MARKET_DCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureSnapData_LF104*		pSnapData = (tagDLFutureSnapData_LF104*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DL, s_mapKindID2PriceRate4DL );

			pBigTable->uiTime = s_nTime4DL;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingDLFuture_SnapHF2QuoSnapData : public InnerRecord { MappingDLFuture_SnapHF2QuoSnapData() : InnerRecord( 105, sizeof(tagDLFutureSnapData_HF105), QUO_MARKET_DCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureSnapData_HF105*		pSnapData = (tagDLFutureSnapData_HF105*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DL, s_mapKindID2PriceRate4DL );

			pBigTable->uiTime = s_nTime4DL;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingDLFuture_BuySell2QuoSnapData : public InnerRecord { MappingDLFuture_BuySell2QuoSnapData() : InnerRecord( 106, sizeof(tagDLFutureSnapBuySell_HF106), QUO_MARKET_DCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureSnapBuySell_HF106*	pSnapData = (tagDLFutureSnapBuySell_HF106*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4DL;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DL, s_mapKindID2PriceRate4DL );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 上海商品期货
static	std::map<short,short>			s_mapKindID2PriceRate4SH;
static	std::map<std::string,short>		s_mapCode2Kind4SH;
static	unsigned int					s_nTime4SH = 0;
static	unsigned int					s_nDate4SH = 0;

struct MappingSHFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHFuture_MkInfo2QuoMarketInfo() : InnerRecord( 107, sizeof(tagSHFutureMarketInfo_LF107), QUO_MARKET_SHFE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureMarketInfo_LF107*	pMkInfo = (tagSHFutureMarketInfo_LF107*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4SH = pMkInfo->MarketDate;
		}
	}
};

struct MappingSHFuture_Kind2QuoCategory : public InnerRecord { MappingSHFuture_Kind2QuoCategory() : InnerRecord( 108, sizeof(tagSHFutureKindDetail_LF108), QUO_MARKET_SHFE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureKindDetail_LF108*	pKind = (tagSHFutureKindDetail_LF108*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4SH[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingSHFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHFuture_MkStatus2QuoMarketInfo() : InnerRecord( 109, sizeof(tagSHFutureMarketStatus_HF109), QUO_MARKET_SHFE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureMarketStatus_HF109*	pMkStatus = (tagSHFutureMarketStatus_HF109*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4SH = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingSHFuture_Reference2QuoReference : public InnerRecord { MappingSHFuture_Reference2QuoReference() : InnerRecord( 110, sizeof(tagSHFutureReferenceData_LF110), QUO_MARKET_SHFE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureReferenceData_LF110*	pRefData = (tagSHFutureReferenceData_LF110*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4SH[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SH, s_mapKindID2PriceRate4SH );

			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingSHFuture_SnapLF2QuoSnapData : public InnerRecord { MappingSHFuture_SnapLF2QuoSnapData() : InnerRecord( 111, sizeof(tagSHFutureSnapData_LF111), QUO_MARKET_SHFE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureSnapData_LF111*		pSnapData = (tagSHFutureSnapData_LF111*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SH, s_mapKindID2PriceRate4SH );

			pBigTable->uiTime = s_nTime4SH;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingSHFuture_SnapHF2QuoSnapData : public InnerRecord { MappingSHFuture_SnapHF2QuoSnapData() : InnerRecord( 112, sizeof(tagSHFutureSnapData_HF112), QUO_MARKET_SHFE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureSnapData_HF112*		pSnapData = (tagSHFutureSnapData_HF112*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SH, s_mapKindID2PriceRate4SH );

			pBigTable->uiTime = s_nTime4SH;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingSHFuture_BuySell2QuoSnapData : public InnerRecord { MappingSHFuture_BuySell2QuoSnapData() : InnerRecord( 113, sizeof(tagSHFutureSnapBuySell_HF113), QUO_MARKET_SHFE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureSnapBuySell_HF113*	pSnapData = (tagSHFutureSnapBuySell_HF113*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4SH;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SH, s_mapKindID2PriceRate4SH );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 郑州商品期货
static	std::map<short,short>			s_mapKindID2PriceRate4ZZ;
static	std::map<std::string,short>		s_mapCode2Kind4ZZ;
static	unsigned int					s_nTime4ZZ = 0;
static	unsigned int					s_nDate4ZZ = 0;

struct MappingZZFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingZZFuture_MkInfo2QuoMarketInfo() : InnerRecord( 114, sizeof(tagZZFutureMarketInfo_LF114), QUO_MARKET_CZCE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureMarketInfo_LF114*	pMkInfo = (tagZZFutureMarketInfo_LF114*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4ZZ = pMkInfo->MarketDate;
		}
	}
};

struct MappingZZFuture_Kind2QuoCategory : public InnerRecord { MappingZZFuture_Kind2QuoCategory() : InnerRecord( 115, sizeof(tagZZFutureKindDetail_LF115), QUO_MARKET_CZCE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureKindDetail_LF115*	pKind = (tagZZFutureKindDetail_LF115*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4ZZ[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingZZFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingZZFuture_MkStatus2QuoMarketInfo() : InnerRecord( 116, sizeof(tagZZFutureMarketStatus_HF116), QUO_MARKET_CZCE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureMarketStatus_HF116*	pMkStatus = (tagZZFutureMarketStatus_HF116*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4ZZ = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingZZFuture_Reference2QuoReference : public InnerRecord { MappingZZFuture_Reference2QuoReference() : InnerRecord( 117, sizeof(tagZZFutureReferenceData_LF117), QUO_MARKET_CZCE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureReferenceData_LF117*	pRefData = (tagZZFutureReferenceData_LF117*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4ZZ[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZ, s_mapKindID2PriceRate4ZZ );

			pBigTable->uiKindID = pRefData->Kind;
			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingZZFuture_SnapLF2QuoSnapData : public InnerRecord { MappingZZFuture_SnapLF2QuoSnapData() : InnerRecord( 118, sizeof(tagZZFutureSnapData_LF118), QUO_MARKET_CZCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureSnapData_LF118*		pSnapData = (tagZZFutureSnapData_LF118*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZ, s_mapKindID2PriceRate4ZZ );

			pBigTable->uiTime = s_nTime4ZZ;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingZZFuture_SnapHF2QuoSnapData : public InnerRecord { MappingZZFuture_SnapHF2QuoSnapData() : InnerRecord( 119, sizeof(tagZZFutureSnapData_HF119), QUO_MARKET_CZCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureSnapData_HF119*		pSnapData = (tagZZFutureSnapData_HF119*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZ, s_mapKindID2PriceRate4ZZ );

			pBigTable->uiTime = s_nTime4ZZ;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingZZFuture_BuySell2QuoSnapData : public InnerRecord { MappingZZFuture_BuySell2QuoSnapData() : InnerRecord( 120, sizeof(tagZZFutureSnapBuySell_HF120), QUO_MARKET_CZCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureSnapBuySell_HF120*	pSnapData = (tagZZFutureSnapBuySell_HF120*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4ZZ;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZ, s_mapKindID2PriceRate4ZZ );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 大连商品期权
static	std::map<short,short>			s_mapKindID2PriceRate4DLOPT;
static	std::map<std::string,short>		s_mapCode2Kind4DLOPT;
static	unsigned int					s_nTime4DLOPT = 0;
static	unsigned int					s_nDate4DLOPT = 0;

struct MappingDLOption_MkInfo2QuoMarketInfo : public InnerRecord { MappingDLOption_MkInfo2QuoMarketInfo() : InnerRecord( 128, sizeof(tagDLOptionMarketInfo_LF128), QUO_MARKET_DCEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionMarketInfo_LF128*	pMkInfo = (tagDLOptionMarketInfo_LF128*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4DLOPT = pMkInfo->MarketDate;
		}
	}
};

struct MappingDLOption_Kind2QuoCategory : public InnerRecord { MappingDLOption_Kind2QuoCategory() : InnerRecord( 129, sizeof(tagDLOptionKindDetail_LF129), QUO_MARKET_DCEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionKindDetail_LF129*	pKind = (tagDLOptionKindDetail_LF129*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4DLOPT[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingDLOption_MkStatus2QuoMarketInfo : public InnerRecord { MappingDLOption_MkStatus2QuoMarketInfo() : InnerRecord( 130, sizeof(tagDLOptionMarketStatus_HF130), QUO_MARKET_DCEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionMarketStatus_HF130*	pMkStatus = (tagDLOptionMarketStatus_HF130*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4DLOPT = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingDLOption_Reference2QuoReference : public InnerRecord { MappingDLOption_Reference2QuoReference() : InnerRecord( 131, sizeof(tagDLOptionReferenceData_LF131), QUO_MARKET_DCEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionReferenceData_LF131*	pRefData = (tagDLOptionReferenceData_LF131*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4DLOPT[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DLOPT, s_mapKindID2PriceRate4DLOPT );

			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiExerciseDate = pRefData->XqDate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingDLOption_SnapLF2QuoSnapData : public InnerRecord { MappingDLOption_SnapLF2QuoSnapData() : InnerRecord( 132, sizeof(tagDLOptionSnapData_LF132), QUO_MARKET_DCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionSnapData_LF132*		pSnapData = (tagDLOptionSnapData_LF132*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DLOPT, s_mapKindID2PriceRate4DLOPT );

			pBigTable->uiTime = s_nTime4DLOPT;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingDLOption_SnapHF2QuoSnapData : public InnerRecord { MappingDLOption_SnapHF2QuoSnapData() : InnerRecord( 133, sizeof(tagDLOptionSnapData_HF133), QUO_MARKET_DCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionSnapData_HF133*		pSnapData = (tagDLOptionSnapData_HF133*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DLOPT, s_mapKindID2PriceRate4DLOPT );

			pBigTable->uiTime = s_nTime4DLOPT;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingDLOption_BuySell2QuoSnapData : public InnerRecord { MappingDLOption_BuySell2QuoSnapData() : InnerRecord( 134, sizeof(tagDLOptionSnapBuySell_HF134), QUO_MARKET_DCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionSnapBuySell_HF134*	pSnapData = (tagDLOptionSnapBuySell_HF134*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4DLOPT;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4DLOPT, s_mapKindID2PriceRate4DLOPT );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 上海商品期权
static	std::map<short,short>			s_mapKindID2PriceRate4SHOPT;
static	std::map<std::string,short>		s_mapCode2Kind4SHOPT;
static	unsigned int					s_nTime4SHOPT = 0;
static	unsigned int					s_nDate4SHOPT = 0;

struct MappingSHOption_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHOption_MkInfo2QuoMarketInfo() : InnerRecord( 135, sizeof(tagSHOptionMarketInfo_LF135), QUO_MARKET_SHFEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionMarketInfo_LF135*	pMkInfo = (tagSHOptionMarketInfo_LF135*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4SHOPT = pMkInfo->MarketDate;
		}
	}
};

struct MappingSHOption_Kind2QuoCategory : public InnerRecord { MappingSHOption_Kind2QuoCategory() : InnerRecord( 136, sizeof(tagSHOptionKindDetail_LF136), QUO_MARKET_SHFEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionKindDetail_LF136*	pKind = (tagSHOptionKindDetail_LF136*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4SHOPT[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingSHOption_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHOption_MkStatus2QuoMarketInfo() : InnerRecord( 137, sizeof(tagSHOptionMarketStatus_HF137), QUO_MARKET_SHFEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionMarketStatus_HF137*	pMkStatus = (tagSHOptionMarketStatus_HF137*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4SHOPT = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingSHOption_Reference2QuoReference : public InnerRecord { MappingSHOption_Reference2QuoReference() : InnerRecord( 138, sizeof(tagSHOptionReferenceData_LF138), QUO_MARKET_SHFEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionReferenceData_LF138*	pRefData = (tagSHOptionReferenceData_LF138*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4SHOPT[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHOPT, s_mapKindID2PriceRate4SHOPT );

			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiExerciseDate = pRefData->XqDate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingSHOption_SnapLF2QuoSnapData : public InnerRecord { MappingSHOption_SnapLF2QuoSnapData() : InnerRecord( 139, sizeof(tagSHOptionSnapData_LF139), QUO_MARKET_SHFEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionSnapData_LF139*		pSnapData = (tagSHOptionSnapData_LF139*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHOPT, s_mapKindID2PriceRate4SHOPT );

			pBigTable->uiTime = s_nTime4SHOPT;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingSHOption_SnapHF2QuoSnapData : public InnerRecord { MappingSHOption_SnapHF2QuoSnapData() : InnerRecord( 140, sizeof(tagSHOptionSnapData_HF140), QUO_MARKET_SHFEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionSnapData_HF140*		pSnapData = (tagSHOptionSnapData_HF140*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHOPT, s_mapKindID2PriceRate4SHOPT );

			pBigTable->uiTime = s_nTime4SHOPT;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingSHOption_BuySell2QuoSnapData : public InnerRecord { MappingSHOption_BuySell2QuoSnapData() : InnerRecord( 141, sizeof(tagSHOptionSnapBuySell_HF141), QUO_MARKET_SHFEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionSnapBuySell_HF141*	pSnapData = (tagSHOptionSnapBuySell_HF141*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4SHOPT;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHOPT, s_mapKindID2PriceRate4SHOPT );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 郑州商品期权
static	std::map<short,short>			s_mapKindID2PriceRate4ZZOPT;
static	std::map<std::string,short>		s_mapCode2Kind4ZZOPT;
static	unsigned int					s_nTime4ZZOPT = 0;
static	unsigned int					s_nDate4ZZOPT = 0;

struct MappingZZOption_MkInfo2QuoMarketInfo : public InnerRecord { MappingZZOption_MkInfo2QuoMarketInfo() : InnerRecord( 142, sizeof(tagZZOptionMarketInfo_LF142), QUO_MARKET_CZCEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionMarketInfo_LF142*	pMkInfo = (tagZZOptionMarketInfo_LF142*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4ZZOPT = pMkInfo->MarketDate;
		}
	}
};

struct MappingZZOption_Kind2QuoCategory : public InnerRecord { MappingZZOption_Kind2QuoCategory() : InnerRecord( 143, sizeof(tagZZOptionKindDetail_LF143), QUO_MARKET_CZCEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionKindDetail_LF143*	pKind = (tagZZOptionKindDetail_LF143*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4ZZOPT[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingZZOption_MkStatus2QuoMarketInfo : public InnerRecord { MappingZZOption_MkStatus2QuoMarketInfo() : InnerRecord( 144, sizeof(tagZZOptionMarketStatus_HF144), QUO_MARKET_CZCEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionMarketStatus_HF144*	pMkStatus = (tagZZOptionMarketStatus_HF144*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4ZZOPT = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingZZOption_Reference2QuoReference : public InnerRecord { MappingZZOption_Reference2QuoReference() : InnerRecord( 145, sizeof(tagZZOptionReferenceData_LF145), QUO_MARKET_CZCEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionReferenceData_LF145*	pRefData = (tagZZOptionReferenceData_LF145*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4ZZOPT[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZOPT, s_mapKindID2PriceRate4ZZOPT );

			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiExerciseDate = pRefData->XqDate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingZZOption_SnapLF2QuoSnapData : public InnerRecord { MappingZZOption_SnapLF2QuoSnapData() : InnerRecord( 146, sizeof(tagZZOptionSnapData_LF146), QUO_MARKET_CZCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionSnapData_LF146*		pSnapData = (tagZZOptionSnapData_LF146*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZOPT, s_mapKindID2PriceRate4ZZOPT );

			pBigTable->uiTime = s_nTime4ZZOPT;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;	
		}
	}
};

struct MappingZZOption_SnapHF2QuoSnapData : public InnerRecord { MappingZZOption_SnapHF2QuoSnapData() : InnerRecord( 147, sizeof(tagZZOptionSnapData_HF147), QUO_MARKET_CZCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionSnapData_HF147*		pSnapData = (tagZZOptionSnapData_HF147*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZOPT, s_mapKindID2PriceRate4ZZOPT );

			pBigTable->uiTime = s_nTime4ZZOPT;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingZZOption_BuySell2QuoSnapData : public InnerRecord { MappingZZOption_BuySell2QuoSnapData() : InnerRecord( 148, sizeof(tagZZOptionSnapBuySell_HF148), QUO_MARKET_CZCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionSnapBuySell_HF148*	pSnapData = (tagZZOptionSnapBuySell_HF148*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4ZZOPT;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZZOPT, s_mapKindID2PriceRate4ZZOPT );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 中金商品期货
static	std::map<short,short>			s_mapKindID2PriceRate4ZJ;
static	std::map<std::string,short>		s_mapCode2Kind4ZJ;
static	unsigned int					s_nTime4ZJ = 0;
static	unsigned int					s_nDate4ZJ = 0;

struct MappingCFFFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingCFFFuture_MkInfo2QuoMarketInfo() : InnerRecord( 172, sizeof(tagCFFFutureMarketInfo_LF172), QUO_MARKET_CFFEX*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureMarketInfo_LF172*	pMkInfo = (tagCFFFutureMarketInfo_LF172*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4ZJ = pMkInfo->MarketDate;
		}
	}
};

struct MappingCFFFuture_Kind2QuoCategory : public InnerRecord { MappingCFFFuture_Kind2QuoCategory() : InnerRecord( 173, sizeof(tagCFFFutureKindDetail_LF173), QUO_MARKET_CFFEX*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureKindDetail_LF173*	pKind = (tagCFFFutureKindDetail_LF173*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractMult = pKind->ContractMult;
				pBigTable->objData.mKindRecord[nIndex].uiContractUnit = pKind->ContractUnit;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4ZJ[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingCFFFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingCFFFuture_MkStatus2QuoMarketInfo() : InnerRecord( 174, sizeof(tagCFFFutureMarketStatus_HF174), QUO_MARKET_CFFEX*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureMarketStatus_HF174*	pMkStatus = (tagCFFFutureMarketStatus_HF174*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4ZJ = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingCFFFuture_Reference2QuoReference : public InnerRecord { MappingCFFFuture_Reference2QuoReference() : InnerRecord( 175, sizeof(tagCFFFutureReferenceData_LF175), QUO_MARKET_CFFEX*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureReferenceData_LF175*	pRefData = (tagCFFFutureReferenceData_LF175*)pMessagePtr;
			tagQUO_ReferenceData*				pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4ZJ[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZJ, s_mapKindID2PriceRate4ZJ );

			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiExerciseDate = pRefData->XqDate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingCFFFuture_SnapLF2QuoSnapData : public InnerRecord { MappingCFFFuture_SnapLF2QuoSnapData() : InnerRecord( 176, sizeof(tagCFFFutureSnapData_LF176), QUO_MARKET_CFFEX*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureSnapData_LF176*		pSnapData = (tagCFFFutureSnapData_LF176*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZJ, s_mapKindID2PriceRate4ZJ );

			pBigTable->uiTime = s_nTime4ZJ;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingCFFFuture_SnapHF2QuoSnapData : public InnerRecord { MappingCFFFuture_SnapHF2QuoSnapData() : InnerRecord( 177, sizeof(tagCFFFutureSnapData_HF177), QUO_MARKET_CFFEX*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureSnapData_HF177*		pSnapData = (tagCFFFutureSnapData_HF177*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZJ, s_mapKindID2PriceRate4ZJ );

			pBigTable->uiTime = s_nTime4ZJ;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingCFFFuture_BuySell2QuoSnapData : public InnerRecord { MappingCFFFuture_BuySell2QuoSnapData() : InnerRecord( 178, sizeof(tagCFFFutureSnapBuySell_HF178), QUO_MARKET_CFFEX*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagCFFFutureSnapBuySell_HF178*	pSnapData = (tagCFFFutureSnapBuySell_HF178*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4ZJ;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4ZJ, s_mapKindID2PriceRate4ZJ );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 上海Lv1现货
static	std::map<short,short>			s_mapKindID2PriceRate4SHL1;
static	std::map<std::string,short>		s_mapCode2Kind4SHL1;
static	unsigned int					s_nTime4SHL1 = 0;
static	unsigned int					s_nDate4SHL1 = 0;

struct MappingSHL1_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHL1_MkInfo2QuoMarketInfo() : InnerRecord( 149, sizeof(tagSHL1MarketInfo_LF149), QUO_MARKET_SSE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1MarketInfo_LF149*		pMkInfo = (tagSHL1MarketInfo_LF149*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4SHL1 = pMkInfo->MarketDate;
		}
	}
};

struct MappingSHL1_Kind2QuoCategory : public InnerRecord { MappingSHL1_Kind2QuoCategory() : InnerRecord( 150, sizeof(tagSHL1KindDetail_LF150), QUO_MARKET_SSE*100+2, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1KindDetail_LF150*		pKind = (tagSHL1KindDetail_LF150*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				s_mapKindID2PriceRate4SHL1[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingSHL1_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHL1_MkStatus2QuoMarketInfo() : InnerRecord( 151, sizeof(tagSHL1MarketStatus_HF151), QUO_MARKET_SSE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1MarketStatus_HF151*		pMkStatus = (tagSHL1MarketStatus_HF151*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4SHL1 = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingSHL1_Reference2QuoReference : public InnerRecord { MappingSHL1_Reference2QuoReference() : InnerRecord( 152, sizeof(tagSHL1ReferenceData_LF152), QUO_MARKET_SSE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1ReferenceData_LF152	*	pRefData = (tagSHL1ReferenceData_LF152*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4SHL1[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
		}
	}
};

struct MappingSHL1_Extension2QuoReference : public InnerRecord { MappingSHL1_Extension2QuoReference() : InnerRecord( 153, sizeof(tagSHL1ReferenceExtension_LF153), QUO_MARKET_SSE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1ReferenceExtension_LF153*	pExtensionData = (tagSHL1ReferenceExtension_LF153*)pMessagePtr;
			tagQUO_ReferenceData*				pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);
		}
	}
};

struct MappingSHL1_SnapLF2QuoSnapData : public InnerRecord { MappingSHL1_SnapLF2QuoSnapData() : InnerRecord( 154, sizeof(tagSHL1SnapData_LF154), QUO_MARKET_SSE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1SnapData_LF154*			pSnapData = (tagSHL1SnapData_LF154*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1, s_mapKindID2PriceRate4SHL1 );

			pBigTable->uiTime = s_nTime4SHL1;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->HighLimit / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowLimit / dRate;
		}
	}
};

struct MappingSHL1_SnapHF2QuoSnapData : public InnerRecord { MappingSHL1_SnapHF2QuoSnapData() : InnerRecord( 155, sizeof(tagSHL1SnapData_HF155), QUO_MARKET_SSE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1SnapData_HF155*			pSnapData = (tagSHL1SnapData_HF155*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1, s_mapKindID2PriceRate4SHL1 );

			pBigTable->uiTime = s_nTime4SHL1;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
		}
	}
};

struct MappingSHL1_BuySell2QuoSnapData : public InnerRecord { MappingSHL1_BuySell2QuoSnapData() : InnerRecord( 156, sizeof(tagSHL1SnapBuySell_HF156), QUO_MARKET_SSE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHL1SnapBuySell_HF156*	pSnapData = (tagSHL1SnapBuySell_HF156*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4SHL1;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1, s_mapKindID2PriceRate4SHL1 );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};


///< 上海期权
static	std::map<short,short>			s_mapKindID2PriceRate4SHL1OPT;
static	std::map<std::string,short>		s_mapCode2Kind4SHL1OPT;
static	unsigned int					s_nTime4SHL1OPT = 0;
static	unsigned int					s_nDate4SHL1OPT = 0;

struct MappingSHL1Option_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHL1Option_MkInfo2QuoMarketInfo() : InnerRecord( 157, sizeof(tagSHOptMarketInfo_LF157), QUO_MARKET_SSEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptMarketInfo_LF157*	pMkInfo = (tagSHOptMarketInfo_LF157*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4SHL1OPT = pMkInfo->MarketDate;
		}
	}
};

struct MappingSHL1Option_Kind2QuoCategory : public InnerRecord { MappingSHL1Option_Kind2QuoCategory() : InnerRecord( 158, sizeof(tagSHOptKindDetail_LF158), QUO_MARKET_SSEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptKindDetail_LF158*	pKind = (tagSHOptKindDetail_LF158*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::memcpy( pBigTable->objData.mKindRecord[nIndex].szUnderlyingCode, pKind->UnderlyingCode, sizeof(pKind->UnderlyingCode) );
				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].cOptionType = pKind->OptionType;
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				pBigTable->objData.mKindRecord[nIndex].uiContractUnit = pKind->ContractUnit;
				pBigTable->objData.mKindRecord[nIndex].dPriceTick = pKind->PriceTick;
				s_mapKindID2PriceRate4SHL1OPT[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingSHL1Option_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHL1Option_MkStatus2QuoMarketInfo() : InnerRecord( 159, sizeof(tagSHOptMarketStatus_HF159), QUO_MARKET_SSEOPT*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptMarketStatus_HF159*		pMkStatus = (tagSHOptMarketStatus_HF159*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4SHL1OPT = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingSHL1Option_Reference2QuoReference : public InnerRecord { MappingSHL1Option_Reference2QuoReference() : InnerRecord( 160, sizeof(tagSHOptReferenceData_LF160), QUO_MARKET_SSEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptReferenceData_LF160*	pRefData = (tagSHOptReferenceData_LF160*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4SHL1OPT[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1OPT, s_mapKindID2PriceRate4SHL1OPT );

			pBigTable->dExercisePrice = pRefData->XqPrice / dRate;
			pBigTable->uiExerciseDate = pRefData->XqDate;
			pBigTable->uiStartDate = pRefData->StartDate;
			pBigTable->uiEndDate = pRefData->EndDate;
			pBigTable->uiDeliveryDate = pRefData->DeliveryDate;
			pBigTable->uiExpireDate = pRefData->ExpireDate;
			pBigTable->cCallOrPut = pRefData->CallOrPut;
		}
	}
};

struct MappingSHL1Option_SnapLF2QuoSnapData : public InnerRecord { MappingSHL1Option_SnapLF2QuoSnapData() : InnerRecord( 161, sizeof(tagSHOptSnapData_LF161), QUO_MARKET_SSEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptSnapData_LF161*		pSnapData = (tagSHOptSnapData_LF161*)pMessagePtr;
			tagQUO_SnapData*			pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1OPT, s_mapKindID2PriceRate4SHL1OPT );

			pBigTable->uiTime = s_nTime4SHL1OPT;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->UpperPrice / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowerPrice / dRate;
			pBigTable->dSettlePx = pSnapData->SettlePrice / dRate;
			pBigTable->dPreSettlePx = pSnapData->PreSettlePrice / dRate;
			pBigTable->ui64PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingSHL1Option_SnapHF2QuoSnapData : public InnerRecord { MappingSHL1Option_SnapHF2QuoSnapData() : InnerRecord( 162, sizeof(tagSHOptSnapData_HF162), QUO_MARKET_SSEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptSnapData_HF162*		pSnapData = (tagSHOptSnapData_HF162*)pMessagePtr;
			tagQUO_SnapData*			pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1OPT, s_mapKindID2PriceRate4SHL1OPT );

			pBigTable->uiTime = s_nTime4SHL1OPT;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
			pBigTable->ui64OpenInterest = pSnapData->Position;
		}
	}
};

struct MappingSHL1Option_BuySell2QuoSnapData : public InnerRecord { MappingSHL1Option_BuySell2QuoSnapData() : InnerRecord( 163, sizeof(tagSHOptSnapBuySell_HF163), QUO_MARKET_SSEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSHOptSnapBuySell_HF163*	pSnapData = (tagSHOptSnapBuySell_HF163*)pMessagePtr;
			tagQUO_SnapData*			pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4SHL1OPT;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SHL1OPT, s_mapKindID2PriceRate4SHL1OPT );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};

///< 深圳Lv1现货
static	std::map<short,short>			s_mapKindID2PriceRate4SZL1;
static	std::map<std::string,short>		s_mapCode2Kind4SZL1;
static	unsigned int					s_nTime4SZL1 = 0;
static	unsigned int					s_nDate4SZL1 = 0;

struct MappingSZL1_MkInfo2QuoMarketInfo : public InnerRecord { MappingSZL1_MkInfo2QuoMarketInfo() : InnerRecord( 164, sizeof(tagSZL1MarketInfo_LF164), QUO_MARKET_SZSE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1MarketInfo_LF164*	pMkInfo = (tagSZL1MarketInfo_LF164*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			::memset( pBigTable->szCode, 0, sizeof(pBigTable->szCode) );
			pBigTable->objData.uiMarketDate = pMkInfo->MarketDate;
			pBigTable->objData.eMarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->objData.uiKindCount = pMkInfo->KindCount;
			pBigTable->objData.uiWareCount = pMkInfo->WareCount;
			s_nDate4SZL1 = pMkInfo->MarketDate;
		}
	}
};

struct MappingSZL1_Kind2QuoCategory : public InnerRecord { MappingSZL1_Kind2QuoCategory() : InnerRecord( 165, sizeof(tagSZL1KindDetail_LF165), QUO_MARKET_SZSE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1KindDetail_LF165*		pKind = (tagSZL1KindDetail_LF165*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);
			int								nIndex = ::atoi( pKind->Key );

			if( nIndex >= 0 && nIndex < QUO_MAX_KIND )
			{
				pBigTable->objData.mKindRecord[nIndex].uiTradeSessionCount = pKind->PeriodsCount;
				memcpy( (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+nIndex, (pBigTable->objData.mKindRecord[0].mTradeSessionRecord)+0, sizeof(tagQUO_TradeSession) );

				::strcpy( pBigTable->objData.mKindRecord[nIndex].szKindName, pKind->KindName );
				pBigTable->objData.mKindRecord[nIndex].uiLotSize = pKind->LotSize;
				s_mapKindID2PriceRate4SZL1[nIndex] = pKind->PriceRate;
			}
		}
	}
};

struct MappingSZL1_MkStatus2QuoMarketInfo : public InnerRecord { MappingSZL1_MkStatus2QuoMarketInfo() : InnerRecord( 166, sizeof(tagSZL1MarketStatus_HF166), QUO_MARKET_SZSE*100+1, true ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1MarketStatus_HF166*		pMkStatus = (tagSZL1MarketStatus_HF166*)pMessagePtr;
			T_Inner_MarketInfo*				pBigTable = (T_Inner_MarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->objData.uiMarketTime = pMkStatus->MarketTime;
			s_nTime4SZL1 = pMkStatus->MarketTime * 1000;
		}
	}
};

struct MappingSZL1_Reference2QuoReference : public InnerRecord { MappingSZL1_Reference2QuoReference() : InnerRecord( 167, sizeof(tagSZL1ReferenceData_LF167), QUO_MARKET_SZSE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1ReferenceData_LF167	*	pRefData = (tagSZL1ReferenceData_LF167*)pMessagePtr;
			tagQUO_ReferenceData*			pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);

			::memcpy( pBigTable->szCode, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->szName, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->uiKindID = pRefData->Kind;
			s_mapCode2Kind4SZL1[std::string(pBigTable->szCode)] = pBigTable->uiKindID;
		}
	}
};

struct MappingSZL1_Extension2QuoReference : public InnerRecord { MappingSZL1_Extension2QuoReference() : InnerRecord( 168, sizeof(tagSZL1ReferenceExtension_LF168), QUO_MARKET_SZSE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1ReferenceExtension_LF168*	pExtensionData = (tagSZL1ReferenceExtension_LF168*)pMessagePtr;
			tagQUO_ReferenceData*				pBigTable = (tagQUO_ReferenceData*)&(m_objUnionData.ReferenceData_2);
		}
	}
};

struct MappingSZL1_SnapLF2QuoSnapData : public InnerRecord { MappingSZL1_SnapLF2QuoSnapData() : InnerRecord( 169, sizeof(tagSZL1SnapData_LF169), QUO_MARKET_SZSE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1SnapData_LF169*			pSnapData = (tagSZL1SnapData_LF169*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SZL1, s_mapKindID2PriceRate4SZL1 );

			pBigTable->uiTime = s_nTime4SZL1;
			pBigTable->dOpenPx = pSnapData->Open / dRate;
			pBigTable->dClosePx = pSnapData->Close / dRate;
			pBigTable->dPreClosePx = pSnapData->PreClose / dRate;
			pBigTable->dUpperLimitPx = pSnapData->HighLimit / dRate;
			pBigTable->dLowerLimitPx = pSnapData->LowLimit / dRate;
		}
	}
};

struct MappingSZL1_SnapHF2QuoSnapData : public InnerRecord { MappingSZL1_SnapHF2QuoSnapData() : InnerRecord( 170, sizeof(tagSZL1SnapData_HF170), QUO_MARKET_SZSE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1SnapData_HF170*			pSnapData = (tagSZL1SnapData_HF170*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SZL1, s_mapKindID2PriceRate4SZL1 );

			pBigTable->uiTime = s_nTime4SZL1;
			pBigTable->dNowPx = pSnapData->Now / dRate;
			pBigTable->dHighPx = pSnapData->High / dRate;
			pBigTable->dLowPx = pSnapData->Low / dRate;
			pBigTable->dAmount = pSnapData->Amount;
			pBigTable->ui64Volume = pSnapData->Volume;
		}
	}
};

struct MappingSZL1_BuySell2QuoSnapData : public InnerRecord { MappingSZL1_BuySell2QuoSnapData() : InnerRecord( 171, sizeof(tagSZL1SnapBuySell_HF171), QUO_MARKET_SZSE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr, bool bNewRecord = false )	{
		if( NULL != pMessagePtr )	{
			tagSZL1SnapBuySell_HF171*		pSnapData = (tagSZL1SnapBuySell_HF171*)pMessagePtr;
			tagQUO_SnapData*				pBigTable = (tagQUO_SnapData*)&(m_objUnionData.SnapData_3);

			::memcpy( pBigTable->szCode, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->uiTime = s_nTime4SZL1;
			if( true == bNewRecord )	{	pBigTable->uiKindID = 0xFFFFffff;	}
			double							dRate = CalPriceRate( pBigTable->szCode, pBigTable->uiKindID, s_mapCode2Kind4SZL1, s_mapKindID2PriceRate4SZL1 );

			for( int n = 0; n < 5; n++ )
			{
				pBigTable->mBid[n].dVPrice = pSnapData->Buy[n].Price / dRate;
				pBigTable->mBid[n].ui64Volume = pSnapData->Buy[n].Volume;
				pBigTable->mAsk[n].dVPrice = pSnapData->Sell[n].Price / dRate;
				pBigTable->mAsk[n].ui64Volume = pSnapData->Sell[n].Volume;
			}
		}
	}
};


///< -----------------------------------------------------------------------------
const unsigned int TableFillerRegister::s_nRegisterTableSize = 2048;
std::vector<InnerRecord*> TableFillerRegister::s_vctRegisterTable;

int TableFillerRegister::Initialize()
{
	///< ------------- 建立行情数据结构到内存数据结构的映射表 --------------
	///< 大连商品期货
	static MappingDLFuture_MkInfo2QuoMarketInfo		objDLFuture100Cast;	if( GetRegister().Register( objDLFuture100Cast ) != 0 )	return -1;
	static MappingDLFuture_Kind2QuoCategory			objDLFuture101Cast;	if( GetRegister().Register( objDLFuture101Cast ) != 0 )	return -2;
	static MappingDLFuture_MkStatus2QuoMarketInfo	objDLFuture102Cast;	if( GetRegister().Register( objDLFuture102Cast ) != 0 )	return -3;
	static MappingDLFuture_Reference2QuoReference	objDLFuture103Cast;	if( GetRegister().Register( objDLFuture103Cast ) != 0 )	return -4;
	static MappingDLFuture_SnapLF2QuoSnapData		objDLFuture104Cast;	if( GetRegister().Register( objDLFuture104Cast ) != 0 )	return -5;
	static MappingDLFuture_SnapHF2QuoSnapData		objDLFuture105Cast;	if( GetRegister().Register( objDLFuture105Cast ) != 0 )	return -6;
	static MappingDLFuture_BuySell2QuoSnapData		objDLFuture106Cast;	if( GetRegister().Register( objDLFuture106Cast ) != 0 )	return -7;

	///< 上海商品期货
	static MappingSHFuture_MkInfo2QuoMarketInfo		objSHFuture107Cast;	if( GetRegister().Register( objSHFuture107Cast ) != 0 )	return -8;
	static MappingSHFuture_Kind2QuoCategory			objSHFuture108Cast;	if( GetRegister().Register( objSHFuture108Cast ) != 0 )	return -9;
	static MappingSHFuture_MkStatus2QuoMarketInfo	objSHFuture109Cast;	if( GetRegister().Register( objSHFuture109Cast ) != 0 )	return -10;
	static MappingSHFuture_Reference2QuoReference	objSHFuture110Cast;	if( GetRegister().Register( objSHFuture110Cast ) != 0 )	return -11;
	static MappingSHFuture_SnapLF2QuoSnapData		objSHFuture111Cast;	if( GetRegister().Register( objSHFuture111Cast ) != 0 )	return -12;
	static MappingSHFuture_SnapHF2QuoSnapData		objSHFuture112Cast;	if( GetRegister().Register( objSHFuture112Cast ) != 0 )	return -13;
	static MappingSHFuture_BuySell2QuoSnapData		objSHFuture113Cast;	if( GetRegister().Register( objSHFuture113Cast ) != 0 )	return -14;

	///< 郑州商品期货
	static MappingZZFuture_MkInfo2QuoMarketInfo		objZZFuture114Cast;	if( GetRegister().Register( objZZFuture114Cast ) != 0 )	return -15;
	static MappingZZFuture_Kind2QuoCategory			objZZFuture115Cast;	if( GetRegister().Register( objZZFuture115Cast ) != 0 )	return -16;
	static MappingZZFuture_MkStatus2QuoMarketInfo	objZZFuture116Cast;	if( GetRegister().Register( objZZFuture116Cast ) != 0 )	return -17;
	static MappingZZFuture_Reference2QuoReference	objZZFuture117Cast;	if( GetRegister().Register( objZZFuture117Cast ) != 0 )	return -18;
	static MappingZZFuture_SnapLF2QuoSnapData		objZZFuture118Cast;	if( GetRegister().Register( objZZFuture118Cast ) != 0 )	return -19;
	static MappingZZFuture_SnapHF2QuoSnapData		objZZFuture119Cast;	if( GetRegister().Register( objZZFuture119Cast ) != 0 )	return -20;
	static MappingZZFuture_BuySell2QuoSnapData		objZZFuture120Cast;	if( GetRegister().Register( objZZFuture120Cast ) != 0 )	return -21;

	///< 大连商品期权
	static MappingDLOption_MkInfo2QuoMarketInfo		objDLOption128Cast;	if( GetRegister().Register( objDLOption128Cast ) != 0 )	return -22;
	static MappingDLOption_Kind2QuoCategory			objDLOption129Cast;	if( GetRegister().Register( objDLOption129Cast ) != 0 )	return -23;
	static MappingDLOption_MkStatus2QuoMarketInfo	objDLOption130Cast;	if( GetRegister().Register( objDLOption130Cast ) != 0 )	return -24;
	static MappingDLOption_Reference2QuoReference	objDLOption131Cast;	if( GetRegister().Register( objDLOption131Cast ) != 0 )	return -25;
	static MappingDLOption_SnapLF2QuoSnapData		objDLOption132Cast;	if( GetRegister().Register( objDLOption132Cast ) != 0 )	return -26;
	static MappingDLOption_SnapHF2QuoSnapData		objDLOption133Cast;	if( GetRegister().Register( objDLOption133Cast ) != 0 )	return -27;
	static MappingDLOption_BuySell2QuoSnapData		objDLOption134Cast;	if( GetRegister().Register( objDLOption134Cast ) != 0 )	return -28;

	///< 上海商品期权
	static MappingSHOption_MkInfo2QuoMarketInfo		objSHOption135Cast;	if( GetRegister().Register( objSHOption135Cast ) != 0 )	return -28;
	static MappingSHOption_Kind2QuoCategory			objSHOption136Cast;	if( GetRegister().Register( objSHOption136Cast ) != 0 )	return -29;
	static MappingSHOption_MkStatus2QuoMarketInfo	objSHOption137Cast;	if( GetRegister().Register( objSHOption137Cast ) != 0 )	return -30;
	static MappingSHOption_Reference2QuoReference	objSHOption138Cast;	if( GetRegister().Register( objSHOption138Cast ) != 0 )	return -31;
	static MappingSHOption_SnapLF2QuoSnapData		objSHOption139Cast;	if( GetRegister().Register( objSHOption139Cast ) != 0 )	return -32;
	static MappingSHOption_SnapHF2QuoSnapData		objSHOption140Cast;	if( GetRegister().Register( objSHOption140Cast ) != 0 )	return -33;
	static MappingSHOption_BuySell2QuoSnapData		objSHOption141Cast;	if( GetRegister().Register( objSHOption141Cast ) != 0 )	return -34;

	///< 郑州商品期权
	static MappingZZOption_MkInfo2QuoMarketInfo		objZZOption142Cast;	if( GetRegister().Register( objZZOption142Cast ) != 0 )	return -35;
	static MappingZZOption_Kind2QuoCategory			objZZOption143Cast;	if( GetRegister().Register( objZZOption143Cast ) != 0 )	return -36;
	static MappingZZOption_MkStatus2QuoMarketInfo	objZZOption144Cast;	if( GetRegister().Register( objZZOption144Cast ) != 0 )	return -37;
	static MappingZZOption_Reference2QuoReference	objZZOption145Cast;	if( GetRegister().Register( objZZOption145Cast ) != 0 )	return -38;
	static MappingZZOption_SnapLF2QuoSnapData		objZZOption146Cast;	if( GetRegister().Register( objZZOption146Cast ) != 0 )	return -39;
	static MappingZZOption_SnapHF2QuoSnapData		objZZOption147Cast;	if( GetRegister().Register( objZZOption147Cast ) != 0 )	return -40;
	static MappingZZOption_BuySell2QuoSnapData		objZZOption148Cast;	if( GetRegister().Register( objZZOption148Cast ) != 0 )	return -41;

	///< 中金商品期货
	static MappingCFFFuture_MkInfo2QuoMarketInfo	objCFFFuture172Cast;	if( GetRegister().Register( objCFFFuture172Cast ) != 0 )	return -42;
	static MappingCFFFuture_Kind2QuoCategory		objCFFFuture173Cast;	if( GetRegister().Register( objCFFFuture173Cast ) != 0 )	return -43;
	static MappingCFFFuture_MkStatus2QuoMarketInfo	objCFFFuture174Cast;	if( GetRegister().Register( objCFFFuture174Cast ) != 0 )	return -44;
	static MappingCFFFuture_Reference2QuoReference	objCFFFuture175Cast;	if( GetRegister().Register( objCFFFuture175Cast ) != 0 )	return -45;
	static MappingCFFFuture_SnapLF2QuoSnapData		objCFFFuture176Cast;	if( GetRegister().Register( objCFFFuture176Cast ) != 0 )	return -46;
	static MappingCFFFuture_SnapHF2QuoSnapData		objCFFFuture177Cast;	if( GetRegister().Register( objCFFFuture177Cast ) != 0 )	return -47;
	static MappingCFFFuture_BuySell2QuoSnapData		objCFFFuture178Cast;	if( GetRegister().Register( objCFFFuture178Cast ) != 0 )	return -48;

	///< 上海Lv1现货
	static MappingSHL1_MkInfo2QuoMarketInfo			objSHL1149Cast;	if( GetRegister().Register( objSHL1149Cast ) != 0 )	return -49;
	static MappingSHL1_Kind2QuoCategory				objSHL1150Cast;	if( GetRegister().Register( objSHL1150Cast ) != 0 )	return -50;
	static MappingSHL1_MkStatus2QuoMarketInfo		objSHL1151Cast;	if( GetRegister().Register( objSHL1151Cast ) != 0 )	return -51;
	static MappingSHL1_Reference2QuoReference		objSHL1152Cast;	if( GetRegister().Register( objSHL1152Cast ) != 0 )	return -52;
	static MappingSHL1_Extension2QuoReference		objSHL1153Cast;	if( GetRegister().Register( objSHL1153Cast ) != 0 )	return -53;
	static MappingSHL1_SnapLF2QuoSnapData			objSHL1154Cast;	if( GetRegister().Register( objSHL1154Cast ) != 0 )	return -54;
	static MappingSHL1_SnapHF2QuoSnapData			objSHL1155Cast;	if( GetRegister().Register( objSHL1155Cast ) != 0 )	return -55;
	static MappingSHL1_BuySell2QuoSnapData			objSHL1156Cast;	if( GetRegister().Register( objSHL1156Cast ) != 0 )	return -56;

	///< 上海期权
	static MappingSHL1Option_MkInfo2QuoMarketInfo	objSHL1Option157Cast;	if( GetRegister().Register( objSHL1Option157Cast ) != 0 )	return -57;
	static MappingSHL1Option_Kind2QuoCategory		objSHL1Option158Cast;	if( GetRegister().Register( objSHL1Option158Cast ) != 0 )	return -58;
	static MappingSHL1Option_MkStatus2QuoMarketInfo	objSHL1Option159Cast;	if( GetRegister().Register( objSHL1Option159Cast ) != 0 )	return -59;
	static MappingSHL1Option_Reference2QuoReference	objSHL1Option160Cast;	if( GetRegister().Register( objSHL1Option160Cast ) != 0 )	return -60;
	static MappingSHL1Option_SnapLF2QuoSnapData		objSHL1Option161Cast;	if( GetRegister().Register( objSHL1Option161Cast ) != 0 )	return -61;
	static MappingSHL1Option_SnapHF2QuoSnapData		objSHL1Option162Cast;	if( GetRegister().Register( objSHL1Option162Cast ) != 0 )	return -62;
	static MappingSHL1Option_BuySell2QuoSnapData	objSHL1Option163Cast;	if( GetRegister().Register( objSHL1Option163Cast ) != 0 )	return -63;

	///< 深圳Lv1现货
	static MappingSZL1_MkInfo2QuoMarketInfo			objSZL1164Cast;	if( GetRegister().Register( objSZL1164Cast ) != 0 )	return -64;
	static MappingSZL1_Kind2QuoCategory				objSZL1165Cast;	if( GetRegister().Register( objSZL1165Cast ) != 0 )	return -65;
	static MappingSZL1_MkStatus2QuoMarketInfo		objSZL1166Cast;	if( GetRegister().Register( objSZL1166Cast ) != 0 )	return -66;
	static MappingSZL1_Reference2QuoReference		objSZL1167Cast;	if( GetRegister().Register( objSZL1167Cast ) != 0 )	return -67;
	static MappingSZL1_Extension2QuoReference		objSZL1168Cast;	if( GetRegister().Register( objSZL1168Cast ) != 0 )	return -68;
	static MappingSZL1_SnapLF2QuoSnapData			objSZL1169Cast;	if( GetRegister().Register( objSZL1169Cast ) != 0 )	return -69;
	static MappingSZL1_SnapHF2QuoSnapData			objSZL1170Cast;	if( GetRegister().Register( objSZL1170Cast ) != 0 )	return -70;
	static MappingSZL1_BuySell2QuoSnapData			objSZL1171Cast;	if( GetRegister().Register( objSZL1171Cast ) != 0 )	return -71;

	return 0;
}

int TableFillerRegister::Register( InnerRecord& refRecordFiller )
{
	unsigned int			nMessageID = refRecordFiller.GetMessageID();
	unsigned int			nPos = nMessageID % s_nRegisterTableSize;
	InnerRecord*			pInnerRecord = s_vctRegisterTable[nPos];

	if( NULL != pInnerRecord )	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::Register() : filler has existed in register table, msgid(%d)", nMessageID );
		return -1;
	}

	s_vctRegisterTable[nPos] = &refRecordFiller;
	DataIOEngine::GetEngineObj().WriteInfo( "TableFillerRegister::Register() : Building Mapping Table..., MessageID(%u-->%u), Size(%u-->%u)", nMessageID, refRecordFiller.GetBigTableID(), refRecordFiller.GetMessageLength(), refRecordFiller.GetBigTableWidth() );

	if( nMessageID == 0 || refRecordFiller.GetBigTableID() == 0 || refRecordFiller.GetMessageLength() == 0 || refRecordFiller.GetBigTableWidth() == 0 )	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::Register() : invalid parameter : MessageID(%u-->%u), Size(%u-->%u)", nMessageID, refRecordFiller.GetBigTableID(), refRecordFiller.GetMessageLength(), refRecordFiller.GetBigTableWidth() );
		return -2;
	}

	return 0;
}

InnerRecord* TableFillerRegister::PrepareNewTableBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen )
{
	unsigned int			nPos = nMessageID % s_nRegisterTableSize;
	InnerRecord*			pInnerRecord = s_vctRegisterTable[nPos];

	if( NULL == pMsgPtr || nMsgLen < 20 || NULL == pInnerRecord )	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : invalid parameters, MessageID(%d), MessagePtr(%x), MessageLength(%d), MemoryBlockPtr(%x)", nMessageID, pMsgPtr, nMsgLen, pInnerRecord );
		return NULL;
	}

	if( pInnerRecord->GetMessageID() != nMessageID || pInnerRecord->GetMessageLength() != nMsgLen )	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : invalid parameters, MessageID(%d!=%d), MessageLength(%d!=%d)", nMessageID, pInnerRecord->GetMessageID(), nMsgLen, pInnerRecord->GetMessageLength() );
		return NULL;
	}

	::memset( pInnerRecord->GetBigTableRecordPtr(), 0, pInnerRecord->GetBigTableWidth() );	///< 先清一把，腾出空间
	if( false == pInnerRecord->IsSingleton() )												///< 对于只有一条记录的数据表类型，不需要copy商品代码
	{
		::memcpy( pInnerRecord->GetBigTableRecordPtr(), pMsgPtr, 20 );						///< 再把Code先copy进去
	}

	return pInnerRecord;
}



