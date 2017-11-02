#ifndef CMD_GAME_HEAD_FILE
#define CMD_GAME_HEAD_FILE

#pragma pack(1)

//////////////////////////////////////////////////////////////////////////

//配牌器开关
//#ifdef _DEBUG
#define CARD_DISPATCHER_CONTROL
//#endif

///////////////////////////////////////////////////////////////

#define MAX_MAGIC_COUNT				1									//最大财神数
#define MAX_RECORD_COUNT			32									//房卡结算统计最大局数

//////////////////////////////////////////////////////////////////////////////////
//服务定义

//游戏属性
#define KIND_ID						400									//游戏 I D
#define GAME_NAME					TEXT("义乌麻将")					//游戏名字

//组件属性
#define VERSION_SERVER				PROCESS_VERSION(7,0,1)				//程序版本
#define VERSION_CLIENT				PROCESS_VERSION(7,0,1)				//程序版本

//////////////////////////////////////////////////////////////////////////////////

#define MIN_USER_NUM				2									//最小人数
#define MAX_USER_NUM				4									//最大人数
#define INVALID_VAL					0xFF								//非法值
//胡牌算法相关定义，别乱动
#define MAX_INDEX					34
#define MAX_VAL_NUM					9
#define MAX_KEY_NUM					(MAX_VAL_NUM+1)						//9+赖子
#define MAX_NAI_NUM					4									//赖子最大个数
#define BIT_VAL_NUM					3									//一个值占的bit数
#define BIT_VAL_FLAG				0x07								//

//常量定义
#define MAX_WEAVE					4									//最大组合
#define MAX_COUNT					14									//最大手牌数
#define MAX_ALL_NUM					136									//最大库存
#define MAX_TING_SHOW				8									//听牌提示最大显示数量

#define HUANG_NUM1					34									//标准荒庄留牌数 lv1
#define HUANG_NUM2					14									//标准荒庄留牌数 lv2
#define HUANG_NUM3					6									//标准荒庄留牌数 lv3

#define SCORE_DI_7					7									//7底分
#define SCORE_DI_11					11									//11底分
#define SCORE_DI_ZHUANG				2									//庄额外底分
#define SCORE_DI_PAO				1									//放炮额外分
#define SCORE_BUY_ZI				2									//买子底分

//数值掩码
#define	MASK_COLOR					0xF0								//花色掩码
#define	MASK_VALUE					0x0F								//数值掩码

//游戏玩法
#define PlayMode_Magic_3			0x00000001							//1: 三百搭		0: 四百搭
#define PlayMode_IsCenBao			0x00000002							//1: 三口承包	0：三口不承包
#define PlayMode_IsDeFen7			0x00000004							//1: 7底分		0：11底分
#define PlayMode_CanDianHu			0x00000008							//1: 可放炮胡	0：不可放炮胡（只能自摸胡）
#define PlayMode_DaKun				0x00000010							//1: 打捆模式	0：非打捆模式

//////////////////////////////////////////////////////////////////////////////////

//动作标志
#define WIK_NULL					0x00000000							//没有类型(过)
#define WIK_CHI						0x00000001							//吃
#define WIK_PENG					0x00000002							//碰牌
#define WIK_GANG_MING				0x00000004							//明杠（点杠）
#define WIK_GANG_BU					0x00000008							//补杠（碰后再杠）
#define WIK_GANG_AN					0x00000010							//暗杠
#define WIK_LISTEN					0x00000020							//听牌
#define WIK_ZIMO_HU					0x00000040							//自摸胡
#define WIK_PAO_HU					0x00000080							//放炮胡

//胡牌定义
#define CHR_HU_NORMAL				0x00000001							//平胡
#define CHR_HU_QI_DUI				0x00000002							//七对
#define CHR_HU_13_LAN				0x00000004                          //十三烂
#define CHR_NAIZI_JIANG				0x00000008							//财神头（赖子做将、只能自摸）
#define CHR_HU_ZIMO					0x00000010							//自摸
#define CHR_GANG_QIANG_HU			0x00000020							//抢杠胡
#define CHR_GANG_KAIHUA				0x00000040							//杠上开花
#define CHR_PIAO_NAI1				0x00000080							//单漂
#define CHR_PIAO_NAI2				0x00000100							//双漂

// #define CHR_HUN_YI_SE				0x00000010							//混一色
// #define CHR_QING_YI_SE				0x00000020							//清一色
// #define CHR_PENG_PENG				0x00000040							//碰碰胡
// #define CHR_QIANG_GANG_HU			0x00000200							//抢杠胡
// #define CHR_HAI_DI_LAO			    0x00000400							//海底捞

//当前操作状态
enum enStationOpt
{
	enStationOpt_doing,					//正在操作中
	enStationOpt_outCard,				//出牌后,等其他玩家操作(吃碰杠胡过)
	enStationOpt_end					//操作完成
};

//麻将颜色（种类）定义
enum enColorMJ
{
	enCor_WAN = 0,			//万
	enCor_TONG,				//筒
	enCor_TIAO,				//条
	enCor_FenZi,			//风、字 牌
	enCor_Max,
};

//麻将牌面值
static BYTE s_cbCardsArray[MAX_ALL_NUM] =
{
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,						//万子
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,						//万子
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,						//万子
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,						//万子
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,						//索子
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,						//索子
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,						//索子
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,						//索子
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,						//同子
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,						//同子
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,						//同子
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,						//同子
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,									//东 南 西 北 中 发 白
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,									//东 南 西 北 中 发 白
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,									//东 南 西 北 中 发 白
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,									//东 南 西 北 中 发 白
};

static BYTE s_cbCardsVal[MAX_INDEX] =
{
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
};

static BYTE s_cbCardsIndex[256] = {};

//由 牌索引值 得到 牌面值
inline BYTE getValByIndex(BYTE byIndex)
{
	if (byIndex < MAX_INDEX)
		return s_cbCardsVal[byIndex];
	return INVALID_VAL;
}

//由 牌面值 得到 牌索引值
inline BYTE getIndexByVal(BYTE byCard)
{
	if (s_cbCardsIndex[0] == 0)
	{
		memset(s_cbCardsIndex, INVALID_VAL, sizeof(s_cbCardsIndex));
		for (int i = 0; i < MAX_INDEX; ++i)
			s_cbCardsIndex[s_cbCardsVal[i]] = i;
	}
	return s_cbCardsIndex[byCard];

// 	BYTE byCor = byCard & 0xF0;
// 	BYTE byVal = byCard & 0x0F;
// 	BYTE byMaxVal = (byCor == enCor_FenZi) ? 7 : 9;
// 	if (byCor < enCor_Max && byVal > 0 && byVal <= byMaxVal)
// 		return byCor * 9 + byVal - 1;
// 	return INVALID_VAL;
}

//////////////////////////////////////////////////////////////////////////////////
//组合子项
struct tagWeaveItem
{
	DWORD							dwWeaveKind;						//组合类型 WIK_NULL
	BYTE							cbCenterCard;						//中心扑克
	WORD							wProvideUser;						//供应用户
	BYTE							cbCardData[4];						//麻将数据

	tagWeaveItem(){ memset(this, 0, sizeof(*this)); }
};

//房卡记录
struct CMD_S_RECORD
{
	int			nCount;
	BYTE		cbHuCount[MAX_USER_NUM];								//胡牌次数
	BYTE		cbChenBaoCount[MAX_USER_NUM];							//承包次数
	BYTE		cbPiaoOne[MAX_USER_NUM];								//单漂次数
	BYTE		cbPiaoTwo[MAX_USER_NUM];								//双漂次数
	LONGLONG	lAllScore[MAX_USER_NUM];								//总结算分
	LONGLONG	lDetailScore[MAX_USER_NUM][MAX_RECORD_COUNT];			//单局结算分
	LONGLONG	lAllScoreSp[MAX_USER_NUM];								//总结算特殊分	
	LONGLONG	lDetailScoreSp[MAX_USER_NUM][MAX_RECORD_COUNT];			//单局结算特殊分

	CMD_S_RECORD(){ memset(this, 0, sizeof(*this)); }
};

//状态定义
#define GAME_SCENE_FREE				GAME_STATUS_FREE					//等待开始
#define GAME_SCENE_XIAZI			50									//下子状态
#define GAME_SCENE_PLAY				GAME_STATUS_PLAY					//游戏进行

struct stStatusBase
{
	BYTE							byPlayNum;							//人数
	LONG							lCellScore;							//基础积分
	//时间信息
	BYTE							cbTimeOutCard;						//出牌时间
	BYTE							cbTimeOperateCard;					//操作时间
	BYTE							cbTimeStartGame;					//开始时间
	BYTE							cbTimeWaitEnd;						//结算等待	
	//历史积分
	LONGLONG						lCurScore[MAX_USER_NUM];			//本局积分信息
	LONGLONG						lAllScore[MAX_USER_NUM];			//总积分信息
	LONGLONG						lAllScoreSpe[MAX_USER_NUM];			//特殊总积分信息
	//规则
	DWORD							dwPlayMode;							//本局玩法标记
	//买子下子状态
	BYTE							byBuyZi[MAX_USER_NUM];				//买子数
	BYTE							byXiaZi[MAX_USER_NUM];				//下子数
	BYTE							byXiaZiVal;							//本局下子数

	stStatusBase(){ memset(this, 0, sizeof(*this)); }
};

//空闲状态
struct CMD_S_StatusFree
{
	stStatusBase					sBaseInfo;							//基础信息	
};

//下子状态
struct CMD_S_StatusXiaZi
{
	stStatusBase					sBaseInfo;							//基础信息	
	bool							bCanBuyXiaZi[MAX_USER_NUM];			//是否可买、下子
	WORD							wZhuangUser;						//庄家索引
};

//游戏状态
struct CMD_S_StatusPlay
{
	stStatusBase					sBaseInfo;							//基础信息

	//游戏变量
	WORD							wBankerUser;						//庄家用户
	WORD							wCurrentUser;						//当前用户
	BYTE							cbMagicIndex;						//财神索引	

	//状态变量
	BYTE							cbActionCard;						//动作扑克
	DWORD							dwActionMask;						//动作掩码
	BYTE							cbLeftCardCount;					//剩余数目

	//出牌信息
	WORD							wOutCardUser;						//出牌用户
	BYTE							cbOutCardData;						//出牌扑克
	BYTE							cbDiscardCount[MAX_USER_NUM];		//丢弃数目
	BYTE							cbDiscardCard[MAX_USER_NUM][60];	//丢弃记录

	//扑克数据
	BYTE							cbCardCount[MAX_USER_NUM];			//牌数目
	BYTE							cbCardData[MAX_COUNT];				//牌数据
	BYTE							cbSendCardData;						//摸牌

	//组合扑克
	BYTE							cbWeaveCount[MAX_USER_NUM];				//组合数目
	tagWeaveItem					stWeaveArray[MAX_USER_NUM][MAX_WEAVE];	//组合数据
	BYTE							byOptCount;							//后接 stOptData 数量
	//... stOptData * byOptCount

	CMD_S_StatusPlay(){ memset(this, 0, sizeof(*this)); }
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_S_GAME_START				100									//游戏开始
#define SUB_S_MAGIC_CARD				102									//鬼牌
#define SUB_S_OUT_CARD					104									//用户出牌
#define SUB_S_SEND_CARD					105									//发送扑克
#define SUB_S_OPERATE_NOTIFY			106									//操作提示
#define SUB_S_OPERATE_RESULT			107									//操作结果
#define SUB_S_TING_DATA					108									//听牌数据
#define SUB_S_RECORD					109									//游戏记录
#define SUB_S_GAME_CONCLUDE				110									//游戏结束
#define SUB_S_SET_BASESCORE				111									//设置基数
#define SUB_S_TRUSTEE					112									//用户托管
#define SUB_S_BUY_XIA_ZI				113									//买子下子
#define SUB_S_BUY_XIA_ZI_RESULT			114									//买子下子结果

struct stOptData
{
	DWORD								dwAction;							//WIK_NULL
	BYTE								byCards[3];

	stOptData(){ memset(this, 0, sizeof(*this)); }
	stOptData(DWORD dwAct, BYTE byCard1, BYTE byCard2 = INVALID_VAL, BYTE byCard3 = INVALID_VAL)
	{
		dwAction = dwAct;
		byCards[0] = byCard1;
		byCards[1] = byCard2;
		byCards[2] = byCard3;
	}
};
//游戏开始	SUB_S_GAME_START
struct CMD_S_GameStart
{
	WORD				 				wBankerUser;						//当前庄家
	WORD								wSiceCount;							//骰子点数
	BYTE								cbCardData[MAX_COUNT];				//牌数据
	BYTE								byLeftNum;							//剩余牌数量
	BYTE								byOptCount;							//后接 stOptData 数量
	//... stOptData * byOptCount
	CMD_S_GameStart(){ memset(this, 0, sizeof(*this)); }
};
//录像数据	
struct Video_GameStart
{	
	TCHAR								szNickName[32];						//用户昵称		
	WORD								wChairID;							//椅子ID
	LONGLONG							lScore;								//积分
	LONG								lCellScore;							//底分
	LONG								lMaxLoseScore;						//单局最大输分
	DWORD								dwPlayMode;							//玩法
	BYTE								byUserCount;						//玩家数量
	CMD_S_GameStart						cmdPlayStart;						//

	Video_GameStart(){ memset(this, 0, sizeof(*this)); }
};
//发送扑克格式	SUB_S_SEND_CARD
struct CMD_S_SendCard
{
	WORD								wChairID;							//发牌用户
	BYTE								byCard;								//扑克值
	BYTE								byLeftNum;							//剩余牌数量
	BYTE								byOptCount;							//后接 stOptData 数量
	//... stOptData * byOptCount
	CMD_S_SendCard(){ memset(this, 0, sizeof(*this)); }
};
//用户出牌		SUB_S_OUT_CARD
struct CMD_S_OutCard
{
	WORD								wOutCardUser;						//出牌用户
	BYTE								cbOutCardData;						//出牌扑克
	bool								bSysOut;							//托管出牌

	CMD_S_OutCard(){ memset(this, 0, sizeof(*this)); }
};
//操作提示	SUB_S_OPERATE_NOTIFY
struct CMD_S_OperateNotify
{
	BYTE								cbActionCard;						//动作扑克
	WORD								wProvideUser;						//提供者
	BYTE								byOptCount;							//后接 stOptData 数量
	//... stOptData * byOptCount
	CMD_S_OperateNotify(){ memset(this, 0, sizeof(*this)); }
};

//操作命令	SUB_S_OPERATE_RESULT
struct CMD_S_OperateResult
{
	WORD								wOperateUser;						//操作用户
	WORD								wProvideUser;						//供应用户
	DWORD								cbOptAction;						//操作代码
	BYTE								cbOperateCard[3];					//操作扑克

	CMD_S_OperateResult(){ memset(this, 0, sizeof(*this)); }
};

struct stTingData
{
	BYTE							byOutCard;
	BYTE							byHuNum;
	BYTE							byHuCard[MAX_TING_SHOW];
	BYTE							byHuLeft[MAX_TING_SHOW];
	BYTE							byHuFan[MAX_TING_SHOW];

	stTingData(){ memset(this, 0, sizeof(*this)); }
	void addData(BYTE byCard, BYTE byLeft, BYTE byFan)
	{
		if (getIndexByVal(byCard) != INVALID_VAL)
		{
			if (byHuNum < MAX_TING_SHOW)
			{
				byHuCard[byHuNum] = byCard;
				byHuLeft[byHuNum] = byLeft;
				byHuFan[byHuNum] = byFan;
			}
			++byHuNum;
		}		
	}
};

//听牌辅助信息	SUB_S_TING_DATA
struct CMD_S_TING_DATA
{
	BYTE							byOutCount;							//可以听出牌数量
	//... stTingData * byOutCount
	CMD_S_TING_DATA(){ memset(this, 0, sizeof(*this)); }
};

//游戏结束	SUB_S_GAME_CONCLUDE
struct CMD_S_GameConclude
{
	//积分变量
	LONGLONG						lGameScore[MAX_USER_NUM];			//本局输赢
	LONGLONG						lGameScoreSp[MAX_USER_NUM];			//本局特殊分
	LONGLONG						lRevenue[MAX_USER_NUM];				//税收积分

	//结束信息
	WORD							wHuUser;							//胡的玩家
	WORD							wPaoUser;							//放炮玩家
	BYTE							cbProvideCard;						//供应扑克(自摸时最后一张，或点炮的牌)
	BYTE							cbMagicCard;						//赖子牌
	DWORD							dwChiHuKind[MAX_USER_NUM];			//胡牌标识 WIK_NULL

	bool							bChengBao[MAX_USER_NUM];			//承包关系   0: 不承包 1：承包
	bool							bFanChenBao;						//反承包翻倍 0：不翻倍 1：两口翻倍
	DWORD							dwPlayMode;							//游戏玩法 
	//买子下子状态
	BYTE							byBuyZi[MAX_USER_NUM];				//买子数
	BYTE							byXiaZi[MAX_USER_NUM];				//下子数	
	
	//游戏信息
	BYTE							cbCardCount[MAX_USER_NUM];						//扑克数目
	BYTE							cbHandCardData[MAX_USER_NUM][MAX_COUNT];		//扑克列表

	BYTE							cbWeaveNum[MAX_USER_NUM];						//组合数目
	tagWeaveItem					WeaveItemArray[MAX_USER_NUM][MAX_WEAVE];		//组合扑克

	CMD_S_GameConclude(){ memset(this, 0, sizeof(*this)); }
};

//用户托管		SUB_S_TRUSTEE
struct CMD_S_Trustee
{
	bool							bTrustee;							//是否托管
	WORD							wChairID;							//托管用户

	CMD_S_Trustee(){ memset(this, 0, sizeof(*this)); }
};

//买子下子		SUB_S_BUY_XIA_ZI
struct CMD_S_BuyXiaZi
{
	BYTE							byBuyZi[MAX_USER_NUM];				//买子数
	BYTE							byXiaZi[MAX_USER_NUM];				//下子数
	bool							bCanBuyXiaZi[MAX_USER_NUM];			//是否可买、下子
	BYTE							byXiaZiVal;							//本局下子数
	WORD							wZhuangUser;						//庄家索引

	CMD_S_BuyXiaZi(){ memset(this, 0, sizeof(*this)); }
};

//买子下子结果	SUB_S_BUY_XIA_ZI_RESULT
struct CMD_S_BuyXiaZiResult
{
	WORD							wChairID;							//玩家索引
	BYTE							byDoType;							//1:下子操作 0：买子操作
	BYTE							byXiaZiVal;							//>0:加子	 0：放弃加子

	CMD_S_BuyXiaZiResult(){ memset(this, 0, sizeof(*this)); }
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_C_OUT_CARD				1									//出牌命令
#define SUB_C_OPERATE_CARD			2									//操作扑克
#define SUB_C_TRUSTEE				3									//用户托管
#define SUB_C_CONFIG_CARD_DATA		4									//控制消息
#define SUB_C_SET_BUY_ZI			5									//买子下子

//出牌命令	SUB_C_OUT_CARD
struct CMD_C_OutCard
{
	BYTE							cbCardData;							//扑克数据

	CMD_C_OutCard(){ memset(this, 0, sizeof(*this)); }
};

//操作命令	SUB_C_OPERATE_CARD
struct CMD_C_OperateCard
{
	DWORD							dwOperateCode;						//操作代码
	BYTE							cbOperateCard[3];					//操作扑克

	CMD_C_OperateCard(){ memset(this, 0, sizeof(*this)); }
};

//用户托管	SUB_C_TRUSTEE
struct CMD_C_Trustee
{
	bool							bTrustee;							//是否托管	

	CMD_C_Trustee(){ memset(this, 0, sizeof(*this)); }
};

//买子下子	SUB_C_SET_BUY_ZI
struct CMD_C_BuyXiaZi
{
	bool							bBugXiaZi;							//买子下子

	CMD_C_BuyXiaZi(){ memset(this, 0, sizeof(*this)); }
};
//////////////////////////////////////////////////////////////////////////////////

#endif
