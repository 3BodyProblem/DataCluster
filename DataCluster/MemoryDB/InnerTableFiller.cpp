#include "InnerTableFiller.h"
#include "../DataCenterEngine/DataCenterEngine.h"


InnerRecord::InnerRecord( unsigned int nMsgID, unsigned int nMsgLen, unsigned int nBigTableID )
 : m_nMessageID( nMsgID ), m_nMessageLength( nMsgLen ), m_nBigTableID( nBigTableID )
{
}

unsigned int InnerRecord::GetBigTableID()
{
	return m_nBigTableID;
}

unsigned int InnerRecord::GetMessageID()
{
	return m_nMessageID;
}

unsigned int InnerRecord::GetMessageLength()
{
	return m_nMessageLength;
}

char* InnerRecord::GetBigTableRecordPtr()
{
	switch( GetBigTableID() % 100 )
	{
	case 1:
		return (char*)&(m_objUnionData.MarketData_1);
	case 2:
		return (char*)&(m_objUnionData.CategoryData_2);
	case 3:
		return (char*)&(m_objUnionData.ReferenceData_3);
	case 4:
		return (char*)&(m_objUnionData.SnapData_4);
	default:
		return NULL;
	}

	return NULL;
}

unsigned int InnerRecord::GetBigTableWidth()
{
	switch( GetBigTableID() % 100 )
	{
	case 1:
		return sizeof( m_objUnionData.MarketData_1 );
	case 2:
		return sizeof( m_objUnionData.CategoryData_2 );
	case 3:
		return sizeof( m_objUnionData.ReferenceData_3 );
	case 4:
		return sizeof( m_objUnionData.SnapData_4 );
	default:
		return 0;
	}

	return 0;
}


///< -----------------------------------------------------------------------------

///< 大连商品期货
struct MappingDLFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingDLFuture_MkInfo2QuoMarketInfo() : InnerRecord( 100, sizeof(tagDLFutureMarketInfo_LF100), QUO_MARKET_DCE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureMarketInfo_LF100*	pMkInfo = (tagDLFutureMarketInfo_LF100*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketDate = pMkInfo->MarketDate;
			pBigTable->MarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->WareCount = pMkInfo->KindCount;
			pBigTable->WareCount = pMkInfo->WareCount;
			pBigTable->PeriodsCount = pMkInfo->PeriodsCount;
			::memcpy( pBigTable->MarketPeriods, pMkInfo->MarketPeriods, sizeof(pBigTable->MarketPeriods) );
		}
	}
};

struct MappingDLFuture_Kind2QuoCategory : public InnerRecord { MappingDLFuture_Kind2QuoCategory() : InnerRecord( 101, sizeof(tagDLFutureKindDetail_LF101), QUO_MARKET_DCE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureKindDetail_LF101*	pKind = (tagDLFutureKindDetail_LF101*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingDLFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingDLFuture_MkStatus2QuoMarketInfo() : InnerRecord( 102, sizeof(tagDLFutureMarketStatus_HF102), QUO_MARKET_DCE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureMarketStatus_HF102*	pMkStatus = (tagDLFutureMarketStatus_HF102*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingDLFuture_Reference2QuoReference : public InnerRecord { MappingDLFuture_Reference2QuoReference() : InnerRecord( 103, sizeof(tagDLFutureReferenceData_LF103), QUO_MARKET_DCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureReferenceData_LF103*	pRefData = (tagDLFutureReferenceData_LF103*)pMessagePtr;
			tagQuoReferenceData*			pBigTable = (tagQuoReferenceData*)&(m_objUnionData.ReferenceData_3);

			::memcpy( pBigTable->Code, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->Name, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->Kind = pRefData->Kind;
			pBigTable->DerivativeType = pRefData->DerivativeType;
			pBigTable->LotSize = pRefData->LotSize;
			::memcpy( pBigTable->UnderlyingCode, pRefData->UnderlyingCode, sizeof(pRefData->UnderlyingCode) );
			pBigTable->ContractMult = pRefData->ContractMult;
			pBigTable->XqPrice = pRefData->XqPrice;
			pBigTable->StartDate = pRefData->StartDate;
			pBigTable->EndDate = pRefData->EndDate;
			pBigTable->DeliveryDate = pRefData->DeliveryDate;
			pBigTable->ExpireDate = pRefData->ExpireDate;
			pBigTable->PriceTick = pRefData->PriceTick;
		}
	}
};

struct MappingDLFuture_SnapLF2QuoSnapData : public InnerRecord { MappingDLFuture_SnapLF2QuoSnapData() : InnerRecord( 104, sizeof(tagDLFutureSnapData_LF104), QUO_MARKET_DCE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureSnapData_LF104*		pSnapData = (tagDLFutureSnapData_LF104*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Open = pSnapData->Open;
			pBigTable->Close = pSnapData->Close;
			pBigTable->PreClose = pSnapData->PreClose;
			pBigTable->UpperPrice = pSnapData->UpperPrice;
			pBigTable->LowerPrice = pSnapData->LowerPrice;
			pBigTable->SettlePrice = pSnapData->SettlePrice;
			pBigTable->PreSettlePrice = pSnapData->PreSettlePrice;
			pBigTable->PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingDLFuture_SnapHF2QuoSnapData : public InnerRecord { MappingDLFuture_SnapHF2QuoSnapData() : InnerRecord( 105, sizeof(tagDLFutureSnapData_HF105), QUO_MARKET_DCE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureSnapData_HF105*		pSnapData = (tagDLFutureSnapData_HF105*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Now = pSnapData->Now;
			pBigTable->High = pSnapData->High;
			pBigTable->Low = pSnapData->Low;
			pBigTable->Amount = pSnapData->Amount;
			pBigTable->Volume = pSnapData->Volume;
			pBigTable->Position = pSnapData->Position;
		}
	}
};

struct MappingDLFuture_BuySell2QuoSnapData : public InnerRecord { MappingDLFuture_BuySell2QuoSnapData() : InnerRecord( 106, sizeof(tagDLFutureSnapBuySell_HF106), QUO_MARKET_DCE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLFutureSnapBuySell_HF106*	pSnapData = (tagDLFutureSnapBuySell_HF106*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			::memcpy( pBigTable->Buy, pSnapData->Buy, sizeof(pSnapData->Buy) );
			::memcpy( pBigTable->Sell, pSnapData->Sell, sizeof(pSnapData->Sell) );
		}
	}
};

///< 上海商品期货
struct MappingSHFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHFuture_MkInfo2QuoMarketInfo() : InnerRecord( 107, sizeof(tagSHFutureMarketInfo_LF107), QUO_MARKET_SHFE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureMarketInfo_LF107*	pMkInfo = (tagSHFutureMarketInfo_LF107*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketDate = pMkInfo->MarketDate;
			pBigTable->MarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->WareCount = pMkInfo->KindCount;
			pBigTable->WareCount = pMkInfo->WareCount;
			pBigTable->PeriodsCount = pMkInfo->PeriodsCount;
			::memcpy( pBigTable->MarketPeriods, pMkInfo->MarketPeriods, sizeof(pBigTable->MarketPeriods) );
		}
	}
};

struct MappingSHFuture_Kind2QuoCategory : public InnerRecord { MappingSHFuture_Kind2QuoCategory() : InnerRecord( 108, sizeof(tagSHFutureKindDetail_LF108), QUO_MARKET_SHFE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureKindDetail_LF108*	pKind = (tagSHFutureKindDetail_LF108*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingSHFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHFuture_MkStatus2QuoMarketInfo() : InnerRecord( 109, sizeof(tagSHFutureMarketStatus_HF109), QUO_MARKET_SHFE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureMarketStatus_HF109*	pMkStatus = (tagSHFutureMarketStatus_HF109*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingSHFuture_Reference2QuoReference : public InnerRecord { MappingSHFuture_Reference2QuoReference() : InnerRecord( 110, sizeof(tagSHFutureReferenceData_LF110), QUO_MARKET_SHFE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureReferenceData_LF110*	pRefData = (tagSHFutureReferenceData_LF110*)pMessagePtr;
			tagQuoReferenceData*			pBigTable = (tagQuoReferenceData*)&(m_objUnionData.ReferenceData_3);

			::memcpy( pBigTable->Code, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->Name, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->Kind = pRefData->Kind;
			pBigTable->DerivativeType = pRefData->DerivativeType;
			pBigTable->LotSize = pRefData->LotSize;
			::memcpy( pBigTable->UnderlyingCode, pRefData->UnderlyingCode, sizeof(pRefData->UnderlyingCode) );
			pBigTable->ContractMult = pRefData->ContractMult;
			pBigTable->XqPrice = pRefData->XqPrice;
			pBigTable->StartDate = pRefData->StartDate;
			pBigTable->EndDate = pRefData->EndDate;
			pBigTable->DeliveryDate = pRefData->DeliveryDate;
			pBigTable->ExpireDate = pRefData->ExpireDate;
			pBigTable->PriceTick = pRefData->PriceTick;
		}
	}
};

struct MappingSHFuture_SnapLF2QuoSnapData : public InnerRecord { MappingSHFuture_SnapLF2QuoSnapData() : InnerRecord( 111, sizeof(tagSHFutureSnapData_LF111), QUO_MARKET_SHFE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureSnapData_LF111*		pSnapData = (tagSHFutureSnapData_LF111*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Open = pSnapData->Open;
			pBigTable->Close = pSnapData->Close;
			pBigTable->PreClose = pSnapData->PreClose;
			pBigTable->UpperPrice = pSnapData->UpperPrice;
			pBigTable->LowerPrice = pSnapData->LowerPrice;
			pBigTable->SettlePrice = pSnapData->SettlePrice;
			pBigTable->PreSettlePrice = pSnapData->PreSettlePrice;
			pBigTable->PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingSHFuture_SnapHF2QuoSnapData : public InnerRecord { MappingSHFuture_SnapHF2QuoSnapData() : InnerRecord( 112, sizeof(tagSHFutureSnapData_HF112), QUO_MARKET_SHFE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureSnapData_HF112*		pSnapData = (tagSHFutureSnapData_HF112*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Now = pSnapData->Now;
			pBigTable->High = pSnapData->High;
			pBigTable->Low = pSnapData->Low;
			pBigTable->Amount = pSnapData->Amount;
			pBigTable->Volume = pSnapData->Volume;
			pBigTable->Position = pSnapData->Position;
		}
	}
};

struct MappingSHFuture_BuySell2QuoSnapData : public InnerRecord { MappingSHFuture_BuySell2QuoSnapData() : InnerRecord( 113, sizeof(tagSHFutureSnapBuySell_HF113), QUO_MARKET_SHFE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureSnapBuySell_HF113*	pSnapData = (tagSHFutureSnapBuySell_HF113*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			::memcpy( pBigTable->Buy, pSnapData->Buy, sizeof(pSnapData->Buy) );
			::memcpy( pBigTable->Sell, pSnapData->Sell, sizeof(pSnapData->Sell) );
		}
	}
};

///< 郑州商品期货
struct MappingZZFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingZZFuture_MkInfo2QuoMarketInfo() : InnerRecord( 114, sizeof(tagZZFutureMarketInfo_LF114), QUO_MARKET_CZCE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureMarketInfo_LF114*	pMkInfo = (tagZZFutureMarketInfo_LF114*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketDate = pMkInfo->MarketDate;
			pBigTable->MarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->WareCount = pMkInfo->KindCount;
			pBigTable->WareCount = pMkInfo->WareCount;
			pBigTable->PeriodsCount = pMkInfo->PeriodsCount;
			::memcpy( pBigTable->MarketPeriods, pMkInfo->MarketPeriods, sizeof(pBigTable->MarketPeriods) );
		}
	}
};

struct MappingZZFuture_Kind2QuoCategory : public InnerRecord { MappingZZFuture_Kind2QuoCategory() : InnerRecord( 115, sizeof(tagZZFutureKindDetail_LF115), QUO_MARKET_CZCE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureKindDetail_LF115*	pKind = (tagZZFutureKindDetail_LF115*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingZZFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingZZFuture_MkStatus2QuoMarketInfo() : InnerRecord( 116, sizeof(tagZZFutureMarketStatus_HF116), QUO_MARKET_CZCE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureMarketStatus_HF116*	pMkStatus = (tagZZFutureMarketStatus_HF116*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingZZFuture_Reference2QuoReference : public InnerRecord { MappingZZFuture_Reference2QuoReference() : InnerRecord( 117, sizeof(tagZZFutureReferenceData_LF117), QUO_MARKET_CZCE*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureReferenceData_LF117*	pRefData = (tagZZFutureReferenceData_LF117*)pMessagePtr;
			tagQuoReferenceData*			pBigTable = (tagQuoReferenceData*)&(m_objUnionData.ReferenceData_3);

			::memcpy( pBigTable->Code, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->Name, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->Kind = pRefData->Kind;
			pBigTable->DerivativeType = pRefData->DerivativeType;
			pBigTable->LotSize = pRefData->LotSize;
			::memcpy( pBigTable->UnderlyingCode, pRefData->UnderlyingCode, sizeof(pRefData->UnderlyingCode) );
			pBigTable->ContractMult = pRefData->ContractMult;
			pBigTable->XqPrice = pRefData->XqPrice;
			pBigTable->StartDate = pRefData->StartDate;
			pBigTable->EndDate = pRefData->EndDate;
			pBigTable->DeliveryDate = pRefData->DeliveryDate;
			pBigTable->ExpireDate = pRefData->ExpireDate;
			pBigTable->PriceTick = pRefData->PriceTick;
		}
	}
};

struct MappingZZFuture_SnapLF2QuoSnapData : public InnerRecord { MappingZZFuture_SnapLF2QuoSnapData() : InnerRecord( 118, sizeof(tagZZFutureSnapData_LF118), QUO_MARKET_CZCE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureSnapData_LF118*		pSnapData = (tagZZFutureSnapData_LF118*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Open = pSnapData->Open;
			pBigTable->Close = pSnapData->Close;
			pBigTable->PreClose = pSnapData->PreClose;
			pBigTable->UpperPrice = pSnapData->UpperPrice;
			pBigTable->LowerPrice = pSnapData->LowerPrice;
			pBigTable->SettlePrice = pSnapData->SettlePrice;
			pBigTable->PreSettlePrice = pSnapData->PreSettlePrice;
			pBigTable->PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingZZFuture_SnapHF2QuoSnapData : public InnerRecord { MappingZZFuture_SnapHF2QuoSnapData() : InnerRecord( 119, sizeof(tagZZFutureSnapData_HF119), QUO_MARKET_CZCE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureSnapData_HF119*		pSnapData = (tagZZFutureSnapData_HF119*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Now = pSnapData->Now;
			pBigTable->High = pSnapData->High;
			pBigTable->Low = pSnapData->Low;
			pBigTable->Amount = pSnapData->Amount;
			pBigTable->Volume = pSnapData->Volume;
			pBigTable->Position = pSnapData->Position;
		}
	}
};

struct MappingZZFuture_BuySell2QuoSnapData : public InnerRecord { MappingZZFuture_BuySell2QuoSnapData() : InnerRecord( 120, sizeof(tagZZFutureSnapBuySell_HF120), QUO_MARKET_CZCE*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZFutureSnapBuySell_HF120*	pSnapData = (tagZZFutureSnapBuySell_HF120*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			::memcpy( pBigTable->Buy, pSnapData->Buy, sizeof(pSnapData->Buy) );
			::memcpy( pBigTable->Sell, pSnapData->Sell, sizeof(pSnapData->Sell) );
		}
	}
};

///< 大连商品期权
struct MappingDLOption_MkInfo2QuoMarketInfo : public InnerRecord { MappingDLOption_MkInfo2QuoMarketInfo() : InnerRecord( 128, sizeof(tagDLOptionMarketInfo_LF128), QUO_MARKET_DCEOPT*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionMarketInfo_LF128*	pMkInfo = (tagDLOptionMarketInfo_LF128*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketDate = pMkInfo->MarketDate;
			pBigTable->MarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->WareCount = pMkInfo->KindCount;
			pBigTable->WareCount = pMkInfo->WareCount;
			pBigTable->PeriodsCount = pMkInfo->PeriodsCount;
			::memcpy( pBigTable->MarketPeriods, pMkInfo->MarketPeriods, sizeof(pBigTable->MarketPeriods) );
		}
	}
};

struct MappingDLOption_Kind2QuoCategory : public InnerRecord { MappingDLOption_Kind2QuoCategory() : InnerRecord( 129, sizeof(tagDLOptionKindDetail_LF129), QUO_MARKET_DCEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionKindDetail_LF129*	pKind = (tagDLOptionKindDetail_LF129*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingDLOption_MkStatus2QuoMarketInfo : public InnerRecord { MappingDLOption_MkStatus2QuoMarketInfo() : InnerRecord( 130, sizeof(tagDLOptionMarketStatus_HF130), QUO_MARKET_DCEOPT*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionMarketStatus_HF130*	pMkStatus = (tagDLOptionMarketStatus_HF130*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingDLOption_Reference2QuoReference : public InnerRecord { MappingDLOption_Reference2QuoReference() : InnerRecord( 131, sizeof(tagDLOptionReferenceData_LF131), QUO_MARKET_DCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionReferenceData_LF131*	pRefData = (tagDLOptionReferenceData_LF131*)pMessagePtr;
			tagQuoReferenceData*			pBigTable = (tagQuoReferenceData*)&(m_objUnionData.ReferenceData_3);

			::memcpy( pBigTable->Code, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->Name, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->Kind = pRefData->Kind;
			pBigTable->DerivativeType = pRefData->DerivativeType;
			pBigTable->LotSize = pRefData->LotSize;
			::memcpy( pBigTable->UnderlyingCode, pRefData->UnderlyingCode, sizeof(pRefData->UnderlyingCode) );
			pBigTable->ContractMult = pRefData->ContractMult;
			pBigTable->XqPrice = pRefData->XqPrice;
			pBigTable->StartDate = pRefData->StartDate;
			pBigTable->EndDate = pRefData->EndDate;
			pBigTable->DeliveryDate = pRefData->DeliveryDate;
			pBigTable->ExpireDate = pRefData->ExpireDate;
			pBigTable->PriceTick = pRefData->PriceTick;
		}
	}
};

struct MappingDLOption_SnapLF2QuoSnapData : public InnerRecord { MappingDLOption_SnapLF2QuoSnapData() : InnerRecord( 132, sizeof(tagDLOptionSnapData_LF132), QUO_MARKET_DCEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionSnapData_LF132*		pSnapData = (tagDLOptionSnapData_LF132*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Open = pSnapData->Open;
			pBigTable->Close = pSnapData->Close;
			pBigTable->PreClose = pSnapData->PreClose;
			pBigTable->UpperPrice = pSnapData->UpperPrice;
			pBigTable->LowerPrice = pSnapData->LowerPrice;
			pBigTable->SettlePrice = pSnapData->SettlePrice;
			pBigTable->PreSettlePrice = pSnapData->PreSettlePrice;
			pBigTable->PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingDLOption_SnapHF2QuoSnapData : public InnerRecord { MappingDLOption_SnapHF2QuoSnapData() : InnerRecord( 133, sizeof(tagDLOptionSnapData_HF133), QUO_MARKET_DCEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionSnapData_HF133*		pSnapData = (tagDLOptionSnapData_HF133*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Now = pSnapData->Now;
			pBigTable->High = pSnapData->High;
			pBigTable->Low = pSnapData->Low;
			pBigTable->Amount = pSnapData->Amount;
			pBigTable->Volume = pSnapData->Volume;
			pBigTable->Position = pSnapData->Position;
		}
	}
};

struct MappingDLOption_BuySell2QuoSnapData : public InnerRecord { MappingDLOption_BuySell2QuoSnapData() : InnerRecord( 134, sizeof(tagDLOptionSnapBuySell_HF134), QUO_MARKET_DCEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagDLOptionSnapBuySell_HF134*	pSnapData = (tagDLOptionSnapBuySell_HF134*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			::memcpy( pBigTable->Buy, pSnapData->Buy, sizeof(pSnapData->Buy) );
			::memcpy( pBigTable->Sell, pSnapData->Sell, sizeof(pSnapData->Sell) );
		}
	}
};

///< 上海商品期权
struct MappingSHOption_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHOption_MkInfo2QuoMarketInfo() : InnerRecord( 135, sizeof(tagSHOptionMarketInfo_LF135), QUO_MARKET_SHFEOPT*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionMarketInfo_LF135*	pMkInfo = (tagSHOptionMarketInfo_LF135*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketDate = pMkInfo->MarketDate;
			pBigTable->MarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->WareCount = pMkInfo->KindCount;
			pBigTable->WareCount = pMkInfo->WareCount;
			pBigTable->PeriodsCount = pMkInfo->PeriodsCount;
			::memcpy( pBigTable->MarketPeriods, pMkInfo->MarketPeriods, sizeof(pBigTable->MarketPeriods) );
		}
	}
};

struct MappingSHOption_Kind2QuoCategory : public InnerRecord { MappingSHOption_Kind2QuoCategory() : InnerRecord( 136, sizeof(tagSHOptionKindDetail_LF136), QUO_MARKET_SHFEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionKindDetail_LF136*	pKind = (tagSHOptionKindDetail_LF136*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingSHOption_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHOption_MkStatus2QuoMarketInfo() : InnerRecord( 137, sizeof(tagSHOptionMarketStatus_HF137), QUO_MARKET_SHFEOPT*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionMarketStatus_HF137*	pMkStatus = (tagSHOptionMarketStatus_HF137*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingSHOption_Reference2QuoReference : public InnerRecord { MappingSHOption_Reference2QuoReference() : InnerRecord( 138, sizeof(tagSHOptionReferenceData_LF138), QUO_MARKET_SHFEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionReferenceData_LF138*	pRefData = (tagSHOptionReferenceData_LF138*)pMessagePtr;
			tagQuoReferenceData*			pBigTable = (tagQuoReferenceData*)&(m_objUnionData.ReferenceData_3);

			::memcpy( pBigTable->Code, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->Name, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->Kind = pRefData->Kind;
			pBigTable->DerivativeType = pRefData->DerivativeType;
			pBigTable->LotSize = pRefData->LotSize;
			::memcpy( pBigTable->UnderlyingCode, pRefData->UnderlyingCode, sizeof(pRefData->UnderlyingCode) );
			pBigTable->ContractMult = pRefData->ContractMult;
			pBigTable->XqPrice = pRefData->XqPrice;
			pBigTable->StartDate = pRefData->StartDate;
			pBigTable->EndDate = pRefData->EndDate;
			pBigTable->DeliveryDate = pRefData->DeliveryDate;
			pBigTable->ExpireDate = pRefData->ExpireDate;
			pBigTable->PriceTick = pRefData->PriceTick;
		}
	}
};

struct MappingSHOption_SnapLF2QuoSnapData : public InnerRecord { MappingSHOption_SnapLF2QuoSnapData() : InnerRecord( 139, sizeof(tagSHOptionSnapData_LF139), QUO_MARKET_SHFEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionSnapData_LF139*		pSnapData = (tagSHOptionSnapData_LF139*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Open = pSnapData->Open;
			pBigTable->Close = pSnapData->Close;
			pBigTable->PreClose = pSnapData->PreClose;
			pBigTable->UpperPrice = pSnapData->UpperPrice;
			pBigTable->LowerPrice = pSnapData->LowerPrice;
			pBigTable->SettlePrice = pSnapData->SettlePrice;
			pBigTable->PreSettlePrice = pSnapData->PreSettlePrice;
			pBigTable->PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingSHOption_SnapHF2QuoSnapData : public InnerRecord { MappingSHOption_SnapHF2QuoSnapData() : InnerRecord( 140, sizeof(tagSHOptionSnapData_HF140), QUO_MARKET_SHFEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionSnapData_HF140*		pSnapData = (tagSHOptionSnapData_HF140*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Now = pSnapData->Now;
			pBigTable->High = pSnapData->High;
			pBigTable->Low = pSnapData->Low;
			pBigTable->Amount = pSnapData->Amount;
			pBigTable->Volume = pSnapData->Volume;
			pBigTable->Position = pSnapData->Position;
		}
	}
};

struct MappingSHOption_BuySell2QuoSnapData : public InnerRecord { MappingSHOption_BuySell2QuoSnapData() : InnerRecord( 141, sizeof(tagSHOptionSnapBuySell_HF141), QUO_MARKET_SHFEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHOptionSnapBuySell_HF141*	pSnapData = (tagSHOptionSnapBuySell_HF141*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			::memcpy( pBigTable->Buy, pSnapData->Buy, sizeof(pSnapData->Buy) );
			::memcpy( pBigTable->Sell, pSnapData->Sell, sizeof(pSnapData->Sell) );
		}
	}
};

///< 郑州商品期权
struct MappingZZOption_MkInfo2QuoMarketInfo : public InnerRecord { MappingZZOption_MkInfo2QuoMarketInfo() : InnerRecord( 142, sizeof(tagZZOptionMarketInfo_LF142), QUO_MARKET_CZCEOPT*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionMarketInfo_LF142*	pMkInfo = (tagZZOptionMarketInfo_LF142*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketDate = pMkInfo->MarketDate;
			pBigTable->MarketID = (enum QUO_MARKET_ID)pMkInfo->MarketID;
			pBigTable->WareCount = pMkInfo->KindCount;
			pBigTable->WareCount = pMkInfo->WareCount;
			pBigTable->PeriodsCount = pMkInfo->PeriodsCount;
			::memcpy( pBigTable->MarketPeriods, pMkInfo->MarketPeriods, sizeof(pBigTable->MarketPeriods) );
		}
	}
};

struct MappingZZOption_Kind2QuoCategory : public InnerRecord { MappingZZOption_Kind2QuoCategory() : InnerRecord( 143, sizeof(tagZZOptionKindDetail_LF143), QUO_MARKET_CZCEOPT*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionKindDetail_LF143*	pKind = (tagZZOptionKindDetail_LF143*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingZZOption_MkStatus2QuoMarketInfo : public InnerRecord { MappingZZOption_MkStatus2QuoMarketInfo() : InnerRecord( 144, sizeof(tagZZOptionMarketStatus_HF144), QUO_MARKET_CZCEOPT*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionMarketStatus_HF144*	pMkStatus = (tagZZOptionMarketStatus_HF144*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingZZOption_Reference2QuoReference : public InnerRecord { MappingZZOption_Reference2QuoReference() : InnerRecord( 145, sizeof(tagZZOptionReferenceData_LF145), QUO_MARKET_CZCEOPT*100+3 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionReferenceData_LF145*	pRefData = (tagZZOptionReferenceData_LF145*)pMessagePtr;
			tagQuoReferenceData*			pBigTable = (tagQuoReferenceData*)&(m_objUnionData.ReferenceData_3);

			::memcpy( pBigTable->Code, pRefData->Code, sizeof(pRefData->Code) );
			::memcpy( pBigTable->Name, pRefData->Name, sizeof(pRefData->Name) );
			pBigTable->Kind = pRefData->Kind;
			pBigTable->DerivativeType = pRefData->DerivativeType;
			pBigTable->LotSize = pRefData->LotSize;
			::memcpy( pBigTable->UnderlyingCode, pRefData->UnderlyingCode, sizeof(pRefData->UnderlyingCode) );
			pBigTable->ContractMult = pRefData->ContractMult;
			pBigTable->XqPrice = pRefData->XqPrice;
			pBigTable->StartDate = pRefData->StartDate;
			pBigTable->EndDate = pRefData->EndDate;
			pBigTable->DeliveryDate = pRefData->DeliveryDate;
			pBigTable->ExpireDate = pRefData->ExpireDate;
			pBigTable->PriceTick = pRefData->PriceTick;
		}
	}
};

struct MappingZZOption_SnapLF2QuoSnapData : public InnerRecord { MappingZZOption_SnapLF2QuoSnapData() : InnerRecord( 146, sizeof(tagZZOptionSnapData_LF146), QUO_MARKET_CZCEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionSnapData_LF146*		pSnapData = (tagZZOptionSnapData_LF146*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Open = pSnapData->Open;
			pBigTable->Close = pSnapData->Close;
			pBigTable->PreClose = pSnapData->PreClose;
			pBigTable->UpperPrice = pSnapData->UpperPrice;
			pBigTable->LowerPrice = pSnapData->LowerPrice;
			pBigTable->SettlePrice = pSnapData->SettlePrice;
			pBigTable->PreSettlePrice = pSnapData->PreSettlePrice;
			pBigTable->PreOpenInterest = pSnapData->PreOpenInterest;
		}
	}
};

struct MappingZZOption_SnapHF2QuoSnapData : public InnerRecord { MappingZZOption_SnapHF2QuoSnapData() : InnerRecord( 147, sizeof(tagZZOptionSnapData_HF147), QUO_MARKET_CZCEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionSnapData_HF147*		pSnapData = (tagZZOptionSnapData_HF147*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			pBigTable->Now = pSnapData->Now;
			pBigTable->High = pSnapData->High;
			pBigTable->Low = pSnapData->Low;
			pBigTable->Amount = pSnapData->Amount;
			pBigTable->Volume = pSnapData->Volume;
			pBigTable->Position = pSnapData->Position;
		}
	}
};

struct MappingZZOption_BuySell2QuoSnapData : public InnerRecord { MappingZZOption_BuySell2QuoSnapData() : InnerRecord( 148, sizeof(tagZZOptionSnapBuySell_HF148), QUO_MARKET_CZCEOPT*100+4 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagZZOptionSnapBuySell_HF148*	pSnapData = (tagZZOptionSnapBuySell_HF148*)pMessagePtr;
			tagQuoSnapData*					pBigTable = (tagQuoSnapData*)&(m_objUnionData.SnapData_4);

			::memcpy( pBigTable->Code, pSnapData->Code, sizeof(pSnapData->Code) );
			::memcpy( pBigTable->Buy, pSnapData->Buy, sizeof(pSnapData->Buy) );
			::memcpy( pBigTable->Sell, pSnapData->Sell, sizeof(pSnapData->Sell) );
		}
	}
};

///< -----------------------------------------------------------------------------


const unsigned int TableFillerRegister::s_nRegisterTableSize = 2048;
std::vector<InnerRecord*> TableFillerRegister::s_vctRegisterTable;


TableFillerRegister::TableFillerRegister()
{
	s_vctRegisterTable.resize( s_nRegisterTableSize );		///< 预留2048个messageid映射器地址
}

int TableFillerRegister::Initialize()
{
	///< ------------- 建立行情数据结构到内存数据结构的映射表 --------------
	///< 大连商品期货
	static MappingDLFuture_MkInfo2QuoMarketInfo		objDLFuture100Cast;
	static MappingDLFuture_Kind2QuoCategory			objDLFuture101Cast;
	static MappingDLFuture_MkStatus2QuoMarketInfo	objDLFuture102Cast;
	static MappingDLFuture_Reference2QuoReference	objDLFuture103Cast;
	static MappingDLFuture_SnapLF2QuoSnapData		objDLFuture104Cast;
	static MappingDLFuture_SnapHF2QuoSnapData		objDLFuture105Cast;
	static MappingDLFuture_BuySell2QuoSnapData		objDLFuture106Cast;
	if( GetRegister().Register( objDLFuture100Cast ) != 0 )	return -1;
	if( GetRegister().Register( objDLFuture101Cast ) != 0 )	return -2;
	if( GetRegister().Register( objDLFuture102Cast ) != 0 )	return -3;
	if( GetRegister().Register( objDLFuture103Cast ) != 0 )	return -4;
	if( GetRegister().Register( objDLFuture104Cast ) != 0 )	return -5;
	if( GetRegister().Register( objDLFuture105Cast ) != 0 )	return -6;
	if( GetRegister().Register( objDLFuture106Cast ) != 0 )	return -7;

	///< 上海商品期货
	static MappingSHFuture_MkInfo2QuoMarketInfo		objSHFuture107Cast;
	static MappingSHFuture_Kind2QuoCategory			objSHFuture108Cast;
	static MappingSHFuture_MkStatus2QuoMarketInfo	objSHFuture109Cast;
	static MappingSHFuture_Reference2QuoReference	objSHFuture110Cast;
	static MappingSHFuture_SnapLF2QuoSnapData		objSHFuture111Cast;
	static MappingSHFuture_SnapHF2QuoSnapData		objSHFuture112Cast;
	static MappingSHFuture_BuySell2QuoSnapData		objSHFuture113Cast;
	if( GetRegister().Register( objSHFuture107Cast ) != 0 )	return -8;
	if( GetRegister().Register( objSHFuture108Cast ) != 0 )	return -9;
	if( GetRegister().Register( objSHFuture109Cast ) != 0 )	return -10;
	if( GetRegister().Register( objSHFuture110Cast ) != 0 )	return -11;
	if( GetRegister().Register( objSHFuture111Cast ) != 0 )	return -12;
	if( GetRegister().Register( objSHFuture112Cast ) != 0 )	return -13;
	if( GetRegister().Register( objSHFuture113Cast ) != 0 )	return -14;

	///< 郑州商品期货
	static MappingZZFuture_MkInfo2QuoMarketInfo		objZZFuture114Cast;
	static MappingZZFuture_Kind2QuoCategory			objZZFuture115Cast;
	static MappingZZFuture_MkStatus2QuoMarketInfo	objZZFuture116Cast;
	static MappingZZFuture_Reference2QuoReference	objZZFuture117Cast;
	static MappingZZFuture_SnapLF2QuoSnapData		objZZFuture118Cast;
	static MappingZZFuture_SnapHF2QuoSnapData		objZZFuture119Cast;
	static MappingZZFuture_BuySell2QuoSnapData		objZZFuture120Cast;
	if( GetRegister().Register( objZZFuture114Cast ) != 0 )	return -15;
	if( GetRegister().Register( objZZFuture115Cast ) != 0 )	return -16;
	if( GetRegister().Register( objZZFuture116Cast ) != 0 )	return -17;
	if( GetRegister().Register( objZZFuture117Cast ) != 0 )	return -18;
	if( GetRegister().Register( objZZFuture118Cast ) != 0 )	return -19;
	if( GetRegister().Register( objZZFuture119Cast ) != 0 )	return -20;
	if( GetRegister().Register( objZZFuture120Cast ) != 0 )	return -21;

	///< 大连商品期权
	static MappingDLOption_MkInfo2QuoMarketInfo		objDLOption128Cast;
	static MappingDLOption_Kind2QuoCategory			objDLOption129Cast;
	static MappingDLOption_MkStatus2QuoMarketInfo	objDLOption130Cast;
	static MappingDLOption_Reference2QuoReference	objDLOption131Cast;
	static MappingDLOption_SnapLF2QuoSnapData		objDLOption132Cast;
	static MappingDLOption_SnapHF2QuoSnapData		objDLOption133Cast;
	static MappingDLOption_BuySell2QuoSnapData		objDLOption134Cast;
	if( GetRegister().Register( objDLOption128Cast ) != 0 )	return -22;
	if( GetRegister().Register( objDLOption129Cast ) != 0 )	return -23;
	if( GetRegister().Register( objDLOption130Cast ) != 0 )	return -24;
	if( GetRegister().Register( objDLOption131Cast ) != 0 )	return -25;
	if( GetRegister().Register( objDLOption132Cast ) != 0 )	return -26;
	if( GetRegister().Register( objDLOption133Cast ) != 0 )	return -27;
	if( GetRegister().Register( objDLOption134Cast ) != 0 )	return -28;

	///< 上海商品期权
	static MappingSHOption_MkInfo2QuoMarketInfo		objSHOption135Cast;
	static MappingSHOption_Kind2QuoCategory			objSHOption136Cast;
	static MappingSHOption_MkStatus2QuoMarketInfo	objSHOption137Cast;
	static MappingSHOption_Reference2QuoReference	objSHOption138Cast;
	static MappingSHOption_SnapLF2QuoSnapData		objSHOption139Cast;
	static MappingSHOption_SnapHF2QuoSnapData		objSHOption140Cast;
	static MappingSHOption_BuySell2QuoSnapData		objSHOption141Cast;
	if( GetRegister().Register( objSHOption135Cast ) != 0 )	return -28;
	if( GetRegister().Register( objSHOption136Cast ) != 0 )	return -29;
	if( GetRegister().Register( objSHOption137Cast ) != 0 )	return -30;
	if( GetRegister().Register( objSHOption138Cast ) != 0 )	return -31;
	if( GetRegister().Register( objSHOption139Cast ) != 0 )	return -32;
	if( GetRegister().Register( objSHOption140Cast ) != 0 )	return -33;
	if( GetRegister().Register( objSHOption141Cast ) != 0 )	return -34;

	///< 郑州商品期权
	static MappingZZOption_MkInfo2QuoMarketInfo		objZZOption142Cast;
	static MappingZZOption_Kind2QuoCategory			objZZOption143Cast;
	static MappingZZOption_MkStatus2QuoMarketInfo	objZZOption144Cast;
	static MappingZZOption_Reference2QuoReference	objZZOption145Cast;
	static MappingZZOption_SnapLF2QuoSnapData		objZZOption146Cast;
	static MappingZZOption_SnapHF2QuoSnapData		objZZOption147Cast;
	static MappingZZOption_BuySell2QuoSnapData		objZZOption148Cast;
	if( GetRegister().Register( objZZOption142Cast ) != 0 )	return -35;
	if( GetRegister().Register( objZZOption143Cast ) != 0 )	return -36;
	if( GetRegister().Register( objZZOption144Cast ) != 0 )	return -37;
	if( GetRegister().Register( objZZOption145Cast ) != 0 )	return -38;
	if( GetRegister().Register( objZZOption146Cast ) != 0 )	return -39;
	if( GetRegister().Register( objZZOption147Cast ) != 0 )	return -40;
	if( GetRegister().Register( objZZOption148Cast ) != 0 )	return -41;

	return 0;
}

TableFillerRegister&	TableFillerRegister::GetRegister()
{
	static TableFillerRegister		obj;

	return obj;
}

int TableFillerRegister::Register( InnerRecord& refRecordFiller )
{
	unsigned int			nMessageID = refRecordFiller.GetMessageID();
	unsigned int			nPos = nMessageID % s_nRegisterTableSize;
	InnerRecord*			pInnerRecord = s_vctRegisterTable[nPos];

	if( NULL != pInnerRecord )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::Register() : filler has existed in register table, msgid(%d)", nMessageID );
		return -1;
	}

	s_vctRegisterTable[nPos] = &refRecordFiller;
	DataIOEngine::GetEngineObj().WriteInfo( "TableFillerRegister::Register() : Building Mapping Table..., MessageID(%u-->%u), Size(%u-->%u)"
											, nMessageID, refRecordFiller.GetBigTableID(), refRecordFiller.GetMessageLength(), refRecordFiller.GetBigTableWidth() );

	if( nMessageID == 0 || refRecordFiller.GetBigTableID() == 0 || refRecordFiller.GetMessageLength() == 0 || refRecordFiller.GetBigTableWidth() == 0 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::Register() : invalid parameter : MessageID(%u-->%u), Size(%u-->%u)"
													, nMessageID, refRecordFiller.GetBigTableID(), refRecordFiller.GetMessageLength(), refRecordFiller.GetBigTableWidth() );
		return -2;
	}

	return 0;
}

InnerRecord* TableFillerRegister::PrepareNewTableBlock( unsigned int nMessageID, const char* pMsgPtr, unsigned int nMsgLen )
{
	unsigned int			nPos = nMessageID % s_nRegisterTableSize;
	InnerRecord*			pInnerRecord = s_vctRegisterTable[nPos];

	if( NULL == pMsgPtr || nMsgLen < 20 )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : invalid parameters, MessageID(%d), MessagePtr(%x), MessageLength(%d)", nMessageID, pMsgPtr, nMsgLen );

		return NULL;
	}

	if( NULL != pInnerRecord )
	{
		if( pInnerRecord->GetMessageID() != nMessageID || pInnerRecord->GetMessageLength() != nMsgLen )
		{
			DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : invalid parameters, MessageID(%d!=%d), MessageLength(%d!=%d)"
													, nMessageID, pInnerRecord->GetMessageID(), nMsgLen, pInnerRecord->GetMessageLength() );
			return NULL;
		}

		::memset( pInnerRecord->GetBigTableRecordPtr(), 0, pInnerRecord->GetBigTableWidth() );	///< 先清一把，腾出空间
		::memcpy( pInnerRecord->GetBigTableRecordPtr(), pMsgPtr, 20 );							///< 再把Code先copy进去

		return pInnerRecord;
	}

	DataIOEngine::GetEngineObj().WriteWarning( "TableFillerRegister::PrepareNewTableBlock() : Invalid MessageID(%d), MessageLength(%d)", nMessageID, nMsgLen );

	return NULL;
}







