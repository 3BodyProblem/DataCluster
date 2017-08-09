#ifndef __DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


#define		QUO_MAX_CODE			32							///< 股票、商品、合约等代码最大长度
#define		QUO_MAX_NAME			64							///< 股票、商品、合约等名称最大长度
#define		QUO_MAX_BUYSELL			10							///< 股票、商品、合约等买卖盘最大数
#define		QUO_MAX_PHASECODE		16							///< 股票、商品、合约等交易状态列表最大长度


/**
 * @class			QUO_MARKET_ID
 * @brief			市场编号
 * @author			barry
 * @date			2017/8/8
 */
enum QUO_MARKET_ID
{
	QUO_MARKET_UNKNOW = 0,										///< 未知市场编号
	QUO_MARKET_SSE = 1,											///< 上海证券交易所现货（包括股票、基金、债券等）
	QUO_MARKET_SSEOPT = 2,										///< 上海证券交易所期权（包括ETF期权、个股期权）
	QUO_MARKET_SZSE = 3,										///< 深圳证券交易所现货（包括股票、基金、债券等）
	QUO_MARKET_SZSEOPT = 4,										///< 深圳证券交易所期权（包括ETF期权、个股期权）
	QUO_MARKET_CFFEX = 5,										///< 中国金融期货交易所期货
	QUO_MARKET_CFFEXOPT = 6,									///< 中国金融期货交易所期权
	QUO_MARKET_DCE = 7,											///< 大连商品交易所期货
	QUO_MARKET_DCEOPT = 8,										///< 大连商品交易所期权
	QUO_MARKET_CZCE = 9,										///< 郑州商品交易所期货
	QUO_MARKET_CZCEOPT = 10,									///< 郑州商品交易所期权
	QUO_MARKET_SHFE = 11,										///< 上海期货交易所期货
	QUO_MARKET_SHFEOPT = 12,									///< 上海期货交易所期权
};


/**
 * @class			QUO_MARKETSTATUS
 * @brief			市场状态
 * @author			barry
 * @date			2017/8/8
 */
enum QUO_MARKETSTATUS
{
	QUO_MARKETSTATUS_INIT = 0,									///< 初始化状态
	QUO_MARKETSTATUS_NORMAL,									///< 正常状态
};


///< ---------------------------------------------------------------------------------------


typedef struct
{
	char						Code[QUO_MAX_CODE];				///< 合约代码
	char						Name[QUO_MAX_NAME];				///< 合约名称
	unsigned int				Kind;							///< 证券类型
	unsigned char				DerivativeType;					///< 衍生品类型：欧式美式+认购认沽
	unsigned int				LotSize;						///< 一手等于几张合约
	char						UnderlyingCode[QUO_MAX_CODE];	///< 标的证券代码
	unsigned int				ContractMult;					///< 合约乘数
	unsigned int				XqPrice;						///< 行权价格[*放大倍数]
	unsigned int				StartDate;						///< 首个交易日(YYYYMMDD)
	unsigned int				EndDate;						///< 最后交易日(YYYYMMDD)
	unsigned int				XqDate;							///< 行权日(YYYYMM)
	unsigned int				DeliveryDate;					///< 交割日(YYYYMMDD)
	unsigned int				ExpireDate;						///< 到期日(YYYYMMDD)
	unsigned short				TypePeriodIdx;					///< 分类交易时间段位置
	unsigned char				EarlyNightFlag;					///< 日盘or夜盘标志 1：日盘 2：夜盘 
	double						PriceTick;						///< 最小变动价位
} tagQuoReferenceData;


typedef struct
{
	double						Price;							///< 委托价格[* 放大倍数]
	unsigned __int64			Volume;							///< 委托量[股]
} tagBuySellItem;


typedef struct
{
	char						Code[QUO_MAX_CODE];				///< 合约代码
	double						Open;							///< 开盘价[*放大倍数]
	double						Close;							///< 今收价[*放大倍数]
	double						PreClose;						///< 昨收价[*放大倍数]
	double						UpperPrice;						///< 当日涨停价格[*放大倍数], 0表示无限制
	double						LowerPrice;						///< 当日跌停价格[*放大倍数], 0表示无限制
	double						SettlePrice;					///< 今结价[*放大倍数]
	double						PreSettlePrice;					///< 合约昨结[*放大倍数]
	unsigned __int64			PreOpenInterest;				///< 昨日持仓量(张)
	double						Now;							///< 最新价[*放大倍数]
	double						High;							///< 最高价[*放大倍数]
	double						Low;							///< 最低价[*放大倍数]
	double						Amount;							///< 总成交金额[元]
	unsigned __int64			Volume;							///< 总成交量[股/张]
	unsigned __int64			Position;						///< 持仓量
	tagBuySellItem				Buy[QUO_MAX_BUYSELL];			///< 买五档
	tagBuySellItem				Sell[QUO_MAX_BUYSELL];			///< 卖五档
	char						TradingPhaseCode[QUO_MAX_PHASECODE];///< 当前商品状态（见下面详细说明）
} tagQuoSnapData;
//当前商品状态
//各个交易所第1字符通用: 'P' 表示停牌 'T'表示交易
//（1）上海证券交易所：第2 -- 9个字符对应行情源TradingPhaseCode字段，具体定义如下
//		该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
//		第1位：‘S’表示启动（开市前）时段，‘C’表示集合竞价时段，‘T’表示连续交易时段，‘B’表示休市时段，‘E’表示闭市时段，‘P’表示产品停牌，‘M’表示可恢复交易的熔断时段（盘中集合竞价），‘N’表示不可恢复交易的熔断时段（暂停交易至闭市），‘D’表示开盘集合竞价阶段结束到连续竞价阶段开始之前的时段（如有）。
//		第2位： ‘0’表示此产品不可正常交易，‘1’表示此产品可正常交易，无意义填空格。
//		第3位：‘0’表示未上市，‘1’表示已上市。
//		第4位：‘0’表示此产品在当前时段不接受进行新订单申报，‘1’ 表示此产品在当前时段可接受进行新订单申报。无意义填空格。
//（2）深圳证券交易所：第2 -- 3个字符对应行情源TradingPhaseCode字段，具体定义如下
//		该字段为2位字符串，产品所处的交易阶段代码
//		第1位：
//			S = 启动（开市前） 启动（开市前） 启动（开市前） 启动（开市前）
//			O = 开盘集合竞价 开盘集合竞价 开盘集合竞价
//			T = 连续竞价 连续竞价
//			B = 休市
//			C = 收盘集合竞价 收盘集合竞价 收盘集合竞价
//			E = 已闭市 已闭市
//			H = 临时停牌 临时停牌
//			A = 盘后交易 盘后交易
//			V = 波动性中断 波动性中断 波动性中断
//		第2位：
//			0= 正常状态 正常状态 正常状态
//			1= 全天停牌 全天停牌 全
//（3）其他交易所无附件内




#pragma pack()
#endif









