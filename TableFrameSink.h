#ifndef TABLE_FRAME_SINK_HEAD_FILE
#define TABLE_FRAME_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "AfxTempl.h"
#include "GameLogic.h"
#include "DlgCustomRule.h"
#include "GameVideoItemSink.h"
#include <vector>

using namespace std;
//////////////////////////////////////////////////////////////////////////////////

//效验类型
enum enEstimatKind
{
	EstimatKind_OutCard,			//出牌效验
	EstimatKind_GangCard,			//杠牌效验
};

//发牌状态
enum enSendStatus
{
	Not_Send = 0,					//无
	OutCard_Send,					//出牌后发牌
	Gang_Send,						//杠牌后发牌
	BuHua_Send,						//补花后发牌
};

//////////////////////////////////////////////////////////////////////////////////

//游戏桌子
class CTableFrameSink : public ITableFrameSink, public ITableUserAction
{
protected://房卡配置
	BYTE							m_cbPlayerCount;								//游戏人数
	int								m_nPlayMode;									//游戏玩法
	CMD_S_RECORD					m_stRecord;										//游戏记录

	//用户状态信息
protected:
	bool							m_bPlayStatus[MAX_USER_NUM];					//是否参与游戏
	bool							m_bTrustee[MAX_USER_NUM];						//是否托管
	DWORD							m_dwActionUndo[MAX_USER_NUM];					//用户可做动作 WIK_NULL 等
	DWORD							m_dwActionDone[MAX_USER_NUM];					//用户已做动作 WIK_NULL 等
	vector<stOptData>				m_vctOptData[MAX_USER_NUM];						//用户可做动作详细数据

	BYTE							m_cbOptCard[MAX_USER_NUM][3];					//操作扑克
	LONGLONG						m_llScore[MAX_USER_NUM];						//本局积分值
	LONGLONG						m_llScoreAll[MAX_USER_NUM];						//总积分值
	LONGLONG						m_llScoreAllSpe[MAX_USER_NUM];					//总特殊分值
	//特殊玩法状态信息
	BYTE							m_byPiaoNum[MAX_USER_NUM];						//飘的个数
	BYTE							m_byBuyZi[MAX_USER_NUM];						//买子数
	BYTE							m_byXiaZi[MAX_USER_NUM];						//下子数
	bool							m_isCanOptZi[MAX_USER_NUM];						//是否能买子下子
	BYTE							m_cbChengBao1[MAX_USER_NUM][MAX_USER_NUM];		//漂前承包
	BYTE							m_cbChengBao2[MAX_USER_NUM][MAX_USER_NUM];		//漂后承包

	//用户扑克信息
protected:
	BYTE							m_cbCardIndex[MAX_USER_NUM][MAX_INDEX];			//用户手牌
	BYTE							m_cbWeaveCount[MAX_USER_NUM];					//组合数目
	tagWeaveItem					m_WeaveItemArray[MAX_USER_NUM][MAX_WEAVE];		//组合扑克
	BYTE							m_cbOutNum[MAX_USER_NUM];						//出牌数目
	BYTE							m_cbOutCards[MAX_USER_NUM][60];					//弃牌记录	

	//操作信息
protected:
	WORD							m_wBankerUser;									//庄家用户
	WORD							m_wCurrentUser;									//当前操作用户
	BYTE							m_cbTouchCard;									//当前发牌
	WORD							m_cbLastOutUser;								//出牌用户
	BYTE							m_cbLastOutCard;								//当前出牌	
	enStationOpt					m_iCurOptStatus;								//操作状态
	WORD							m_wGangChairID;									//杠牌玩家索引
	WORD							m_wGangCount;									//杠牌次数

	//游戏数据
protected:
	BYTE							m_cbRepertoryCard[MAX_ALL_NUM];					//库存扑克(索引值，非牌面值)
	BYTE							m_cbLeftCardCount;								//剩余数目
	BYTE							m_cbLeftShowCount;								//真实剩余牌数
	BYTE							m_cbMagicIndex;									//百搭牌索引（万能牌）
	BYTE							m_cbIndexMShow;									//百搭牌显示索引
	WORD							m_wSiceCount;									//骰子点数
	DWORD							m_dwCurHuFlag;									//本局胡的标记
	
	//游戏视频
protected:
	CGameVideoItemSink				m_gameVideo;	
	
	//组件接口
protected:
	ITableFrame	*					m_pITableFrame;									//框架接口
	tagCustomRule *					m_pGameCustomRule;								//自定规则
	tagGameServiceOption *			m_pGameServiceOption;							//游戏配置
	tagGameServiceAttrib *			m_pGameServiceAttrib;							//游戏属性

	//函数定义
public:
	//构造函数
	CTableFrameSink();
	//析构函数
	virtual ~CTableFrameSink();

	//基础接口
public:
	//释放对象
	virtual VOID Release() { delete this; }
	//接口查询
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

	//管理接口
public:
	//复位桌子
	virtual VOID RepositionSink();
	//配置桌子
	virtual bool Initialization(IUnknownEx * pIUnknownEx);

	//查询接口
public:
	//查询限额
	virtual SCORE QueryConsumeQuota(IServerUserItem * pIServerUserItem);
	//最少积分
	virtual SCORE QueryLessEnterScore(WORD wChairID, IServerUserItem * pIServerUserItem);
	//查询是否扣服务费
	virtual bool QueryBuckleServiceCharge(WORD wChairID){return true;}

	//游戏事件
public:
	//游戏开始
	virtual bool OnEventGameStart(DWORD dwCount, DWORD dwMaxCount);
	//游戏结束
	virtual bool OnEventGameConclude(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
	//发送场景
	virtual bool OnEventSendGameScene(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret);
	
	//事件接口
public:
	//时间事件
	virtual bool OnTimerMessage(DWORD wTimerID, WPARAM wBindParam);
	//数据事件
	virtual bool OnDataBaseMessage(WORD wRequestID, VOID * pData, WORD wDataSize);
	//积分事件
	virtual bool OnUserScroeNotify(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);

	//网络接口
public:
	//游戏消息
	virtual bool OnGameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem);
	//框架消息
	virtual bool OnFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem);

	//比赛接口
public:
	//设置基数
	virtual void SetGameBaseScore(LONG lBaseScore);

	//用户事件
public:
	//用户断线
	virtual bool OnActionUserOffLine(WORD wChairID, IServerUserItem * pIServerUserItem) { return true; }
	//用户重入
	virtual bool OnActionUserConnect(WORD wChairID, IServerUserItem * pIServerUserItem) { return true; }
	//用户坐下
	virtual bool OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//用户起立
	virtual bool OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//用户同意
	virtual bool OnActionUserOnReady(WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) { return true; }

protected://游戏事件
	//用户操作
	bool OnUserOperateCard(WORD wChairID, DWORD dwAction, BYTE cbCard[3]);
	//用户托管
	bool OnUserTrustee(WORD wChairID, bool bTrustee);
	//用户买子下子
	bool OnMsgUserBuyXiaZi(WORD wChairID, bool bTrustee);
	//发牌
	void OnSendCards();
	//玩家逃跑
	bool OnUserFlee(WORD wChairID);
	
	//发回操作结果
	void SendActionResult(WORD wChairID, BYTE byCards[3], DWORD cbAction);
	//获取操作最大权限
	int  GetMaxRightOperation(WORD &wChairID, DWORD& dwMaxAction, DWORD dwAction[MAX_USER_NUM]);
	//检测并且执行等待的操作
	bool CheckDoWaitOperate(WORD wChairID = INVALID_CHAIR);
	//出牌
	bool OnMsgAction_OutCard(WORD wChairID, BYTE cbOutCard, bool bSysOut = false);
	//吃牌
	bool OnMsgAction_EatCard(WORD wChairID, BYTE cbCard[3], DWORD cbAction);
	//碰牌
	bool OnMsgAction_PengCard(WORD wChairID, BYTE cbCard[3]);
	//杠--点杠(明杠)
	bool OnMsgAction_DianGang(WORD wChairID, BYTE cbCard[3]);
	//杠--补杠
	bool OnMsgAction_BuGang(WORD wChairID, BYTE cbCard[3]);
	//杠--暗杠
	bool OnMsgAction_AnGang(WORD wChairID, BYTE cbCard[3]);
	//胡--自摸
	bool OnMsgAction_ZiMoHu(WORD wChairID, BYTE cbCard[3]);
	//胡--点炮胡
	bool OnMsgAction_DianPaoHu(WORD wChairID, BYTE cbCard[3]);
	//过(放弃)
	bool OnMsgAction_GiveUp(WORD wChairID, BYTE cbCard[3]);	
	//加桌面牌组（吃、碰、杠 成功后）
	bool AddWeaveItemArray(WORD wChairID, DWORD dwAction, BYTE cbCard[3]);

protected://辅助函数
	//获取场景基础信息
	void GetGameBaseInfo(stStatusBase &baseInfo);
	//获取某玩家可看到的所有牌统计信息
	void GetShowCardsInfo(WORD wChairID, BYTE byShowIndex[MAX_INDEX]);
	//从牌库中删牌（发牌）
	int	 DeleteCardMJ(BYTE byNum, BYTE byIndexOut[]);
	//得到玩家手牌牌面值
	int  GetHandleCards(WORD wChairID, BYTE byValOut[MAX_COUNT]);
	//获取测试数据
	int  getTestCards(string strFile, BYTE byCardsOut[], BYTE byMaxNum);
	//字符拆分函数
	void SplitString(const string &s, vector<string> &v, string c);
	//得到下一个位置玩家索引
	WORD GetNextChair(WORD wChairID);
	//转到指定玩家
	bool TurnToStation(WORD wChairID, bool bTouchCard);
	//检测当前玩家能进行的操作
	bool CheckCurPlayerOpt(WORD wChairID, BYTE bTouchCard, DWORD dwActionFlag, bool bSendMsg = true);
	//检测其他玩家是否能对出的牌进行操作
	bool CheckOtherPlayerOpt(WORD wOutChairID, BYTE byOutCard, DWORD dwActionFlag);	
	//清理一把游戏数据
	void ResetOneTurnInfo();
	//清理整场游戏数据
	void ResetAllTurnInfo();

	//算输赢分
	BYTE CalGameScore(CMD_S_GameConclude &GameConclude, bool &bScoreSpe);
	//发送听牌数据，打哪些牌胡哪些牌 
	void CheckTingMsg(WORD wChairID);	
	//
	void PrintLog(enTraceLevel lev, const char* format, ...);
};

#endif
