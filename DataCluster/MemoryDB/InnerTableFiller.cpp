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
struct MappingSHFuture_MkInfo2QuoMarketInfo : public InnerRecord { MappingSHFuture_MkInfo2QuoMarketInfo() : InnerRecord( 107, sizeof(tagSHFutureMarketInfo_LF107), QUO_MARKET_DCE*100+1 ) {}
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

struct MappingSHFuture_Kind2QuoCategory : public InnerRecord { MappingSHFuture_Kind2QuoCategory() : InnerRecord( 108, sizeof(tagSHFutureKindDetail_LF108), QUO_MARKET_DCE*100+2 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureKindDetail_LF108*	pKind = (tagSHFutureKindDetail_LF108*)pMessagePtr;
			tagQuoCategory*					pBigTable = (tagQuoCategory*)&(m_objUnionData.CategoryData_2);

			pBigTable->WareCount= pKind->WareCount;
			::memcpy( pBigTable->KindName, pKind->KindName, sizeof(pBigTable->KindName) );
		}
	}
};

struct MappingSHFuture_MkStatus2QuoMarketInfo : public InnerRecord { MappingSHFuture_MkStatus2QuoMarketInfo() : InnerRecord( 109, sizeof(tagSHFutureMarketStatus_HF109), QUO_MARKET_DCE*100+1 ) {}
	void	FillMessage2BigTableRecord(  char* pMessagePtr )	{
		if( NULL != pMessagePtr )	{
			tagSHFutureMarketStatus_HF109*	pMkStatus = (tagSHFutureMarketStatus_HF109*)pMessagePtr;
			tagQuoMarketInfo*				pBigTable = (tagQuoMarketInfo*)&(m_objUnionData.MarketData_1);

			pBigTable->MarketTime = pMkStatus->MarketTime;
			pBigTable->TradingPhaseCode[0] = pMkStatus->MarketStatus;
		}
	}
};

struct MappingSHFuture_Reference2QuoReference : public InnerRecord { MappingSHFuture_Reference2QuoReference() : InnerRecord( 110, sizeof(tagSHFutureReferenceData_LF110), QUO_MARKET_DCE*100+3 ) {}
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

struct MappingSHFuture_SnapLF2QuoSnapData : public InnerRecord { MappingSHFuture_SnapLF2QuoSnapData() : InnerRecord( 111, sizeof(tagSHFutureSnapData_LF111), QUO_MARKET_DCE*100+4 ) {}
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

struct MappingSHFuture_SnapHF2QuoSnapData : public InnerRecord { MappingSHFuture_SnapHF2QuoSnapData() : InnerRecord( 112, sizeof(tagSHFutureSnapData_HF112), QUO_MARKET_DCE*100+4 ) {}
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

struct MappingSHFuture_BuySell2QuoSnapData : public InnerRecord { MappingSHFuture_BuySell2QuoSnapData() : InnerRecord( 113, sizeof(tagSHFutureSnapBuySell_HF113), QUO_MARKET_DCE*100+4 ) {}
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


///< -----------------------------------------------------------------------------


const unsigned int TableFillerRegister::s_nRegisterTableSize = 2048;
std::vector<InnerRecord*> TableFillerRegister::s_vctRegisterTable;


TableFillerRegister::TableFillerRegister()
{
	s_vctRegisterTable.resize( s_nRegisterTableSize );		///< 预留2048个messageid映射器地址
}

int TableFillerRegister::Initialize()
{
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







