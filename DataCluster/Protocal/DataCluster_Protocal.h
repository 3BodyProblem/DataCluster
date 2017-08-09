#ifndef __DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__DATACLUSTER_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


#define		QUO_MAX_CODE			32							///< ��Ʊ����Ʒ����Լ�ȴ�����󳤶�
#define		QUO_MAX_NAME			64							///< ��Ʊ����Ʒ����Լ��������󳤶�
#define		QUO_MAX_BUYSELL			10							///< ��Ʊ����Ʒ����Լ�������������
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


/**
 * @class			QUO_MARKETSTATUS
 * @brief			�г�״̬
 * @author			barry
 * @date			2017/8/8
 */
enum QUO_MARKETSTATUS
{
	QUO_MARKETSTATUS_INIT = 0,									///< ��ʼ��״̬
	QUO_MARKETSTATUS_NORMAL,									///< ����״̬
};


///< ---------------------------------------------------------------------------------------


typedef struct
{
	char						Code[QUO_MAX_CODE];				///< ��Լ����
	char						Name[QUO_MAX_NAME];				///< ��Լ����
	unsigned int				Kind;							///< ֤ȯ����
	unsigned char				DerivativeType;					///< ����Ʒ���ͣ�ŷʽ��ʽ+�Ϲ��Ϲ�
	unsigned int				LotSize;						///< һ�ֵ��ڼ��ź�Լ
	char						UnderlyingCode[QUO_MAX_CODE];	///< ���֤ȯ����
	unsigned int				ContractMult;					///< ��Լ����
	unsigned int				XqPrice;						///< ��Ȩ�۸�[*�Ŵ���]
	unsigned int				StartDate;						///< �׸�������(YYYYMMDD)
	unsigned int				EndDate;						///< �������(YYYYMMDD)
	unsigned int				XqDate;							///< ��Ȩ��(YYYYMM)
	unsigned int				DeliveryDate;					///< ������(YYYYMMDD)
	unsigned int				ExpireDate;						///< ������(YYYYMMDD)
	unsigned short				TypePeriodIdx;					///< ���ཻ��ʱ���λ��
	unsigned char				EarlyNightFlag;					///< ����orҹ�̱�־ 1������ 2��ҹ�� 
	double						PriceTick;						///< ��С�䶯��λ
} tagQuoReferenceData;


typedef struct
{
	double						Price;							///< ί�м۸�[* �Ŵ���]
	unsigned __int64			Volume;							///< ί����[��]
} tagBuySellItem;


typedef struct
{
	char						Code[QUO_MAX_CODE];				///< ��Լ����
	double						Open;							///< ���̼�[*�Ŵ���]
	double						Close;							///< ���ռ�[*�Ŵ���]
	double						PreClose;						///< ���ռ�[*�Ŵ���]
	double						UpperPrice;						///< ������ͣ�۸�[*�Ŵ���], 0��ʾ������
	double						LowerPrice;						///< ���յ�ͣ�۸�[*�Ŵ���], 0��ʾ������
	double						SettlePrice;					///< ����[*�Ŵ���]
	double						PreSettlePrice;					///< ��Լ���[*�Ŵ���]
	unsigned __int64			PreOpenInterest;				///< ���ճֲ���(��)
	double						Now;							///< ���¼�[*�Ŵ���]
	double						High;							///< ��߼�[*�Ŵ���]
	double						Low;							///< ��ͼ�[*�Ŵ���]
	double						Amount;							///< �ܳɽ����[Ԫ]
	unsigned __int64			Volume;							///< �ܳɽ���[��/��]
	unsigned __int64			Position;						///< �ֲ���
	tagBuySellItem				Buy[QUO_MAX_BUYSELL];			///< ���嵵
	tagBuySellItem				Sell[QUO_MAX_BUYSELL];			///< ���嵵
	char						TradingPhaseCode[QUO_MAX_PHASECODE];///< ��ǰ��Ʒ״̬����������ϸ˵����
} tagQuoSnapData;
//��ǰ��Ʒ״̬
//������������1�ַ�ͨ��: 'P' ��ʾͣ�� 'T'��ʾ����
//��1���Ϻ�֤ȯ����������2 -- 9���ַ���Ӧ����ԴTradingPhaseCode�ֶΣ����嶨������
//		���ֶ�Ϊ8λ�ַ���������ÿλ��ʾ�ض��ĺ��壬�޶�������ո�
//		��1λ����S����ʾ����������ǰ��ʱ�Σ���C����ʾ���Ͼ���ʱ�Σ���T����ʾ��������ʱ�Σ���B����ʾ����ʱ�Σ���E����ʾ����ʱ�Σ���P����ʾ��Ʒͣ�ƣ���M����ʾ�ɻָ����׵��۶�ʱ�Σ����м��Ͼ��ۣ�����N����ʾ���ɻָ����׵��۶�ʱ�Σ���ͣ���������У�����D����ʾ���̼��Ͼ��۽׶ν������������۽׶ο�ʼ֮ǰ��ʱ�Σ����У���
//		��2λ�� ��0����ʾ�˲�Ʒ�����������ף���1����ʾ�˲�Ʒ���������ף���������ո�
//		��3λ����0����ʾδ���У���1����ʾ�����С�
//		��4λ����0����ʾ�˲�Ʒ�ڵ�ǰʱ�β����ܽ����¶����걨����1�� ��ʾ�˲�Ʒ�ڵ�ǰʱ�οɽ��ܽ����¶����걨����������ո�
//��2������֤ȯ����������2 -- 3���ַ���Ӧ����ԴTradingPhaseCode�ֶΣ����嶨������
//		���ֶ�Ϊ2λ�ַ�������Ʒ�����Ľ��׽׶δ���
//		��1λ��
//			S = ����������ǰ�� ����������ǰ�� ����������ǰ�� ����������ǰ��
//			O = ���̼��Ͼ��� ���̼��Ͼ��� ���̼��Ͼ���
//			T = �������� ��������
//			B = ����
//			C = ���̼��Ͼ��� ���̼��Ͼ��� ���̼��Ͼ���
//			E = �ѱ��� �ѱ���
//			H = ��ʱͣ�� ��ʱͣ��
//			A = �̺��� �̺���
//			V = �������ж� �������ж� �������ж�
//		��2λ��
//			0= ����״̬ ����״̬ ����״̬
//			1= ȫ��ͣ�� ȫ��ͣ�� ȫ
//��3�������������޸�����




#pragma pack()
#endif









