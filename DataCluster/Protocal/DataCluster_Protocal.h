#ifndef __DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


/*
���г���

���ӿ����г��ĸ����һһ��Ӧ��ʵ�Ľ����������ǰ��չ�Ʊ���ڻ�����Ȩ����ʵ���������л��֣����Ͻ������Է�Ϊ�Ͻ����ֻ����Ͻ�����Ȩ��
���嶨�������μ�QUO_MARKET_ID���塣

���г����ࡿ

�г������ǽ������ĳ���г�������Ʒ�����ٴη��࣬�Ա���ʹ���߿��Խ����ϲ㴦���������ԭ�����£�

֤ȯ�ֻ�������Ʒ�������з��࣬��ָ����A�ɡ�B�ɡ�ծȯ��������С�塢��ҵ��Ƚ��з��࣬�绦�������Ʊ
�����ڻ������ձ�Ľ��з��࣬��IF��IH��IO�����н�����ָ�ڻ�
��Ʒ�ڻ���������Ʒ�����з��࣬�绦ͭ�����׵ȣ�����̡�֣�̡����ڵ���Ʒ�ڻ�
��Ʊ��Ȩ�����ձ�Ľ��з��࣬��50ETF��Ȩ�ȣ����Ͻ���������Ĺ�Ʊ��ETF��Ȩ
��Ʒ��Ȩ�����ձ����Ʒ�����з��࣬�������Ȩ��������Ȩ�ȣ�����̺�֣��������Ʒ��Ȩ

������ʱ�Ρ�

һ���г��ڵ���Ʒ�п����в�ͬ�Ľ���ʱ�Σ�ÿ������ʱ���п�ʼʱ��ͽ���ʱ�䣬���Ͻ����ֻ�(90000,113000)(130000,150000)�����ӿ��н���ʱ�ΰ��ս��׵��Ⱥ�˳��
�������У���(90000,113000)��(130000,150000)֮ǰ���������ҹ�̣����һ������ʱ�εĿ�ʼʱ��������һ�������գ���������Ϊ��������(-210000,10000)��ʾ��ǰһ��
210000��ʼ������10000�������磨-210000,-230000����ʾ��ǰһ��210000��ʼ��ǰһ��230000����
����ҹ�̵���Ʒ������(-210000,10000)(90000,101500)(103000,113000)(130000,150000)
����ע�⣬��������ʱ��һ���ڿ�ʼ����ʱ��֮ǰ����һ������ʱ��ʱ��һ������ǰһ������ʱ��

����ǰ��Ʒ״̬��

������������1�ַ�ͨ��: 'P' ��ʾͣ�� 'T'��ʾ����
��1���Ϻ�֤ȯ����������2 -- 9���ַ���Ӧ����ԴTradingPhaseCode�ֶΣ����嶨������
		���ֶ�Ϊ8λ�ַ���������ÿλ��ʾ�ض��ĺ��壬�޶�������ո�
		��1λ����S����ʾ����������ǰ��ʱ�Σ���C����ʾ���Ͼ���ʱ�Σ���T����ʾ��������ʱ�Σ���B����ʾ����ʱ�Σ���E����ʾ����ʱ�Σ���P����ʾ��Ʒͣ�ƣ���M����ʾ�ɻָ����׵��۶�ʱ�Σ����м��Ͼ��ۣ�����N����ʾ���ɻָ����׵��۶�ʱ�Σ���ͣ���������У�����D����ʾ���̼��Ͼ��۽׶ν������������۽׶ο�ʼ֮ǰ��ʱ�Σ����У���
		��2λ�� ��0����ʾ�˲�Ʒ�����������ף���1����ʾ�˲�Ʒ���������ף���������ո�
		��3λ����0����ʾδ���У���1����ʾ�����С�
		��4λ����0����ʾ�˲�Ʒ�ڵ�ǰʱ�β����ܽ����¶����걨����1�� ��ʾ�˲�Ʒ�ڵ�ǰʱ�οɽ��ܽ����¶����걨����������ո�
��2������֤ȯ����������2 -- 3���ַ���Ӧ����ԴTradingPhaseCode�ֶΣ����嶨������
		���ֶ�Ϊ2λ�ַ�������Ʒ�����Ľ��׽׶δ���
		��1λ��
			S = ����������ǰ�� ����������ǰ�� ����������ǰ�� ����������ǰ��
			O = ���̼��Ͼ��� ���̼��Ͼ��� ���̼��Ͼ���
			T = �������� ��������
			B = ����
			C = ���̼��Ͼ��� ���̼��Ͼ��� ���̼��Ͼ���
			E = �ѱ��� �ѱ���
			H = ��ʱͣ�� ��ʱͣ��
			A = �̺��� �̺���
			V = �������ж� �������ж� �������ж�
		��2λ��
			0= ����״̬ ����״̬ ����״̬
			1= ȫ��ͣ�� ȫ��ͣ�� ȫ
��3�������������޸�����
*/


#define		QUO_MAX_CODE			32							///< ��Ʊ����Ʒ����Լ�ȴ�����󳤶�
#define		QUO_MAX_NAME			64							///< ��Ʊ����Ʒ����Լ��������󳤶�
#define		QUO_MAX_BUYSELL			10							///< ��Ʊ����Ʒ����Լ�������������
#define		QUO_MAX_KIND			64							///< �г�������������
#define		QUO_MAX_TRADESESSION	8							///< �г������ʱ��
#define		QUO_MAX_PHASECODE		16							///< ��Ʊ����Ʒ����Լ�Ƚ���״̬�б���󳤶�


/**
 * @class			QUO_MARKET_ID
 * @brief			�г����
 * @author			barry
 * @date			2017/8/8
 */
enum QUO_MARKET_ID
{
	QUO_MARKET_UNKNOW = 0,										///< δ֪�г����
	QUO_MARKET_SSE = 1,											///< �Ϻ�֤ȯ�������ֻ���������Ʊ������ծȯ�ȣ�
	QUO_MARKET_SSEOPT = 2,										///< �Ϻ�֤ȯ��������Ȩ������ETF��Ȩ��������Ȩ��
	QUO_MARKET_SZSE = 3,										///< ����֤ȯ�������ֻ���������Ʊ������ծȯ�ȣ�
	QUO_MARKET_SZSEOPT = 4,										///< ����֤ȯ��������Ȩ������ETF��Ȩ��������Ȩ��
	QUO_MARKET_CFFEX = 5,										///< �й������ڻ��������ڻ�
	QUO_MARKET_CFFEXOPT = 6,									///< �й������ڻ���������Ȩ
	QUO_MARKET_DCE = 7,											///< ������Ʒ�������ڻ�
	QUO_MARKET_DCEOPT = 8,										///< ������Ʒ��������Ȩ
	QUO_MARKET_CZCE = 9,										///< ֣����Ʒ�������ڻ�
	QUO_MARKET_CZCEOPT = 10,									///< ֣����Ʒ��������Ȩ
	QUO_MARKET_SHFE = 11,										///< �Ϻ��ڻ��������ڻ�
	QUO_MARKET_SHFEOPT = 12,									///< �Ϻ��ڻ���������Ȩ
};


enum QUO_MARKET_STATUS											///< �г�״̬
{
	QUO_STATUS_INIT = 0,										///< ��ʼ��״̬����ʼ��״̬��������������κ����ݣ�������Ӧ�õȵ���ʼ��������
	QUO_STATUS_NORMAL,											///< ����״̬
};


///< ---------------------------------------------------------------------------------------


typedef struct													///< �г�����ʱ����Ϣ
{
	int							iBeginTime;						///< ����ʱ�ο�ʼʱ�䣨HHMMSS��ʽ��������Ϊ��ֵ��ҹ�̣�
	int							iEndTime;						///< ����ʱ�ν���ʱ�䣨HHMMSS��ʽ��������Ϊ��ֵ��ҹ�̣�
} tagQUO_TradeSession;


typedef struct													///< �г������Ϣ
{
	char						szKindName[QUO_MAX_NAME];		///< �г��������
	unsigned int				uiLotSize;						///< ÿ��������ÿ�ֶ��ٹɡ�ÿ�ֶ����ŵȣ����Ʊÿ��100�ɣ�
	unsigned int				uiLotFactor;					///< �ֱ���
	double						dPriceTick;						///< �۸���С�䶯��λ����A�ɱ�����С�䶯0.01Ԫ��
	unsigned int				uiContractMult;					///< ��Լ���������ڻ�����Ȩ��
	unsigned int				uiContractUnit;					///< ��Լ��λ�����ڻ�����Ȩ��
	char						szUnderlyingCode[QUO_MAX_CODE];	///< ��Ĵ��롾����Ȩ��
	char						szUnderlyingName[QUO_MAX_NAME];	///< ������ơ�����Ȩ��
	char						cOptionType;					///< ��Ȩ���ͣ�'E' = ŷʽ��Ȩ 'A' = ��ʽ��Ȩ������Ȩ��
	tagQUO_TradeSession			mTradeSessionRecord[QUO_MAX_TRADESESSION];///< ����ʱ�μ�¼
	unsigned int				uiTradeSessionCount;			///< ����ʱ������
	char						szReserved[256];				///< ����
} tagQUO_KindInfo;


typedef struct													///< �г���Ϣ
{
	QUO_MARKET_ID				eMarketID;						///< �г����
	unsigned int				uiMarketDate;					///< �г����ڣ�YYYYMMDD��ʽ����ǰ�����������������գ�
	unsigned int				uiMarketTime;					///< �г�ʱ�䣨HHMMSSsss��ʽ����ȷ�����룬������Դ���ܾ�ȷ�����룬����벿����0��
	unsigned int				uiWareCount;					///< �г���Ʒ������
	tagQUO_KindInfo				mKindRecord[QUO_MAX_KIND];		///< �г�������Ϣ
	unsigned int				uiKindCount;					///< �г���������
	char						szReserved[256];				///< ����
} tagQUO_MarketInfo;


typedef struct													///< ���ƴ������Ϣ
{
	char						szCode[QUO_MAX_CODE];			///< ��Ʒ����
	QUO_MARKET_ID				eMarketID;						///< �г����
	unsigned int				uiKindID;						///< �����
	char						szName[QUO_MAX_NAME];			///< ��Ʒ����
	char						szContractID[QUO_MAX_CODE];		///< ��Լ���롾����Ȩ��
	char						cCallOrPut;						///< �Ϲ��Ϲ���'C' = �Ϲ� 'P' = �Ϲ�������Ȩ��
	double						dExercisePrice;					///< ��Ȩ�۸���Ȩ��
	unsigned int				uiStartDate;					///< �׸������գ�YYYYMMDD��ʽ������Ȩ��
	unsigned int				uiEndDate;						///< ������գ�YYYYMMDD��ʽ������Ȩ��
	unsigned int				uiExerciseDate;					///< ��Ȩ�գ�YYYYMMDD��ʽ������Ȩ��
	unsigned int				uiDeliveryDate;					///< �����գ�YYYYMMDD��ʽ������Ȩ��
	unsigned int				uiExpireDate;					///< �����գ�YYYYMMDD��ʽ������Ȩ��
	char						szReserved[256];				///< ����
} tagQUO_ReferenceData;


typedef struct													///< ��������Ϣ
{
	double						dVPrice;						///< ί�����۸�
	unsigned __int64			ui64Volume;						///< ί������
	unsigned __int64			ui64Records;					///< �ҵ�����
} tagQUO_BuySell;


typedef struct													///< ����������Ϣ
{
	char						szCode[QUO_MAX_CODE];			///< ��Ʒ����
	QUO_MARKET_ID				eMarketID;						///< �г����
	unsigned int				uiKindID;						///< �����
	unsigned int				uiTime;							///< ʱ�䣨HHMMSSmmm��ʽ����ȷ�����룩
	double						dPreClosePx;					///< ���ռ۸�
	double						dPreSettlePx;					///< ���۸񡾽��ڻ�����Ȩ��
	double						dOpenPx;						///< ���̼۸�
	double						dHighPx;						///< ��߼۸�
	double						dLowPx;							///< ��ͼ۸�
	double						dClosePx;						///< ���̼۸�
	double						dNowPx;							///< ���¼۸�
	double						dSettlePx;						///< ����۸񡾽��ڻ�����Ȩ��
	double						dUpperLimitPx;					///< ��ͣ�۸�
	double						dLowerLimitPx;					///< ��ͣ�۸�
	double						dAmount;						///< �ܳɽ����
	unsigned __int64			ui64Volume;						///< �ܳɽ���
	unsigned __int64			ui64OpenInterest;				///< �ֲܳ��������ڻ�����Ȩ��
	unsigned __int64			ui64PreOpenInterest;			///< ��ֲ��������ڻ�����Ȩ��
	unsigned __int64			ui64NumTrades;					///< �ɽ�����
	tagQUO_BuySell				mBid[QUO_MAX_BUYSELL];			///< ί����Ϣ
	tagQUO_BuySell				mAsk[QUO_MAX_BUYSELL];			///< ί����Ϣ
	double						dVOIP;							///< ����ģ�⾻ֵ��������
	char						szTradingPhaseCode[16];			///< ��ǰ��Ʒ״̬����������ϸ˵����
	char						szReserved[256];				///< ����
} tagQUO_SnapData;


#pragma pack()
#endif









