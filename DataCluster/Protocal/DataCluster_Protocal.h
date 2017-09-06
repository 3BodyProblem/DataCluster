#ifndef __DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


/*
【市场】

本接口中市场的概念并不一一对应真实的交易所，而是按照股票、期货、期权及真实交易所进行划分，如上交所可以分为上交所现货和上交所期权，
具体定义类别请参见QUO_MARKET_ID定义。

【市场分类】

市场分类是将具体的某个市场交易商品进行再次分类，以便于使用者可以进行上层处理，具体分类原则如下：

证券现货：按照品种类别进行分类，如指数、A股、B股、债券、基金、中小板、创业板等进行分类，如沪深交易所股票
金融期货：按照标的进行分类，如IF、IH、IO，如中金所股指期货
商品期货：按照商品类别进行分类，如沪铜、玉米等，如大商、郑商、上期的商品期货
股票期权：按照标的进行分类，如50ETF期权等，如上交所和深交所的股票和ETF期权
商品期权：按照标的商品类别进行分类，如白糖期权、豆粕期权等，如大商和郑商所的商品期权

【交易时段】

一个市场内的商品有可能有不同的交易时段，每个交易时段有开始时间和结束时间，如上交所现货(90000,113000)(130000,150000)，本接口中交易时段按照交易的先后顺序
进行排列，如(90000,113000)在(130000,150000)之前，如果遇到夜盘，则第一个交易时段的开始时间属于上一个日历日，所以描述为负数，如(-210000,10000)表示从前一天
210000开始到今天10000结束，如（-210000,-230000）表示从前一天210000开始到前一天230000结束
即有夜盘的商品描述如(-210000,10000)(90000,101500)(103000,113000)(130000,150000)
必须注意，结束交易时间一定在开始交易时间之前，后一个交易时段时间一定大于前一个交易时段

【当前商品状态】

各个交易所第1字符通用: 'P' 表示停牌 'T'表示交易
（1）上海证券交易所：第2 -- 9个字符对应行情源TradingPhaseCode字段，具体定义如下
		该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
		第1位：‘S’表示启动（开市前）时段，‘C’表示集合竞价时段，‘T’表示连续交易时段，‘B’表示休市时段，‘E’表示闭市时段，‘P’表示产品停牌，‘M’表示可恢复交易的熔断时段（盘中集合竞价），‘N’表示不可恢复交易的熔断时段（暂停交易至闭市），‘D’表示开盘集合竞价阶段结束到连续竞价阶段开始之前的时段（如有）。
		第2位： ‘0’表示此产品不可正常交易，‘1’表示此产品可正常交易，无意义填空格。
		第3位：‘0’表示未上市，‘1’表示已上市。
		第4位：‘0’表示此产品在当前时段不接受进行新订单申报，‘1’ 表示此产品在当前时段可接受进行新订单申报。无意义填空格。
（2）深圳证券交易所：第2 -- 3个字符对应行情源TradingPhaseCode字段，具体定义如下
		该字段为2位字符串，产品所处的交易阶段代码
		第1位：
			S = 启动（开市前） 启动（开市前） 启动（开市前） 启动（开市前）
			O = 开盘集合竞价 开盘集合竞价 开盘集合竞价
			T = 连续竞价 连续竞价
			B = 休市
			C = 收盘集合竞价 收盘集合竞价 收盘集合竞价
			E = 已闭市 已闭市
			H = 临时停牌 临时停牌
			A = 盘后交易 盘后交易
			V = 波动性中断 波动性中断 波动性中断
		第2位：
			0= 正常状态 正常状态 正常状态
			1= 全天停牌 全天停牌 全
（3）其他交易所无附件内
*/


#define		QUO_MAX_CODE			32							///< 股票、商品、合约等代码最大长度
#define		QUO_MAX_NAME			64							///< 股票、商品、合约等名称最大长度
#define		QUO_MAX_BUYSELL			10							///< 股票、商品、合约等买卖盘最大数
#define		QUO_MAX_KIND			64							///< 市场中最大分类数量
#define		QUO_MAX_TRADESESSION	8							///< 市场最大交易时段
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


enum QUO_MARKET_STATUS											///< 市场状态
{
	QUO_STATUS_INIT = 0,										///< 初始化状态（初始化状态不能请求和推送任何数据，调用者应该等到初始化结束）
	QUO_STATUS_NORMAL,											///< 正常状态
};


///< ---------------------------------------------------------------------------------------


typedef struct													///< 市场交易时段信息
{
	int							iBeginTime;						///< 交易时段开始时间（HHMMSS格式），可能为负值（夜盘）
	int							iEndTime;						///< 交易时段结束时间（HHMMSS格式），可能为负值（夜盘）
} tagQUO_TradeSession;


typedef struct													///< 市场类别信息
{
	char						szKindName[QUO_MAX_NAME];		///< 市场类别名称
	unsigned int				uiLotSize;						///< 每手数量（每手多少股、每手多少张等，如股票每手100股）
	unsigned int				uiLotFactor;					///< 手比率
	double						dPriceTick;						///< 价格最小变动单位（如A股报价最小变动0.01元）
	unsigned int				uiContractMult;					///< 合约乘数【仅期货、期权】
	unsigned int				uiContractUnit;					///< 合约单位【仅期货、期权】
	char						szUnderlyingCode[QUO_MAX_CODE];	///< 标的代码【仅期权】
	char						szUnderlyingName[QUO_MAX_NAME];	///< 标的名称【仅期权】
	char						cOptionType;					///< 期权类型（'E' = 欧式期权 'A' = 美式期权）【期权】
	tagQUO_TradeSession			mTradeSessionRecord[QUO_MAX_TRADESESSION];///< 交易时段记录
	unsigned int				uiTradeSessionCount;			///< 交易时段数量
	char						szReserved[256];				///< 保留
} tagQUO_KindInfo;


typedef struct													///< 市场信息
{
	QUO_MARKET_ID				eMarketID;						///< 市场编号
	unsigned int				uiMarketDate;					///< 市场日期（YYYYMMDD格式，当前行情数据所属交易日）
	unsigned int				uiMarketTime;					///< 市场时间（HHMMSSsss格式，精确到毫秒，如行情源不能精确到毫秒，则毫秒部分填0）
	unsigned int				uiWareCount;					///< 市场商品总数量
	tagQUO_KindInfo				mKindRecord[QUO_MAX_KIND];		///< 市场分类信息
	unsigned int				uiKindCount;					///< 市场分类数量
	char						szReserved[256];				///< 保留
} tagQUO_MarketInfo;


typedef struct													///< 名称代码表信息
{
	char						szCode[QUO_MAX_CODE];			///< 商品代码
	QUO_MARKET_ID				eMarketID;						///< 市场编号
	unsigned int				uiKindID;						///< 类别编号
	char						szName[QUO_MAX_NAME];			///< 商品名称
	char						szContractID[QUO_MAX_CODE];		///< 合约代码【仅期权】
	char						cCallOrPut;						///< 认沽认购（'C' = 认购 'P' = 认沽）【期权】
	double						dExercisePrice;					///< 行权价格【期权】
	unsigned int				uiStartDate;					///< 首个交易日（YYYYMMDD格式）【期权】
	unsigned int				uiEndDate;						///< 最后交易日（YYYYMMDD格式）【期权】
	unsigned int				uiExerciseDate;					///< 行权日（YYYYMMDD格式）【期权】
	unsigned int				uiDeliveryDate;					///< 交割日（YYYYMMDD格式）【期权】
	unsigned int				uiExpireDate;					///< 到期日（YYYYMMDD格式）【期权】
	char						szReserved[256];				///< 保留
} tagQUO_ReferenceData;


typedef struct													///< 买卖盘信息
{
	double						dVPrice;						///< 委买卖价格
	unsigned __int64			ui64Volume;						///< 委买卖量
	unsigned __int64			ui64Records;					///< 挂单笔数
} tagQUO_BuySell;


typedef struct													///< 快照数据信息
{
	char						szCode[QUO_MAX_CODE];			///< 商品代码
	QUO_MARKET_ID				eMarketID;						///< 市场编号
	unsigned int				uiKindID;						///< 类别编号
	unsigned int				uiTime;							///< 时间（HHMMSSmmm格式，精确到毫秒）
	double						dPreClosePx;					///< 昨收价格
	double						dPreSettlePx;					///< 昨结价格【仅期货、期权】
	double						dOpenPx;						///< 开盘价格
	double						dHighPx;						///< 最高价格
	double						dLowPx;							///< 最低价格
	double						dClosePx;						///< 收盘价格
	double						dNowPx;							///< 最新价格
	double						dSettlePx;						///< 结算价格【仅期货、期权】
	double						dUpperLimitPx;					///< 涨停价格
	double						dLowerLimitPx;					///< 跌停价格
	double						dAmount;						///< 总成交金额
	unsigned __int64			ui64Volume;						///< 总成交量
	unsigned __int64			ui64OpenInterest;				///< 总持仓量【仅期货、期权】
	unsigned __int64			ui64PreOpenInterest;			///< 昨持仓量【仅期货、期权】
	unsigned __int64			ui64NumTrades;					///< 成交笔数
	tagQUO_BuySell				mBid[QUO_MAX_BUYSELL];			///< 委买信息
	tagQUO_BuySell				mAsk[QUO_MAX_BUYSELL];			///< 委卖信息
	double						dVOIP;							///< 基金模拟净值【仅基金】
	char						szTradingPhaseCode[16];			///< 当前商品状态（见上面详细说明）
	char						szReserved[256];				///< 保留
} tagQUO_SnapData;


#pragma pack()
#endif









