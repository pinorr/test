#include "StdAfx.h"
#include "TableFrameSink.h"
#include <algorithm> 

//////////////////////////////////////////////////////////////////////////////////

#define IDI_OUT_CARD				1									//出牌定时器

//构造函数
CTableFrameSink::CTableFrameSink()
{
	srand((size_t)time(NULL));	

	//组件变量
	m_pITableFrame = NULL;
	m_pGameCustomRule = NULL;
	m_pGameServiceOption = NULL;
	m_pGameServiceAttrib = NULL;

	CHuPaiMJ::TrainAll();
	RepositionSink();

	//游戏变量
	m_wBankerUser = INVALID_CHAIR;
	m_cbPlayerCount = MAX_USER_NUM;
	ZeroMemory(&m_stRecord,sizeof(m_stRecord));

	return;
}

//析构函数
CTableFrameSink::~CTableFrameSink()
{
	
}

//接口查询
VOID * CTableFrameSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(ITableFrameSink,Guid,dwQueryVer);
	QUERYINTERFACE(ITableUserAction,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(ITableFrameSink,Guid,dwQueryVer);
	return NULL;
}

//复位桌子
VOID CTableFrameSink::RepositionSink()
{	
	return;
}

//配置桌子
bool CTableFrameSink::Initialization(IUnknownEx * pIUnknownEx)
{
	//查询接口
	m_pITableFrame = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,ITableFrame);

	//错误判断
	if (m_pITableFrame==NULL)
	{
		CTraceService::TraceString(TEXT("游戏桌子 CTableFrameSink 查询 ITableFrame 接口失败"),TraceLevel_Exception);
		return false;
	}

	//游戏配置
	m_pGameServiceAttrib = m_pITableFrame->GetGameServiceAttrib();
	m_pGameServiceOption = m_pITableFrame->GetGameServiceOption();

	//自定规则
	m_pGameCustomRule = (tagCustomRule *)m_pITableFrame->GetCustomRule();
	m_cbPlayerCount = max(MIN_USER_NUM, min(MAX_USER_NUM, m_pGameCustomRule->byPlayNum));

	m_nPlayMode = 0;
	//配置参数 pino
	if (m_pGameCustomRule->bBaiDaNum3)
		m_nPlayMode |= PlayMode_Magic_3;
	if (m_pGameCustomRule->bChengBaoChi3)
		m_nPlayMode |= PlayMode_IsCenBao;
	if (m_pGameCustomRule->bDiFenScore7)
		m_nPlayMode |= PlayMode_IsDeFen7;
	if (m_pGameCustomRule->bCanPaoHu)
		m_nPlayMode |= PlayMode_CanDianHu;
	if (m_pGameCustomRule->bDaKunMode)
		m_nPlayMode |= PlayMode_DaKun;
		
	ZeroMemory(&m_stRecord, sizeof(m_stRecord));
	m_pITableFrame->SetStartMode(START_MODE_FULL_READY);
	return true;
}

//消费能力
SCORE CTableFrameSink::QueryConsumeQuota(IServerUserItem * pIServerUserItem)
{
	return 0L;
}

//最少积分
SCORE CTableFrameSink::QueryLessEnterScore(WORD wChairID, IServerUserItem * pIServerUserItem)
{
	//金币游戏
	if (m_pGameServiceOption->wServerType&(GAME_GENRE_GOLD|SCORE_GENRE_POSITIVE))
	{		
		return 64 * m_pITableFrame->GetCellScore();
	}
	return 0;
}

//游戏开始
bool CTableFrameSink::OnEventGameStart(DWORD dwCount, DWORD dwMaxCount)
{
	//重置一把游戏数据
	ResetOneTurnInfo();
	m_pITableFrame->SetGameStatus(GAME_SCENE_XIAZI); 
	m_gameVideo.StartVideo(m_pITableFrame);

	//确定庄家,游戏第一局为第一个进入玩家当庄，后面闲家胡则换下一位置当庄
	if (m_wBankerUser == INVALID_CHAIR)
		m_wBankerUser = 0;
					
	//下子买子操作
	BYTE byHuVal = CHuPaiMJ::GetHuRate(m_dwCurHuFlag);
	bool bNeedBuyZi = false;
	CMD_S_BuyXiaZi cmdBuyXiaZi;
	cmdBuyXiaZi.byXiaZiVal = byHuVal;
	cmdBuyXiaZi.wZhuangUser = m_wBankerUser;		
	for (int i = 0; i < m_cbPlayerCount; ++i)
	{
		if (m_byBuyZi[i] == 0)
		{
			bNeedBuyZi = true;
			m_isCanOptZi[i] = true;
		}
		else if (byHuVal > 0)
		{
			if (m_byXiaZi[i] > 0)
			{
				m_byXiaZi[i] *= 2;
			}
			else if (i != m_wBankerUser)	//庄家不能下子
			{
				bNeedBuyZi = true;
				m_isCanOptZi[i] = true;
			}
		}
	}
	memcpy(cmdBuyXiaZi.byBuyZi, m_byBuyZi, sizeof(cmdBuyXiaZi.byBuyZi));
	memcpy(cmdBuyXiaZi.byXiaZi, m_byXiaZi, sizeof(cmdBuyXiaZi.byXiaZi));
	memcpy(cmdBuyXiaZi.bCanBuyXiaZi, m_isCanOptZi, sizeof(cmdBuyXiaZi.bCanBuyXiaZi));
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_BUY_XIA_ZI, &cmdBuyXiaZi, sizeof(cmdBuyXiaZi));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_BUY_XIA_ZI, &cmdBuyXiaZi, sizeof(cmdBuyXiaZi));
	m_gameVideo.AddVideoData(SUB_S_BUY_XIA_ZI, &cmdBuyXiaZi, sizeof(cmdBuyXiaZi));
	if (bNeedBuyZi == false)
	{
		OnSendCards();
	}
	return true;
}

//游戏结束
bool CTableFrameSink::OnEventGameConclude(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason)
{
	switch (cbReason)
	{
	case GER_NORMAL:		//常规结束
		{
			m_dwCurHuFlag = 0;
			m_pITableFrame->KillGameTimer(IDI_OUT_CARD);
			//变量定义
			CMD_S_GameConclude GameConclude;
			GameConclude.cbProvideCard = m_cbLastOutCard;
			GameConclude.cbMagicCard = getValByIndex(m_cbIndexMShow);
			GameConclude.dwPlayMode = m_nPlayMode;
			GameConclude.wPaoUser = INVALID_CHAIR;
			GameConclude.wHuUser = INVALID_CHAIR;
			memcpy(GameConclude.byBuyZi, m_byBuyZi, sizeof(GameConclude.byBuyZi));
			memcpy(GameConclude.byXiaZi, m_byXiaZi, sizeof(GameConclude.byXiaZi));

			DWORD dwHuFlag = 0;
			//结束信息
			for (WORD i = 0; i < m_cbPlayerCount; i++)
			{				
				GameConclude.cbCardCount[i] = CHuPaiMJ::SwitchToCardData(m_cbCardIndex[i], GameConclude.cbHandCardData[i]);
				GameConclude.cbWeaveNum[i] = m_cbWeaveCount[i];
				memcpy(GameConclude.WeaveItemArray[i], m_WeaveItemArray[i], sizeof(tagWeaveItem)*MAX_WEAVE);

				if (m_dwActionDone[i] & WIK_ZIMO_HU)
				{
					dwHuFlag = CHuPaiMJ::GetHuFlag(m_cbCardIndex[i], getIndexByVal(m_cbTouchCard), m_cbIndexMShow, true);
					if (dwHuFlag > 0)
					{
						dwHuFlag |= CHR_HU_ZIMO;
						if (m_wGangChairID != INVALID_CHAIR && m_wGangChairID == i)
							dwHuFlag |= CHR_GANG_KAIHUA;
						if (dwHuFlag & CHR_NAIZI_JIANG)
						{
							if (m_byPiaoNum[i] == 1)
								dwHuFlag |= CHR_PIAO_NAI1;
							else if (m_byPiaoNum[i] >= 2)
								dwHuFlag |= CHR_PIAO_NAI2;
						}
						m_dwCurHuFlag = dwHuFlag;
						GameConclude.dwChiHuKind[i] = dwHuFlag;
						GameConclude.wHuUser = i;
						++m_stRecord.cbHuCount[i];
					}
				}
				else if (m_dwActionDone[i] & WIK_PAO_HU)
				{
					dwHuFlag = CHuPaiMJ::GetHuFlag(m_cbCardIndex[i], getIndexByVal(m_cbLastOutCard), m_cbIndexMShow, false);
					if (dwHuFlag > 0)
					{
						if (m_wGangChairID != INVALID_CHAIR && m_wGangChairID != i)
							dwHuFlag |= CHR_GANG_QIANG_HU;

						m_dwCurHuFlag = dwHuFlag;
						GameConclude.dwChiHuKind[i] = dwHuFlag;
						GameConclude.wHuUser = i;
						GameConclude.wPaoUser = m_cbLastOutUser;

						++m_stRecord.cbHuCount[i];
						
						//胡的牌加入手牌
						if (m_cbLastOutCard != INVALID_VAL && GameConclude.cbCardCount[i] < MAX_COUNT)
						{
							GameConclude.cbHandCardData[i][GameConclude.cbCardCount[i]] = m_cbLastOutCard;
							++GameConclude.cbCardCount[i];
						}						
					}
				}
			}

			//统计承包关系次数
			for (WORD i = 0; i < m_cbPlayerCount; ++i)
			{
				for (WORD j = 0; j < m_cbPlayerCount; ++j)
				{
					if (i == j) continue;
					if (m_cbChengBao2[j][i] >= 2)
						++m_stRecord.cbChenBaoCount[j];
					else if (m_nPlayMode & PlayMode_IsCenBao && m_cbChengBao1[j][i] + m_cbChengBao2[j][i] >= 3)
						++m_stRecord.cbChenBaoCount[j];
				}				
			}
			//统计漂的次数
			for (WORD i = 0; i < m_cbPlayerCount; ++i)
			{
				if (m_byPiaoNum[i] == 1)
					++m_stRecord.cbPiaoOne[i];
				else if (m_byPiaoNum[i] == 2)
					++m_stRecord.cbPiaoTwo[i];
			}
			
			//计算输赢分
			bool bScoreSpe = false;
			BYTE byHuNum = CalGameScore(GameConclude, bScoreSpe);
			
			//积分变量
			tagScoreInfo ScoreInfoArray[MAX_USER_NUM];
			ZeroMemory(&ScoreInfoArray, sizeof(ScoreInfoArray));

			//统计积分
			for (WORD i = 0; i < m_cbPlayerCount; i++)
			{
				if(!m_bPlayStatus[i]) continue;

				//收税
				if (GameConclude.lGameScore[i]>0 && (m_pGameServiceOption->wServerType&GAME_GENRE_GOLD)!=0)
				{
					GameConclude.lRevenue[i] = m_pITableFrame->CalculateRevenue(i,GameConclude.lGameScore[i]);
					GameConclude.lGameScore[i] -= GameConclude.lRevenue[i];
				}				

				//历史积分
				m_llScore[i] = GameConclude.lGameScore[i];
				if (bScoreSpe == false || (m_nPlayMode & PlayMode_DaKun) == 0)
				{
					m_llScoreAll[i] += m_llScore[i];
					ScoreInfoArray[i].lScore = m_llScore[i];
					ScoreInfoArray[i].lRevenue = GameConclude.lRevenue[i];
					ScoreInfoArray[i].cbType = ScoreInfoArray[i].lScore > 0 ? SCORE_TYPE_WIN : SCORE_TYPE_LOSE;
				}
				else
				{
					GameConclude.lGameScoreSp[i] = GameConclude.lGameScore[i];
					GameConclude.lGameScore[i] = 0;
					m_llScoreAllSpe[i] += m_llScore[i];
				}					

				//朋友局记录
				if(m_stRecord.nCount<MAX_RECORD_COUNT)
				{
					m_stRecord.lDetailScore[i][m_stRecord.nCount] = GameConclude.lGameScore[i];
					m_stRecord.lAllScore[i] += GameConclude.lGameScore[i];
					m_stRecord.lDetailScoreSp[i][m_stRecord.nCount] = GameConclude.lGameScoreSp[i];
					m_stRecord.lAllScoreSp[i] += GameConclude.lGameScoreSp[i];
				}
			}

			m_stRecord.nCount++;
			
			//发送数据
			if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) !=0 )	//房卡模式
			{
				m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_RECORD,&m_stRecord,sizeof(m_stRecord));
				m_gameVideo.AddVideoData(SUB_S_RECORD, &m_stRecord, sizeof(m_stRecord));
			}

			m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));
			m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));			
			m_gameVideo.AddVideoData(SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));
			m_gameVideo.StopAndSaveVideo(m_pGameServiceOption->wServerID, m_pITableFrame->GetTableID());

			//写入积分
			m_pITableFrame->WriteTableScore(ScoreInfoArray, m_cbPlayerCount);			

			//结束游戏
			m_pITableFrame->ConcludeGame(GAME_SCENE_FREE);
			m_pITableFrame->SetGameStatus(GAME_STATUS_FREE);
			if((m_pGameServiceOption->wServerType & GAME_GENRE_PERSONAL) !=0 )	//房卡模式
			{
				if(m_pITableFrame->IsPersonalRoomDisumme() )	//当前朋友局解散清理记录
				{
					ResetAllTurnInfo();
				}
			}

			// 换庄 闲家赢则换下一位当庄 清空买子下子
			if (wChairID != INVALID_CHAIR && wChairID != m_wBankerUser)
			{
				memset(m_byBuyZi, 0, sizeof(m_byBuyZi));
				memset(m_byXiaZi, 0, sizeof(m_byXiaZi));
				m_wBankerUser = (m_wBankerUser+1) % m_cbPlayerCount;
			}			
			return true;
		}
	case GER_NETWORK_ERROR:	//网络中断
	case GER_USER_LEAVE:	//用户强退
		{
			return OnUserFlee(wChairID);
		}
	case GER_DISMISS:		//游戏解散
		{
			//变量定义
			CMD_S_GameConclude GameConclude;
			ZeroMemory(&GameConclude, sizeof(GameConclude));

			m_dwCurHuFlag = 0;
			m_wBankerUser = INVALID_CHAIR;
			GameConclude.cbProvideCard = m_cbLastOutCard;
			GameConclude.cbMagicCard = getValByIndex(m_cbIndexMShow);
			GameConclude.dwPlayMode = m_nPlayMode;
			GameConclude.wPaoUser = INVALID_CHAIR;
			GameConclude.wHuUser = INVALID_CHAIR;
			memcpy(GameConclude.byBuyZi, m_byBuyZi, sizeof(GameConclude.byBuyZi));
			memcpy(GameConclude.byXiaZi, m_byXiaZi, sizeof(GameConclude.byXiaZi));

			//用户扑克
			BYTE cbCardIndex=0;
			for (WORD i = 0; i < MAX_USER_NUM; i++)
			{
				GameConclude.cbCardCount[i] = CHuPaiMJ::SwitchToCardData(m_cbCardIndex[i], GameConclude.cbHandCardData[i]);
				GameConclude.cbWeaveNum[i]=m_cbWeaveCount[i];
				memcpy(GameConclude.WeaveItemArray[i], m_WeaveItemArray[i], sizeof(tagWeaveItem)*MAX_WEAVE);
			}

			//发送信息
			m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));
			m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));
			
			//结束游戏
			m_pITableFrame->ConcludeGame(GAME_SCENE_FREE);
			m_pITableFrame->SetGameStatus(GAME_STATUS_FREE);
			if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) !=0 )	//房卡模式
			{
				if( m_pITableFrame->IsPersonalRoomDisumme() )		//当前朋友局解散清理记录
				{
					ResetAllTurnInfo();
				}
			}			
			return true;
		}
	}

	//错误断言
	ASSERT(FALSE);
	return false;
}

//发送场景
bool CTableFrameSink::OnEventSendGameScene(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret)
{
	switch (cbGameStatus)
	{
	case GAME_SCENE_FREE:	//空闲状态
		{
			//构造数据
			CMD_S_StatusFree StatusFree;
			GetGameBaseInfo(StatusFree.sBaseInfo);

			if ((m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) != 0)//房卡模式
				m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_RECORD, &m_stRecord, sizeof(m_stRecord));

			//发送场景
			return m_pITableFrame->SendGameScene(pIServerUserItem, &StatusFree, sizeof(StatusFree));
		}
	case GAME_SCENE_XIAZI:	//下子状态
		{
			CMD_S_StatusXiaZi StatusXiaZi;
			GetGameBaseInfo(StatusXiaZi.sBaseInfo);
			StatusXiaZi.wZhuangUser = m_wBankerUser;
			memcpy(StatusXiaZi.bCanBuyXiaZi, m_isCanOptZi, MAX_USER_NUM);

			if ((m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) != 0)//房卡模式
				m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_RECORD, &m_stRecord, sizeof(m_stRecord));

			//发送场景
			return m_pITableFrame->SendGameScene(pIServerUserItem, &StatusXiaZi, sizeof(StatusXiaZi));
		}
	case GAME_SCENE_PLAY:	//游戏状态
		{
			//重入取消托管
			OnUserTrustee(wChairID, false);
			//变量定义
			int  nLen = 0;
			BYTE byData[2048] = {};
			CMD_S_StatusPlay StatusPlay;
			GetGameBaseInfo(StatusPlay.sBaseInfo);			

			//游戏变量
			StatusPlay.wBankerUser = m_wBankerUser;
			StatusPlay.wCurrentUser = m_wCurrentUser;
			StatusPlay.cbMagicIndex = m_cbIndexMShow;

			//状态变量
			StatusPlay.cbActionCard = m_cbLastOutCard;
			StatusPlay.cbLeftCardCount = m_cbLeftShowCount;
			StatusPlay.dwActionMask = m_dwActionUndo[wChairID];
	
			for (BYTE i = 0; i < min(MAX_USER_NUM, m_cbPlayerCount); ++i)
			{
				if (wChairID != i)
					StatusPlay.cbCardCount[i] = GetHandleCards(i, StatusPlay.cbCardData);
			}
			memset(StatusPlay.cbCardData, INVALID_VAL, sizeof(StatusPlay.cbCardData));
			StatusPlay.cbCardCount[wChairID] = GetHandleCards(wChairID, StatusPlay.cbCardData);
			if (m_wCurrentUser == wChairID)
				StatusPlay.cbSendCardData = m_cbTouchCard;

			//历史记录
			StatusPlay.wOutCardUser = m_cbLastOutUser;
			StatusPlay.cbOutCardData = m_cbLastOutCard;
			CopyMemory(StatusPlay.cbDiscardCard, m_cbOutCards, sizeof(StatusPlay.cbDiscardCard));
			CopyMemory(StatusPlay.cbDiscardCount, m_cbOutNum, sizeof(StatusPlay.cbDiscardCount));

			//组合扑克
			CopyMemory(StatusPlay.stWeaveArray, m_WeaveItemArray, sizeof(StatusPlay.stWeaveArray));
			CopyMemory(StatusPlay.cbWeaveCount, m_cbWeaveCount, sizeof(StatusPlay.cbWeaveCount));	

			StatusPlay.byOptCount = m_vctOptData[wChairID].size();
			memcpy(byData + nLen, &StatusPlay, sizeof(StatusPlay));
			nLen += sizeof(StatusPlay);
			if (StatusPlay.byOptCount > 0)
			{
				memcpy(byData + nLen, &m_vctOptData[wChairID][0], sizeof(stOptData)*StatusPlay.byOptCount);
				nLen += sizeof(stOptData)*StatusPlay.byOptCount;
			}
			
			//发送场景
			bool bRes = m_pITableFrame->SendGameScene(pIServerUserItem, byData, nLen);
			if(bRes)
			{
				if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) != 0 )	//房卡模式
					m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_RECORD,&m_stRecord,sizeof(m_stRecord));
				if (wChairID == m_wCurrentUser)
					CheckTingMsg(wChairID);
			}
			return bRes;
		}
	}

	//错误断言
	ASSERT(FALSE);

	return false;
}

//时间事件
bool CTableFrameSink::OnTimerMessage(DWORD wTimerID, WPARAM wBindParam)
{
	if(wTimerID == IDI_OUT_CARD)
	{
		m_pITableFrame->KillGameTimer(wTimerID);

		// 有操作先过
		if (m_dwActionUndo[m_wCurrentUser] > 0)
		{
			OnUserOperateCard(m_wCurrentUser, WIK_NULL, NULL);
		}
		BYTE byCard = m_cbTouchCard;
		if (getIndexByVal(byCard) == INVALID_VAL)
		{
			for (int i = 0; i < MAX_INDEX; i++)
			{
				if (m_cbCardIndex[m_wCurrentUser][i] > 0)
				{
					byCard = getValByIndex(i);
					break;
				}
			}
		}
		OnMsgAction_OutCard(m_wCurrentUser, byCard, true);
		return true;
	}
	return false;
}

//数据事件
bool CTableFrameSink::OnDataBaseMessage(WORD wRequestID, VOID * pData, WORD wDataSize)
{
	return false;
}

//积分事件
bool CTableFrameSink::OnUserScroeNotify(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason)
{
	return false;
}

//游戏消息
bool CTableFrameSink::OnGameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	switch (wSubCmdID)
	{
	case SUB_C_OUT_CARD:
		{
			if (wDataSize != sizeof(CMD_C_OutCard)) return false;
			if (pIServerUserItem->GetUserStatus() != US_PLAYING) return true;

			//消息处理
			CMD_C_OutCard * pOutCard = (CMD_C_OutCard *)pData;
			return OnMsgAction_OutCard(pIServerUserItem->GetChairID(), pOutCard->cbCardData);
		}
	case SUB_C_OPERATE_CARD:	//操作消息
		{
			if (wDataSize != sizeof(CMD_C_OperateCard)) return false;
			if (pIServerUserItem->GetUserStatus() != US_PLAYING) return true;

			//消息处理
			CMD_C_OperateCard * pOperateCard = (CMD_C_OperateCard *)pData;
			return OnUserOperateCard(pIServerUserItem->GetChairID(), pOperateCard->dwOperateCode, pOperateCard->cbOperateCard);
		}
	case SUB_C_TRUSTEE:
		{
			if (wDataSize != sizeof(CMD_C_Trustee)) return false;
			if (pIServerUserItem->GetUserStatus() != US_PLAYING) return true;

			CMD_C_Trustee *pTrustee =(CMD_C_Trustee *)pData;
			return OnUserTrustee(pIServerUserItem->GetChairID(), pTrustee->bTrustee);
		}
	case SUB_C_SET_BUY_ZI:
		{
			if (wDataSize != sizeof(CMD_C_BuyXiaZi)) return false;
			if (pIServerUserItem->GetUserStatus() != US_PLAYING) return true;

			CMD_C_BuyXiaZi *pBuyXiaZi = (CMD_C_BuyXiaZi *)pData;
			return OnMsgUserBuyXiaZi(pIServerUserItem->GetChairID(), pBuyXiaZi->bBugXiaZi);
		}
	case SUB_C_CONFIG_CARD_DATA:
		{
			return true;
		}
	default:
		break;
	}

	return false;
}

//框架消息
bool CTableFrameSink::OnFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	return false;
}

//用户坐下
bool CTableFrameSink::OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser)
{
	if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) != 0 )//房卡模式
	{
		//获取房卡规则
		BYTE* pSetInfo = m_pITableFrame->GetGameRule();
		if(pSetInfo[0] == 1)
		{
			m_cbPlayerCount = min(MAX_USER_NUM, pSetInfo[1]);
			m_cbPlayerCount = max(MIN_USER_NUM, m_cbPlayerCount);
			m_nPlayMode = 0;

			//BYTE cbMaxPlayerCount = pSetInfo[2];
			if (pSetInfo[3])	//1: 三百搭		0: 四百搭
				m_nPlayMode |= PlayMode_Magic_3;
			if (pSetInfo[4])	//1: 三吃承包	0：三吃不承包
				m_nPlayMode |= PlayMode_IsCenBao;
			if (pSetInfo[5])	//1: 7底分		0：11底分
				m_nPlayMode |= PlayMode_IsDeFen7;
			if (pSetInfo[6])	//1: 可放炮胡	0：不可放炮胡（只能自摸胡）
				m_nPlayMode |= PlayMode_CanDianHu;
			if (pSetInfo[7])	//1: 打捆模式   0：非打捆模式
			{
				m_cbPlayerCount = MAX_USER_NUM;
				m_nPlayMode |= PlayMode_DaKun;
			}			
		}
	}

	int nUserNum = 0;
	for (int i = 0; i < MAX_USER_NUM; ++i)
	{
		IServerUserItem *pServerUserItem = m_pITableFrame->GetTableUserItem(i);
		if (pServerUserItem)
			++nUserNum;
	}
	if (nUserNum == 1)			//第一个人进来时，清理桌子
		ResetAllTurnInfo();

	//历史积分
	if (bLookonUser == false)
	{
		ASSERT(wChairID != INVALID_CHAIR);
		m_llScore[wChairID] = 0;
		m_llScoreAll[wChairID] = 0;
		m_llScoreAllSpe[wChairID] = 0;
		m_wBankerUser = INVALID_CHAIR;
	}
	return true;
}

//用户起立
bool CTableFrameSink::OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser)
{
	//历史积分
	if (bLookonUser==false)
	{
		ASSERT(wChairID != INVALID_CHAIR);
		m_llScore[wChairID] = 0;
		m_llScoreAll[wChairID] = 0;
		m_llScoreAllSpe[wChairID] = 0;
		m_wBankerUser = INVALID_CHAIR;
	}

	if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) !=0 )//房卡模式
	{
		if(m_pITableFrame->IsPersonalRoomDisumme() )//当前朋友局解散清理记录
			ResetAllTurnInfo();
	}
	return true;
}

//得到下一个位置玩家索引
WORD CTableFrameSink::GetNextChair(WORD wChairID)
{
	return (wChairID + 1) % m_cbPlayerCount;
}

//发回操作结果
void CTableFrameSink::SendActionResult(WORD wChairID, BYTE byCards[3], DWORD cbAction)
{
	//构造结果
	CMD_S_OperateResult result;
	ZeroMemory(&result, sizeof(result));
	result.wOperateUser = wChairID;
	result.cbOptAction = cbAction;
	result.wProvideUser = m_cbLastOutUser;
	memcpy(result.cbOperateCard, byCards, 3);
	//发送消息
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_OPERATE_RESULT, &result, sizeof(result));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_OPERATE_RESULT, &result, sizeof(result));
	m_gameVideo.AddVideoData(SUB_S_OPERATE_RESULT, &result, sizeof(result));
	//清理数据
	m_dwActionUndo[wChairID] = 0;
	m_vctOptData[wChairID].clear();	
	m_cbTouchCard = INVALID_VAL;
	m_wGangChairID = INVALID_CHAIR;

	if (cbAction & (WIK_GANG_AN | WIK_GANG_BU | WIK_GANG_MING))
	{
		m_wGangChairID = wChairID;
		if (cbAction == WIK_GANG_AN)
			++m_stRecord.cbPiaoOne[wChairID];
		else if (cbAction == WIK_GANG_MING)
			++m_stRecord.cbPiaoTwo[wChairID];
	}
	if (wChairID < MAX_COUNT && m_cbLastOutUser < MAX_USER_NUM && cbAction & (WIK_GANG_MING | WIK_PENG | WIK_CHI))
	{
		if (m_byPiaoNum[wChairID] == 0)
			++m_cbChengBao1[wChairID][m_cbLastOutUser];
		else
			++m_cbChengBao2[wChairID][m_cbLastOutUser];
	}
};

//获取没有操作的最大权限相对值
int CTableFrameSink::GetMaxRightOperation(WORD &wChairID, DWORD& dwMaxAction, DWORD dwActionArray[])
{
	// 吃的等级本来是一致的，但只有一个玩家可以吃，所以不同也没事
	static DWORD s_dwAction[5] = { WIK_NULL, WIK_CHI, WIK_PENG, WIK_GANG_MING, WIK_PAO_HU };
	int nMaxLev = 0;
	for (int i = 0; i < m_cbPlayerCount; i++)
	{
		int nTempLev = 0;
		if (dwActionArray[i] > 0)
		{
			int nSize = sizeof(s_dwAction) / sizeof(DWORD);
			for (int n = nSize - 1; n > 0; --n)
			{
				if (dwActionArray[i] & s_dwAction[n])
				{
					nTempLev = n;
					break;
				}
			}
			int nDisChair = m_cbPlayerCount - ((i + m_cbPlayerCount - m_wCurrentUser) % m_cbPlayerCount);
			if (nMaxLev < (nTempLev << 8) + nDisChair)
			{
				dwMaxAction = s_dwAction[nTempLev];
				nMaxLev = (nTempLev << 8) + nDisChair;
				wChairID = i;				
			}
		}
	}
	return nMaxLev;
}

//检测并且执行等待的操作
bool CTableFrameSink::CheckDoWaitOperate(WORD nChairID/*INVALID_CHAIR*/)
{
	//检测没有操作的最大操作
	WORD  nMaxChair_Undo = INVALID_CHAIR;
	DWORD dwMacActon_Undo = WIK_NULL;
	int nMaxLev_Undo = GetMaxRightOperation(nMaxChair_Undo, dwMacActon_Undo, m_dwActionUndo);

	//获取已经操作的最大操作
	WORD  nMaxChair_Done = INVALID_CHAIR;
	DWORD dwMacActon_Done = WIK_NULL;
	int nMaxLev_Done = GetMaxRightOperation(nMaxChair_Done, dwMacActon_Done, m_dwActionDone);

	int nMaxChairID = nMaxChair_Done;
	if (nMaxLev_Done >= nMaxLev_Undo)	//已经操作的 大于等于 没有操作的 则直接操作
	{		
		if (nMaxChairID >= m_cbPlayerCount && dwMacActon_Done != WIK_NULL)
		{
			PrintLog(TraceLevel_Warning, "%s:%d nMaxChairID = %d  dwMacActon_Done = %x m_cbPlayerCount = %d",
				__FUNCTION__, __LINE__,	nMaxChairID, dwMacActon_Done, m_cbPlayerCount);
			return false;
		}
		memset(m_dwActionDone, WIK_NULL, sizeof(m_dwActionDone));
		if (nMaxChairID < MAX_USER_NUM)
			m_dwActionDone[nMaxChairID] = dwMacActon_Done;
		
		switch (dwMacActon_Done)
		{
		case WIK_CHI:  //吃牌
		{
			BYTE byIndex1 = getIndexByVal(m_cbOptCard[nMaxChairID][1]);
			BYTE byIndex2 = getIndexByVal(m_cbOptCard[nMaxChairID][2]);
			if (byIndex1 == INVALID_VAL || m_cbCardIndex[nMaxChairID][byIndex1] < 1 ||
				byIndex2 == INVALID_VAL || m_cbCardIndex[nMaxChairID][byIndex2] < 1)
			{
				PrintLog(TraceLevel_Warning, "%s:%d m_cbOptCard[1] = %02x  m_cbOptCard[2] = %02x", 
					__FUNCTION__, __LINE__, m_cbOptCard[nMaxChairID][1], m_cbOptCard[nMaxChairID][2]);
				return false;
			}
			//清空出牌牌堆
			if (m_cbOutNum[m_cbLastOutUser] > 0)
				--m_cbOutNum[m_cbLastOutUser];

			//执行动作			
			AddWeaveItemArray(nMaxChairID, dwMacActon_Done, m_cbOptCard[nMaxChairID]);
			--m_cbCardIndex[nMaxChairID][byIndex1];
			--m_cbCardIndex[nMaxChairID][byIndex2];			

			//清除各玩家操作的权限
			memset(m_dwActionUndo, 0, sizeof(m_dwActionUndo));
			memset(m_dwActionDone, 0, sizeof(m_dwActionDone));
			for (int i = 0; i < MAX_USER_NUM; ++i)
				m_vctOptData[i].clear();

			//转到当前玩家
			m_wCurrentUser = nMaxChairID;
			m_iCurOptStatus = enStationOpt_doing;

			//通知客户端
			SendActionResult(nMaxChairID, m_cbOptCard[nMaxChairID], dwMacActon_Done);
			CheckTingMsg(nMaxChairID);
			return true;
		}
		case WIK_PENG:  //碰牌
		{
			BYTE byIndex = getIndexByVal(m_cbOptCard[nMaxChairID][0]);
			if (byIndex == INVALID_VAL || m_cbCardIndex[nMaxChairID][byIndex] < 2)
			{
				PrintLog(TraceLevel_Warning, "%s:%d m_cbOptCard = %02x num=%d", 
					__FUNCTION__, __LINE__, m_cbOptCard[nMaxChairID][0], byIndex<MAX_INDEX ? m_cbCardIndex[nMaxChairID][byIndex] : 0);
				return false;
			}

			//清空出牌牌堆
			if (m_cbOutNum[m_cbLastOutUser] > 0)
				--m_cbOutNum[m_cbLastOutUser];

			//执行动作
			AddWeaveItemArray(nMaxChairID, dwMacActon_Done, m_cbOptCard[nMaxChairID]);
			m_cbCardIndex[nMaxChairID][byIndex] -= 2;			

			//清除各玩家操作的权限
			memset(m_dwActionUndo, 0, sizeof(m_dwActionUndo));
			memset(m_dwActionDone, 0, sizeof(m_dwActionDone));
			for (int i = 0; i < MAX_USER_NUM; ++i)
				m_vctOptData[i].clear();
			
			//转到当前玩家
			m_wCurrentUser = nMaxChairID;
			m_iCurOptStatus = enStationOpt_doing;

			//通知客户端
			SendActionResult(nMaxChairID, m_cbOptCard[nMaxChairID], dwMacActon_Done);

			//检测当前玩家能进行的操作
			//CheckCurPlayerOpt(nMaxChairID, INVALID_VAL, 0);		
			CheckTingMsg(nMaxChairID);
			return true;
		}		
		case WIK_GANG_MING: //杠--(明杠)点杠
		{			
			BYTE byIndex = getIndexByVal(m_cbOptCard[nMaxChairID][0]);
			if (byIndex == INVALID_VAL || m_cbCardIndex[nMaxChairID][byIndex] < 3)
			{
				return false;
			}

			//清空出牌牌堆
			if (m_cbOutNum[m_cbLastOutUser] > 0)
				--m_cbOutNum[m_cbLastOutUser];

			//执行动作
			AddWeaveItemArray(nMaxChairID, dwMacActon_Done, m_cbOptCard[nMaxChairID]);
			m_cbCardIndex[nMaxChairID][byIndex] -= 3;

			//清除各玩家操作的权限
			memset(m_dwActionUndo, 0, sizeof(m_dwActionUndo));
			memset(m_dwActionDone, 0, sizeof(m_dwActionDone));
			for (int i = 0; i < MAX_USER_NUM; ++i)
				m_vctOptData[i].clear();
									
			//通知客户端
			SendActionResult(nMaxChairID, m_cbOptCard[nMaxChairID], dwMacActon_Done);

			//转到当前玩家
			TurnToStation(nMaxChairID, true);
			return true;
		}
		case WIK_PAO_HU:
		{
			DWORD nHuFlag = CHuPaiMJ::CheckHu(m_cbCardIndex[nMaxChairID], getIndexByVal(m_cbLastOutCard), m_cbIndexMShow, false);
			BYTE byIndex = getIndexByVal(m_cbLastOutCard);
			if (byIndex == INVALID_VAL || nHuFlag == 0)
				return false;

			//清空出牌牌堆
			if (m_cbOutNum[m_cbLastOutUser] > 0)
				--m_cbOutNum[m_cbLastOutUser];
						
			//通知客户端
			//SendActionResult(nMaxChairID, m_cbOptCard[nMaxChairID], dwMacActon_Done);

			OnEventGameConclude(nMaxChairID, NULL, GER_NORMAL);
			return true;
		}
		case WIK_NULL:		//过
		{
			//清除各玩家操作的权限
			memset(m_dwActionUndo, 0, sizeof(m_dwActionUndo));
			memset(m_dwActionDone, 0, sizeof(m_dwActionDone));
			for (int i = 0; i < MAX_USER_NUM; ++i)
				m_vctOptData[i].clear();

			//转到下一个玩家
			if (m_wGangChairID == m_wCurrentUser)
				TurnToStation(m_wCurrentUser, true);		// 抢杠胡过
			else
				TurnToStation(GetNextChair(m_wCurrentUser), true);
			return true;
		}
		}
	}
	return false;
}

//出牌
bool CTableFrameSink::OnMsgAction_OutCard(WORD wChairID, BYTE byOutCard, bool bSysOut/*false*/)
{
	BYTE byIndex = getIndexByVal(byOutCard);
	if (wChairID >= m_cbPlayerCount || m_wCurrentUser != wChairID || enStationOpt_doing != m_iCurOptStatus || byIndex == INVALID_VAL)
	{		
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_wCurrentUser=%d m_iCurOptStatus=%d byOutCard=0x%02x",
			__FUNCTION__, __LINE__, wChairID, m_wCurrentUser, m_iCurOptStatus, byOutCard);
		return false;
	}

	if (m_cbCardIndex[wChairID][byIndex] == 0)
		return false;

	// 手上只有一张赖子牌时，不允许打出赖子牌
	if (byIndex == m_cbMagicIndex)
	{
		if (m_cbCardIndex[wChairID][byIndex] <= 1 || m_byPiaoNum[wChairID] >= 2)
		{
			PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_wCurrentUser=%d m_cbMagicIndex=%d byOutCard=0x%02x",
				__FUNCTION__, __LINE__, wChairID, m_wCurrentUser, m_cbMagicIndex, byOutCard);
			return false;
		}		

		BYTE byPiaoMaxOld = 0;
		for (int i = 0; i < m_cbPlayerCount; ++i)
			byPiaoMaxOld = max(byPiaoMaxOld, m_byPiaoNum[i]);

		++m_byPiaoNum[wChairID];

		//
		BYTE byPiaoMax = 0;
		for (int i = 0; i < m_cbPlayerCount; ++i)
			byPiaoMax = max(byPiaoMax, m_byPiaoNum[i]);

		if (byPiaoMaxOld < byPiaoMax)
		{
			//单漂每人加5张，双漂每人加2张
			if (byPiaoMax == 1)
				m_cbLeftShowCount += m_cbPlayerCount * 5;
			else if (byPiaoMax == 2)
				m_cbLeftShowCount += m_cbPlayerCount * 2;
		}		
	}

	--m_cbCardIndex[wChairID][byIndex];	
	m_cbLastOutCard = byOutCard;
	m_cbLastOutUser = wChairID;
	m_cbOutCards[wChairID][m_cbOutNum[wChairID]++] = byOutCard;	

	//SetCurMjShowSeat(m_byLastOutMjSetaNo);

	//出牌成功 清理状态
	m_wGangChairID = INVALID_CHAIR;
	m_dwActionUndo[wChairID] = 0;
	m_vctOptData[wChairID].clear();
	m_cbTouchCard = INVALID_VAL;
	memset(m_dwActionDone, 0, sizeof(m_dwActionDone));

	//通知客户端
	CMD_S_OutCard OutCard;
	OutCard.wOutCardUser = wChairID;
	OutCard.cbOutCardData = byOutCard;
	OutCard.bSysOut = bSysOut;
	
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard));
	m_gameVideo.AddVideoData(SUB_S_OUT_CARD, &OutCard, sizeof(OutCard));
	
	DWORD dwActionFlag = WIK_CHI | WIK_PENG | WIK_GANG_MING;
	if (m_nPlayMode & PlayMode_CanDianHu)
		dwActionFlag |= WIK_PAO_HU;

	if (m_cbMagicIndex != INVALID_VAL && m_cbMagicIndex == getIndexByVal(byOutCard))
		dwActionFlag = 0;	
	
	//检测是否有其他玩家 对出的牌 进行操作
	if (CheckOtherPlayerOpt(wChairID, byOutCard, dwActionFlag))
	{
		m_iCurOptStatus = enStationOpt_outCard;  //等待其他玩家操作
	}
	else
	{
		m_iCurOptStatus = enStationOpt_end;
		//下一个玩家
		TurnToStation(GetNextChair(wChairID), true);
	}
	return true;
}

//转到指定玩家
bool CTableFrameSink::TurnToStation(WORD wChairID, bool bTouchCard)
{
	m_wCurrentUser = wChairID;
	m_iCurOptStatus = enStationOpt_doing;
	m_cbLastOutCard = INVALID_VAL;
	m_cbLastOutUser = INVALID_CHAIR;
	m_cbTouchCard = INVALID_VAL;

	BYTE byIndex = INVALID_VAL;
	if (bTouchCard)
	{		
		//摸牌		
		int num = DeleteCardMJ(1, &byIndex);
		if (num <= 0 || byIndex == INVALID_VAL)
		{
			// 没牌可发了,荒庄
			m_wCurrentUser = INVALID_CHAIR;
			OnEventGameConclude(INVALID_CHAIR, NULL, GER_NORMAL);
			return false;
		}
		m_cbTouchCard = getValByIndex(byIndex);
		++m_cbCardIndex[wChairID][byIndex];

		if (m_wGangChairID != INVALID_CHAIR)
			++m_wGangCount;

		//清理数据		
		memset(m_dwActionDone, 0, sizeof(m_dwActionDone));		
	}

	DWORD dwActionFlag = WIK_ZIMO_HU | WIK_GANG_BU | WIK_GANG_AN;
	//检测当前玩家能进行的操作
	CheckCurPlayerOpt(wChairID, m_cbTouchCard, dwActionFlag);
	return true;
}
//检测当前玩家能进行的操作
bool CTableFrameSink::CheckCurPlayerOpt(WORD wChairID, BYTE byTouchCard, DWORD dwActionFlag, bool bSendMsg/*true*/)
{
	BYTE byTouchIndex = getIndexByVal(byTouchCard);
	if (wChairID >= m_cbPlayerCount || byTouchIndex == INVALID_VAL || m_cbMagicIndex >= MAX_INDEX)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d m_cbTouchCard=0x%02x m_cbMagicIndex=0x%02x", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount, byTouchCard, m_cbMagicIndex);
		
		return false;
	}

	m_dwActionUndo[wChairID] = 0;
	m_vctOptData[wChairID].clear();
	BYTE byMaxPiaoNum = 0;
	for (int i = 0; i < MAX_USER_NUM; ++i)
		byMaxPiaoNum = max(byMaxPiaoNum, m_byPiaoNum[i]);

	//能否胡
	if (dwActionFlag & WIK_ZIMO_HU && CHuPaiMJ::CheckHu(m_cbCardIndex[wChairID], byTouchIndex, m_cbIndexMShow, true))
	{
		bool bCanHu = true;
		// 注：双飘后飘家只能胡财神头的牌型，其余三家不能平胡和自摸，只能胡杠上开花一种牌型；
		if (byMaxPiaoNum >= 2)
		{
			bCanHu = false;
			DWORD dwHuFlag = CHuPaiMJ::GetHuFlag(m_cbCardIndex[wChairID], byTouchIndex, m_cbIndexMShow, true);
			if (m_wGangChairID != INVALID_CHAIR && m_wGangChairID == wChairID)
				dwHuFlag |= CHR_GANG_KAIHUA;

			if (m_byPiaoNum[wChairID] >= 2 && (dwHuFlag & CHR_NAIZI_JIANG))
				bCanHu = true;

			if (m_byPiaoNum[wChairID] < 2 && (dwHuFlag & CHR_GANG_KAIHUA))
				bCanHu = true;
		}
		else if (m_byPiaoNum[wChairID] == 1 || m_cbCardIndex[wChairID][m_cbMagicIndex] >= 3)		// 注：单漂、3赖子以上 只能胡财神头
		{
			bCanHu = false;
			DWORD dwHuFlag = CHuPaiMJ::GetHuFlag(m_cbCardIndex[wChairID], byTouchIndex, m_cbIndexMShow, true);
			if (dwHuFlag & CHR_NAIZI_JIANG)
				bCanHu = true;
		}
		if (bCanHu)
		{
			m_dwActionUndo[wChairID] |= WIK_ZIMO_HU;
			m_vctOptData[wChairID].push_back(stOptData(WIK_ZIMO_HU, byTouchCard));
		}			
	}
	//能否补杠
	if (dwActionFlag & WIK_GANG_BU)
	{
		for (int i = 0; i < min(MAX_WEAVE, m_cbWeaveCount[wChairID]); ++i)
		{
			if (m_WeaveItemArray[wChairID][i].dwWeaveKind == WIK_PENG)
			{
				BYTE byIndex = getIndexByVal(m_WeaveItemArray[wChairID][i].cbCenterCard);
				if (byIndex != INVALID_VAL && m_cbCardIndex[wChairID][byIndex] >= 1)
				{
					m_dwActionUndo[wChairID] |= WIK_GANG_BU;
					m_vctOptData[wChairID].push_back(stOptData(WIK_GANG_BU, getValByIndex(byIndex)));					
				}
			}
		}
	}
	//能否暗杠
	if (dwActionFlag & WIK_GANG_AN)
	{
		m_dwActionUndo[wChairID] |= CHuPaiMJ::CheckGangAn(m_cbCardIndex[wChairID], m_cbMagicIndex, m_vctOptData[wChairID]);
	}

	if (bSendMsg)
	{
		//检测听牌提示
		CheckTingMsg(wChairID);

		//发送操作消息
		int nLen = 0;
		BYTE byData[2048] = {};
		CMD_S_SendCard cmdMsg;
		cmdMsg.wChairID = wChairID;
		cmdMsg.byCard = byTouchCard;
		cmdMsg.byLeftNum = m_cbLeftShowCount;
		cmdMsg.byOptCount = m_vctOptData[wChairID].size();		

		memcpy(byData + nLen, &cmdMsg, sizeof(cmdMsg));
		nLen += sizeof(cmdMsg);
		if (m_vctOptData[wChairID].size() > 0)
		{
			memcpy(byData + nLen, &m_vctOptData[wChairID][0], sizeof(stOptData)*m_vctOptData[wChairID].size());
			nLen += sizeof(stOptData)*m_vctOptData[wChairID].size();
		}

		//发送数据
		m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_SEND_CARD, byData, nLen);
		m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_SEND_CARD, byData, nLen);
		m_gameVideo.AddVideoData(SUB_S_SEND_CARD, byData, sizeof(cmdMsg));
	}
	//托管
// 	if (m_bTrustee[wChairID])
// 		m_pITableFrame->SetGameTimer(IDI_OUT_CARD, 1000, 1, 0);

	return true;
}

//检测其他玩家是否能对出的牌进行操作
bool CTableFrameSink::CheckOtherPlayerOpt(WORD wOutChairID, BYTE byOutCard, DWORD dwActionFlag)
{	
	bool bOtherCanOpt = false;
	memset(m_dwActionUndo, 0, sizeof(m_dwActionUndo));

	BYTE byMaxPiaoNum = 0;
	for (int i = 0; i < MAX_USER_NUM; ++i)
		byMaxPiaoNum = max(byMaxPiaoNum, m_byPiaoNum[i]);
	
	int nNextChairID = (wOutChairID + 1) % m_cbPlayerCount;	
	for (int i = 0; i < m_cbPlayerCount; i++)
	{
		m_vctOptData[i].clear();
		if (wOutChairID == i) continue;
		//能否点炮胡
		if ((dwActionFlag & WIK_PAO_HU) && CHuPaiMJ::CheckHu(m_cbCardIndex[i], getIndexByVal(byOutCard), m_cbIndexMShow, false))
		{
			bool bQiangGangHu = (m_wGangChairID != INVALID_CHAIR && m_wGangChairID != i);
			bool bCanHu = true;
			// 注：双飘后飘家只能胡财神头的牌型，其余三家不能平胡和自摸，只能胡杠上开花一种牌型；
			if (byMaxPiaoNum >= 2 || m_byPiaoNum[i] > 0)
			{
				bCanHu = false;
			}
			else if (byMaxPiaoNum == 1)
			{
				//注：单飘后飘家只能胡财神头的牌型，其余三家不能放铳胡只能胡2倍及以上牌型。(抢杠胡除外)
				if (bQiangGangHu == false)
					bCanHu = false;
			}

			//注：当玩家手上拥有2个财神，不进行单飘，只胡普通的2倍牌型时，也不允许放铳胡（不包括抢杠胡）。
			if (m_byPiaoNum[i] == 0 && m_cbCardIndex[i][m_cbMagicIndex] >= 2)
			{
				if (bQiangGangHu == false)
					bCanHu = false;
			}				

			// 注：财神头牌型只能自摸
			if (!bQiangGangHu && CHuPaiMJ::isJiangNaiZi2(m_cbCardIndex[i], m_cbMagicIndex))
				bCanHu = false;
			
			if (bCanHu)
			{
				m_vctOptData[i].push_back(stOptData(WIK_PAO_HU, byOutCard));
				m_dwActionUndo[i] |= WIK_PAO_HU;
			}				
		}
		bool bCanChiPeng = true;
		//特殊处理：如果玩家双漂且两个赖子+两张牌，则不允许吃碰操作，因为吃碰后就出不了牌了
		if (m_byPiaoNum[i] >= 2 && m_cbCardIndex[i][m_cbMagicIndex] == 2)
		{
			BYTE byCardNum = 0;
			for (int n=0; n<MAX_INDEX; ++n)
				byCardNum += m_cbCardIndex[i][n];

			if (byCardNum == 4)
				bCanChiPeng = false;
		}

		//能否吃
		if ((dwActionFlag & WIK_CHI) && nNextChairID == i && bCanChiPeng)
		{			
			m_dwActionUndo[i] |= CHuPaiMJ::CheckChi(m_cbCardIndex[i], byOutCard, m_cbMagicIndex, m_vctOptData[i]);
		}
		//能否碰
		if (dwActionFlag & WIK_PENG && bCanChiPeng)
		{
			DWORD dwAction = CHuPaiMJ::CheckPeng(m_cbCardIndex[i], byOutCard, m_cbMagicIndex);
			if (dwAction > 0)
			{
				m_dwActionUndo[i] |= dwAction;
				m_vctOptData[i].push_back(stOptData(WIK_PENG, byOutCard));
			}
		}
		//能否明杠（点杠）
		if (dwActionFlag & WIK_GANG_MING)
		{
			DWORD dwAction = CHuPaiMJ::CheckGangDian(m_cbCardIndex[i], byOutCard, m_cbMagicIndex);
			if (dwAction > 0)
			{
				m_dwActionUndo[i] |= dwAction;
				m_vctOptData[i].push_back(stOptData(WIK_GANG_MING, byOutCard));
			}
		}
		
		if (m_dwActionUndo[i] > 0)
		{
			bOtherCanOpt = true;
			int  nLen = 0;
			BYTE byData[2048] = {};
			//通知玩家操作
			CMD_S_OperateNotify OperateNotify;
			OperateNotify.wProvideUser = wOutChairID;
			OperateNotify.cbActionCard = byOutCard;
			OperateNotify.byOptCount = m_vctOptData[i].size();

			memcpy(byData + nLen, &OperateNotify, sizeof(OperateNotify));
			nLen += sizeof(OperateNotify);
			if (m_vctOptData[i].size() > 0)
			{
				memcpy(byData + nLen, &m_vctOptData[i][0], sizeof(stOptData)*m_vctOptData[i].size());
				nLen += sizeof(stOptData)*m_vctOptData[i].size();
			}			

			//发送数据
			m_pITableFrame->SendTableData(i, SUB_S_OPERATE_NOTIFY, byData, nLen);
			m_pITableFrame->SendLookonData(i, SUB_S_OPERATE_NOTIFY, byData, nLen);
			//m_gameVideo.AddVideoData(SUB_S_OPERATE_NOTIFY, byData, sizeof(OperateNotify));

// 			KillGameTimer(TIME_PLAYER_1 + i);
// 			SetGameTimer(TIME_PLAYER_1 + i, nDelayTime);
		}
	}
// 	if (bOtherHasOperates)
// 	{
//  	KillGameTimer(TIME_ONE_COUNT);
//  	SetGameTimer(TIME_ONE_COUNT, 1000);
// 	}
	return bOtherCanOpt;
}

//清理一把游戏数据
void CTableFrameSink::ResetOneTurnInfo()
{
	//游戏变量
	m_wSiceCount = 0;
	m_wCurrentUser = INVALID_CHAIR;
	m_cbMagicIndex = INVALID_VAL;
	m_cbIndexMShow = INVALID_VAL;
	m_cbLeftCardCount = 0;
	m_cbLeftShowCount = 0;
	m_cbLastOutCard = INVALID_VAL;
	m_cbLastOutUser = INVALID_CHAIR;
	m_cbTouchCard = INVALID_VAL;
	m_wGangChairID = INVALID_CHAIR;
	m_wGangCount = 0;
	m_iCurOptStatus = enStationOpt_doing;

	//扑克信息
	memset(m_cbCardIndex, 0, sizeof(m_cbCardIndex));
	memset(m_bTrustee, 0, sizeof(m_bTrustee));
	memset(m_bPlayStatus, 0, sizeof(m_bPlayStatus));

	memset(m_cbWeaveCount, 0, sizeof(m_cbWeaveCount));
	memset(m_WeaveItemArray, 0, sizeof(m_WeaveItemArray));
	memset(m_cbOutNum, 0, sizeof(m_cbOutNum));
	memset(m_cbOutCards, 0, sizeof(m_cbOutCards));

	memset(m_dwActionUndo, 0, sizeof(m_dwActionUndo));
	memset(m_dwActionDone, 0, sizeof(m_dwActionDone));
	for (int i = 0; i < MAX_USER_NUM; ++i)
		m_vctOptData[i].clear();

	memset(m_cbOptCard, 0, sizeof(m_cbOptCard));
	memset(m_byPiaoNum, 0, sizeof(m_byPiaoNum));
	memset(m_isCanOptZi, 0, sizeof(m_isCanOptZi));
	memset(m_cbChengBao1, 0, sizeof(m_cbChengBao1));
	memset(m_cbChengBao2, 0, sizeof(m_cbChengBao2));	
}

//清理整场游戏数据
void CTableFrameSink::ResetAllTurnInfo()
{
	ResetOneTurnInfo();
	
	m_dwCurHuFlag = 0;
	ZeroMemory(&m_stRecord, sizeof(m_stRecord));
	memset(m_bTrustee, 0, sizeof(m_bTrustee));
	memset(m_byBuyZi, 0, sizeof(m_byBuyZi)); 
	memset(m_byXiaZi, 0, sizeof(m_byXiaZi));
	memset(m_llScore, 0, sizeof(m_llScore));
	memset(m_llScoreAll, 0, sizeof(m_llScoreAll));
	memset(m_llScoreAllSpe, 0, sizeof(m_llScoreAllSpe));
}

//吃牌
bool CTableFrameSink::OnMsgAction_EatCard(WORD wChairID, BYTE cbCard[3], DWORD dwAction)
{	
	BYTE byIndex[3] = { INVALID_VAL, INVALID_VAL, INVALID_VAL };
	BYTE byCor[3] = { INVALID_VAL, INVALID_VAL, INVALID_VAL };
	for (int i = 0; i < 3; ++i)
	{
		byIndex[i] = getIndexByVal(cbCard[i]);
		byCor[i] = byIndex[i]/9;
	}	
	BYTE byMinDis = min(abs(byIndex[1] - byIndex[2]), min(abs(byIndex[0] - byIndex[1]), abs(byIndex[0] - byIndex[2])));
	BYTE byMaxDis = max(abs(byIndex[1] - byIndex[2]), max(abs(byIndex[0] - byIndex[1]), abs(byIndex[0] - byIndex[2])));

	//对消息内容进行校验：非法值？不同花色？风字牌？不是连牌？
	if (wChairID >= m_cbPlayerCount || byIndex[0] == INVALID_VAL || byIndex[1] == INVALID_VAL || byIndex[2] == INVALID_VAL
		|| byCor[0] != byCor[1] || byCor[0] != byCor[2] || byCor[0] == enCor_FenZi || byMinDis != 1 || byMaxDis != 2)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d cbCard[0]=0x%02x cbCard[1]=0x%02x cbCard[2]=0x%02x", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount, cbCard[0], cbCard[1], cbCard[2]);
		return false;
	}
	if (m_cbCardIndex[wChairID][byIndex[1]] < 1 || m_cbCardIndex[wChairID][byIndex[2]] < 1)
	{
		PrintLog(TraceLevel_Warning, "%s:%d cbCard[1] num = %d  cbCard[2] num = %d", 
			__FUNCTION__, __LINE__, m_cbCardIndex[wChairID][byIndex[1]], m_cbCardIndex[wChairID][byIndex[2]]);
		return false;
	}	
	// 权限判断
	if ((m_dwActionUndo[wChairID] & dwAction) == 0 || enStationOpt_outCard != m_iCurOptStatus)
	{
		PrintLog(TraceLevel_Warning, "%s:%d m_dwActionUndo[wChairID] = %d  dwAction = %d  m_iCurOptStatus=%d", 
			__FUNCTION__, __LINE__, m_dwActionUndo[wChairID], dwAction, m_iCurOptStatus);

		return false;
	}

	m_dwActionUndo[wChairID] = 0;
	m_dwActionDone[wChairID] = dwAction;
	m_vctOptData[wChairID].clear();
	memcpy(m_cbOptCard[wChairID], cbCard, 3);
	CheckDoWaitOperate(wChairID);
	return true;
}
//碰牌
bool CTableFrameSink::OnMsgAction_PengCard(WORD wChairID, BYTE cbCard[3])
{
	BYTE byIndex = getIndexByVal(cbCard[0]);
	if (wChairID >= m_cbPlayerCount || byIndex == INVALID_VAL || m_cbCardIndex[wChairID][byIndex] < 2)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d cbCard[0]=%02x num=%d", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount, cbCard[0], byIndex<MAX_INDEX ? m_cbCardIndex[wChairID][byIndex] : 0);
		return false;
	}
	// 权限判断
	if ((m_dwActionUndo[wChairID] & WIK_PENG) == 0 || enStationOpt_outCard != m_iCurOptStatus)
	{		
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=%x m_iCurOptStatus=%d",
			__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID], m_iCurOptStatus);
		return false;
	}
	m_dwActionUndo[wChairID] = 0;
	m_dwActionDone[wChairID] = WIK_PENG;
	m_vctOptData[wChairID].clear();
	memset(m_cbOptCard[wChairID], cbCard[0], 3);
	CheckDoWaitOperate(wChairID);
	return true;
}
//杠--点杠(明杠)
bool CTableFrameSink::OnMsgAction_DianGang(WORD wChairID, BYTE cbCard[3])
{
	BYTE byIndex = getIndexByVal(cbCard[0]);
	if (wChairID >= m_cbPlayerCount || byIndex == INVALID_VAL || m_cbCardIndex[wChairID][byIndex] < 3)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d cbCard[0]=%02x num=%d", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount, cbCard[0], byIndex<MAX_INDEX ? m_cbCardIndex[wChairID][byIndex] : 0);
		return false;
	}
	// 权限判断
	if ((m_dwActionUndo[wChairID] & WIK_GANG_MING) == 0 || enStationOpt_outCard != m_iCurOptStatus)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=0x%x m_iCurOptStatus=%d", 
			__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID], m_iCurOptStatus);
		return false;
	}
	m_dwActionUndo[wChairID] = 0;
	m_dwActionDone[wChairID] = WIK_GANG_MING;
	m_vctOptData[wChairID].clear();
	memset(m_cbOptCard[wChairID], cbCard[0], 3);
	CheckDoWaitOperate(wChairID);
	return true;
}
//杠--补杠
bool CTableFrameSink::OnMsgAction_BuGang(WORD wChairID, BYTE cbCard[3])
{
	BYTE byIndex = getIndexByVal(cbCard[0]);
	if (wChairID != m_wCurrentUser || byIndex == INVALID_VAL || m_cbCardIndex[wChairID][byIndex] < 1)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_wCurrentUser=%d cbCard[0]=0x%02x num=%d",
			__FUNCTION__, __LINE__, wChairID, m_wCurrentUser, cbCard[0], byIndex<MAX_INDEX ? m_cbCardIndex[wChairID][byIndex] : 0);
		return false;
	}
	// 权限判断
	if (m_dwActionUndo[wChairID] & WIK_GANG_BU && enStationOpt_doing == m_iCurOptStatus)
	{		
		// 执行动作
		bool bSuc = AddWeaveItemArray(wChairID, WIK_GANG_BU, cbCard);
		if (bSuc)
		{
			m_cbCardIndex[wChairID][byIndex] -= 1;
			m_dwActionUndo[wChairID] = 0;
			m_vctOptData[wChairID].clear();
			SendActionResult(wChairID, cbCard, WIK_GANG_BU);
			// 抢杠胡？
			if (CheckOtherPlayerOpt(wChairID, cbCard[0], WIK_PAO_HU))
			{
				m_cbLastOutCard = cbCard[0];
				m_iCurOptStatus = enStationOpt_outCard;  //等待其他玩家操作
			}
			else
			{
				TurnToStation(wChairID, true);
			}			
		}			
		return bSuc;
	}	
	PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=%x m_iCurOptStatus=%d",
		__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID], m_iCurOptStatus);
	return false;
}
//杠--暗杠
bool CTableFrameSink::OnMsgAction_AnGang(WORD wChairID, BYTE cbCard[3])
{
	BYTE byIndex = getIndexByVal(cbCard[0]);
	if (wChairID != m_wCurrentUser || byIndex == INVALID_VAL || m_cbCardIndex[wChairID][byIndex] < 4 || byIndex == m_cbMagicIndex)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_wCurrentUser=%d m_cbMagicIndex=0x%02x cbCard[0]=0x%02x num=%d",
			__FUNCTION__, __LINE__, wChairID, m_wCurrentUser, m_cbMagicIndex, cbCard[0], byIndex<MAX_INDEX ? m_cbCardIndex[wChairID][byIndex] : 0);
		return false;
	}
	// 权限判断
	if (m_dwActionUndo[wChairID] & WIK_GANG_AN && enStationOpt_doing == m_iCurOptStatus)
	{
		// 执行动作
		bool bSuc = AddWeaveItemArray(wChairID, WIK_GANG_AN, cbCard);
		if (bSuc)
		{
			m_cbCardIndex[wChairID][byIndex] -= 4;
			m_dwActionUndo[wChairID] = 0;
			m_vctOptData[wChairID].clear();
			SendActionResult(wChairID, cbCard, WIK_GANG_AN);	
			TurnToStation(wChairID, true);
		}
		return bSuc;
	}
	PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=0x%x m_iCurOptStatus=%d", 
		__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID], m_iCurOptStatus);
	return false;
}
//胡--自摸
bool CTableFrameSink::OnMsgAction_ZiMoHu(WORD wChairID, BYTE cbCard[3])
{
	BYTE byTouchIndex = getIndexByVal(m_cbTouchCard);
	if (wChairID != m_wCurrentUser || byTouchIndex == INVALID_VAL)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_wCurrentUser=%d m_cbTouchCard=0x%02x", 
			__FUNCTION__, __LINE__, wChairID, m_wCurrentUser, m_cbTouchCard);
		return false;
	}
	DWORD nHuFlag = CHuPaiMJ::CheckHu(m_cbCardIndex[wChairID], byTouchIndex, m_cbIndexMShow, true);
	// 权限判断
	if (m_dwActionUndo[wChairID] & WIK_ZIMO_HU && enStationOpt_doing == m_iCurOptStatus && nHuFlag != 0)
	{
		m_dwActionUndo[wChairID] = 0;
		m_dwActionDone[wChairID] = WIK_ZIMO_HU;
		m_vctOptData[wChairID].clear();
		m_cbLastOutCard = m_cbTouchCard;
		OnEventGameConclude(wChairID, NULL, GER_NORMAL);
		return true;
	}
	PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=0x%x m_iCurOptStatus=%d",
		__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID], m_iCurOptStatus);
	return false;
}
//胡--点炮胡
bool CTableFrameSink::OnMsgAction_DianPaoHu(WORD wChairID, BYTE cbCard[3])
{
	BYTE byIndex = getIndexByVal(cbCard[0]);
	if (wChairID >= m_cbPlayerCount || byIndex == INVALID_VAL || cbCard[0] != m_cbLastOutCard)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d m_cbLastOutCard=0x%02x cbCard[0]=0x%02x", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount, m_cbLastOutCard, cbCard[0]);
		return false;
	}
	// 权限判断
	if ((m_dwActionUndo[wChairID] & WIK_PAO_HU) == 0 || enStationOpt_outCard != m_iCurOptStatus)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=0x%x m_iCurOptStatus=%d",
			__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID], m_iCurOptStatus);
		return false;
	}
	m_dwActionUndo[wChairID] = 0;
	m_dwActionDone[wChairID] = WIK_PAO_HU;
	m_vctOptData[wChairID].clear();
	memcpy(m_cbOptCard[wChairID], cbCard, 3);
	CheckDoWaitOperate(wChairID);
	return true;
}
//过(放弃)
bool CTableFrameSink::OnMsgAction_GiveUp(WORD wChairID, BYTE cbCard[3])
{
	if (wChairID >= m_cbPlayerCount)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount);
		return false;
	}
	// 权限判断
	if (m_dwActionUndo[wChairID] == 0)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_dwActionUndo[wChairID]=0x%x", 
			__FUNCTION__, __LINE__, wChairID, m_dwActionUndo[wChairID]);
		return false;
	}
	m_dwActionUndo[wChairID] = 0;
	m_dwActionDone[wChairID] = WIK_NULL;
	m_vctOptData[wChairID].clear();
	if (m_iCurOptStatus == enStationOpt_outCard)
		CheckDoWaitOperate(wChairID);

	return true;
}
//加桌面牌组（吃、碰、杠 成功后）
bool CTableFrameSink::AddWeaveItemArray(WORD wChairID, DWORD dwAction, BYTE cbCard[3])
{
	if (wChairID >= m_cbPlayerCount || dwAction <= 0 || m_cbWeaveCount[wChairID] >= MAX_WEAVE)
	{
		PrintLog(TraceLevel_Warning, "%s:%d wChairID=%d m_cbPlayerCount=%d dwAction=0x%x", 
			__FUNCTION__, __LINE__, wChairID, m_cbPlayerCount, dwAction);
		return false;
	}
	// 补杠特殊处理
	if (dwAction == WIK_GANG_BU)
	{
		for (int i = 0; i < m_cbWeaveCount[wChairID]; ++i)
		{
			if (m_WeaveItemArray[wChairID][i].dwWeaveKind == WIK_PENG && m_WeaveItemArray[wChairID][i].cbCenterCard == cbCard[0])
			{
				m_WeaveItemArray[wChairID][i].dwWeaveKind = WIK_GANG_BU;
				m_WeaveItemArray[wChairID][i].cbCardData[3] = cbCard[0];
				return true;
			}
		}
		PrintLog(TraceLevel_Warning, "%s:%d WIK_GANG_BU failed", __FUNCTION__, __LINE__);
		return false;
	}
	else
	{
		tagWeaveItem &weaveItem = m_WeaveItemArray[wChairID][m_cbWeaveCount[wChairID]];
		weaveItem.dwWeaveKind = dwAction;
		weaveItem.cbCenterCard = m_cbLastOutCard;
		weaveItem.wProvideUser = m_cbLastOutUser;
		if (dwAction == WIK_GANG_AN || dwAction == WIK_GANG_MING)
			memset(weaveItem.cbCardData, cbCard[0], 4);
		else if (dwAction == WIK_PENG)
			memset(weaveItem.cbCardData, cbCard[0], 3);
		else
			memcpy(weaveItem.cbCardData, cbCard, 3);

		++m_cbWeaveCount[wChairID];		
		return true;
	}	
	PrintLog(TraceLevel_Warning, "%s:%d dwAction failed", __FUNCTION__, __LINE__);
	return false;
}
//用户操作
bool CTableFrameSink::OnUserOperateCard(WORD wChairID, DWORD dwAction, BYTE cbCard[3])
{	
	if (dwAction == WIK_NULL)
	{
		return OnMsgAction_GiveUp(wChairID, cbCard);
	}
	else if (dwAction == WIK_CHI)
	{
		return OnMsgAction_EatCard(wChairID, cbCard, dwAction);
	}
	else if (dwAction == WIK_PENG)
	{
		return OnMsgAction_PengCard(wChairID, cbCard);
	}
	else if (dwAction == WIK_GANG_MING)
	{
		return OnMsgAction_DianGang(wChairID, cbCard);
	}
	else if (dwAction == WIK_GANG_BU)
	{
		return OnMsgAction_BuGang(wChairID, cbCard);
	}
	else if (dwAction == WIK_GANG_AN)
	{
		return OnMsgAction_AnGang(wChairID, cbCard);
	}
	else if (dwAction == WIK_ZIMO_HU)
	{
		return OnMsgAction_ZiMoHu(wChairID, cbCard);
	}
	else if (dwAction == WIK_PAO_HU)
	{
		return OnMsgAction_DianPaoHu(wChairID, cbCard);
	}
	else
	{
		PrintLog(TraceLevel_Warning, "%s:%d cbAction = 0x%x", __FUNCTION__, __LINE__, dwAction);
		return false;
	}	
}

//用户托管
bool CTableFrameSink::OnUserTrustee(WORD wChairID, bool bTrustee)
{
	//效验状态
	ASSERT((wChairID < m_cbPlayerCount));
	if ((wChairID>=m_cbPlayerCount)) return false;

// 	if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) != 0)//房卡模式
// 		return true;

	m_bTrustee[wChairID] = bTrustee;

	CMD_S_Trustee Trustee;
	Trustee.bTrustee = bTrustee;
	Trustee.wChairID = wChairID;
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &Trustee,sizeof(Trustee));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_TRUSTEE, &Trustee,sizeof(Trustee));
	m_gameVideo.AddVideoData(SUB_S_TRUSTEE, &Trustee, sizeof(Trustee));
	
//	if(bTrustee)
	return true;
}

//用户买子下子
bool CTableFrameSink::OnMsgUserBuyXiaZi(WORD wChairID, bool bBuyXiaZi)
{
	if ((wChairID >= m_cbPlayerCount)) 
		return false;
	if (!m_isCanOptZi[wChairID])
		return true;

	BYTE byXiaZiVal = 0;
	BYTE byDoType = m_byBuyZi[wChairID]==0 ? 0 : 1;							//1:下子操作 0：买子操作
	if (bBuyXiaZi)
	{
		if (m_byBuyZi[wChairID] == 0)
		{
			byXiaZiVal = SCORE_BUY_ZI;
			m_byBuyZi[wChairID] = SCORE_BUY_ZI;
		}			
		else if (m_byXiaZi[wChairID] == 0 && wChairID != m_wBankerUser)	//庄家不能下子
		{
			m_byXiaZi[wChairID] = CHuPaiMJ::GetHuRate(m_dwCurHuFlag);
			byXiaZiVal = CHuPaiMJ::GetHuRate(m_dwCurHuFlag);
		}			
	}
	m_isCanOptZi[wChairID] = false;
	// pino 广播操作结果 SUB_S_BUY_XIA_ZI_RESULT
	CMD_S_BuyXiaZiResult cmdResult;
	cmdResult.wChairID = wChairID;
	cmdResult.byDoType = byDoType;
	cmdResult.byXiaZiVal = byXiaZiVal;
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_BUY_XIA_ZI_RESULT, &cmdResult, sizeof(cmdResult));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_BUY_XIA_ZI_RESULT, &cmdResult, sizeof(cmdResult));
	m_gameVideo.AddVideoData(SUB_S_BUY_XIA_ZI_RESULT, &cmdResult, sizeof(cmdResult));
	
	BYTE byWaitNum = 0;
	for (int i = 0; i < MAX_USER_NUM; ++i)
	{
		if (m_isCanOptZi[i]) 
			++byWaitNum;
	}
	if (byWaitNum == 0)
		OnSendCards();

	return true;
}

//发牌
void CTableFrameSink::OnSendCards()
{
	m_pITableFrame->SetGameStatus(GAME_SCENE_PLAY); 
	//随机扑克
	BYTE Sice1 = rand() % 6 + 1;
	BYTE Sice2 = rand() % 6 + 1;
	m_wSiceCount = MAKEWORD(Sice1, Sice2);

	m_iCurOptStatus = enStationOpt_doing;
	memcpy(m_cbRepertoryCard, s_cbCardsArray, MAX_ALL_NUM);
	//洗牌 (3百搭时，少一个赖子牌:0x37)
	m_cbLeftCardCount = (m_nPlayMode & PlayMode_Magic_3) ? MAX_ALL_NUM - 1: MAX_ALL_NUM;
	random_shuffle(m_cbRepertoryCard, m_cbRepertoryCard + m_cbLeftCardCount);

	//配牌 pino
	BYTE byTestCards[MAX_ALL_NUM] = {};
	int num = getTestCards("configcard/test_YWMJ.txt", byTestCards, MAX_ALL_NUM);
	if (num == MAX_ALL_NUM)
		memcpy(m_cbRepertoryCard, byTestCards, MAX_ALL_NUM);

	//将牌库牌面值转成索引值
	for (int i = 0; i < MAX_ALL_NUM; ++i)
		m_cbRepertoryCard[i] = getIndexByVal(m_cbRepertoryCard[i]);	

	m_wCurrentUser = m_wBankerUser;
	//m_nPlayMode = PlayMode_Magic_3 | PlayMode_IsCenBao | PlayMode_IsDeFen7 | PlayMode_CanDianHu | PlayMode_DaKun;
	m_cbMagicIndex = 33;
	m_cbIndexMShow = m_cbRepertoryCard[rand() % m_cbLeftCardCount];
	m_cbLeftShowCount = m_cbLeftCardCount;
//	m_cbIndexMShow = 33;	

	//分发扑克
	BYTE byCardIndex[MAX_COUNT];
	memset(m_bPlayStatus, 1, sizeof(m_bPlayStatus));
	for (int i = 0; i < m_cbPlayerCount; i++)
	{
		int iFaPaiNum = (i == m_wBankerUser) ? MAX_COUNT : MAX_COUNT - 1;
		//int iFaPaiNum = MAX_COUNT - 1;
		memset(byCardIndex, INVALID_VAL, sizeof(byCardIndex));
		int num = DeleteCardMJ(iFaPaiNum, byCardIndex);
		if (num <= 0)
			return;

		for (int n = 0; n < iFaPaiNum; ++n)
			++m_cbCardIndex[i][byCardIndex[n]];

		if (iFaPaiNum == MAX_COUNT)		//检测庄家能进行的操作
		{
			m_cbTouchCard = getValByIndex(byCardIndex[MAX_COUNT - 1]);
			CheckCurPlayerOpt(i, m_cbTouchCard, WIK_ZIMO_HU | WIK_GANG_AN, false);
		}		
	}	
	// 2人24张 3人36张 4人48张 
	m_cbLeftShowCount = 12 * m_cbPlayerCount;

	//发送百搭牌
	//pino 这里发索引还是牌值?
	BYTE byMagicCards = getValByIndex(m_cbIndexMShow);
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_MAGIC_CARD, &byMagicCards, 1);
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_MAGIC_CARD, &byMagicCards, 1);
	m_gameVideo.AddVideoData(SUB_S_MAGIC_CARD, &byMagicCards, 1);

	//构造变量
	int nLen = 0;
	BYTE byData[2048] = {};
	CMD_S_GameStart GameStart;
	GameStart.wBankerUser = m_wBankerUser;
	GameStart.wSiceCount = m_wSiceCount;
	GameStart.byLeftNum = m_cbLeftShowCount;
	//发送数据
	for (WORD i = 0; i < m_cbPlayerCount; i++)
	{
		nLen = 0;
		memset(GameStart.cbCardData, 0, sizeof(GameStart.cbCardData));
		IServerUserItem *pServerUserItem = m_pITableFrame->GetTableUserItem(i);
		if (pServerUserItem == NULL)
			continue;

		GetHandleCards(i, GameStart.cbCardData);
		GameStart.byOptCount = m_vctOptData[i].size();
		memcpy(byData + nLen, &GameStart, sizeof(GameStart));
		nLen += sizeof(GameStart);
		if (m_vctOptData[i].size() > 0)
		{
			memcpy(byData + nLen, &m_vctOptData[i][0], sizeof(stOptData)*m_vctOptData[i].size());
			nLen += sizeof(stOptData)*m_vctOptData[i].size();
		}
		m_pITableFrame->SendTableData(i, SUB_S_GAME_START, byData, nLen);
		m_pITableFrame->SendLookonData(i, SUB_S_GAME_START, byData, nLen);

		Video_GameStart video;
		lstrcpyn(video.szNickName, pServerUserItem->GetNickName(), CountArray(video.szNickName));
		video.wChairID = i;
		video.lScore = pServerUserItem->GetUserScore();
		video.lCellScore = m_pITableFrame->GetCellScore();
		video.dwPlayMode = m_nPlayMode;
		video.byUserCount = m_cbPlayerCount;
		video.cmdPlayStart = GameStart;

		memcpy(byData, &video, sizeof(video));
		m_gameVideo.AddVideoData(SUB_S_GAME_START, byData, sizeof(video), i == 0);
	}
}

//算分
BYTE CTableFrameSink::CalGameScore(CMD_S_GameConclude &GameConclude, bool &bScoreSpe)
{
	SCORE lCellScore = m_pITableFrame->GetCellScore();	
	int nVal = CHuPaiMJ::GetHuRate(m_dwCurHuFlag);
	//荒庄 //当玩家实行双飘后流局，需要赔付剩余三家每个玩家4倍牌型分
	WORD wHuPaiChair = GameConclude.wHuUser;
	if (GameConclude.wHuUser == INVALID_CHAIR)
	{
		for (int i = 0; i < m_cbPlayerCount; ++i)
		{
			if (m_byPiaoNum[i] == 2)
			{
				wHuPaiChair = i;
				nVal = -4;			//赔付每个玩家4倍
				bScoreSpe = true;
			}
		}
	}
	//胡牌玩家的承包者(可能有多个)两口承包、三口承包
	set<WORD> setChengBao2, setChengBao3;
	if (wHuPaiChair < MAX_USER_NUM && nVal > 0)
	{
		for (WORD i = 0; i < m_cbPlayerCount; ++i)
		{
			if (m_cbChengBao2[wHuPaiChair][i] >= 2 || m_cbChengBao2[i][wHuPaiChair] >= 2)
				setChengBao2.insert(i);
			else if (m_cbChengBao1[wHuPaiChair][i] + m_cbChengBao2[wHuPaiChair][i] >= 3 ||
				m_cbChengBao1[i][wHuPaiChair] + m_cbChengBao2[i][wHuPaiChair] >= 3 )
			{
				if (m_nPlayMode & PlayMode_IsCenBao)	//三口承包玩法
					setChengBao3.insert(i);
			}
		}
	}	
	BYTE byChengBaoNum = setChengBao2.size() + setChengBao3.size();
	if (byChengBaoNum > 0)
		bScoreSpe = true;

	//胡牌玩家是否有两口承包关系
	if (wHuPaiChair < MAX_USER_NUM && m_byPiaoNum[wHuPaiChair] == 0 && nVal > 0)	//漂了就不翻倍了
	{
		if (setChengBao2.size() > 0)
		{
			nVal *= 2;
			GameConclude.bFanChenBao = true;
		}		
	}	

	if (nVal != 0 && wHuPaiChair < MAX_USER_NUM)	//有人胡牌 m_dwLastHuFlag
	{
		LONGLONG nAllScore = 0;
		for (int i = 0; i < m_cbPlayerCount; i++)
		{
			IServerUserItem *pServerUserItem = m_pITableFrame->GetTableUserItem(i);
			if (i == wHuPaiChair || pServerUserItem == NULL) continue;
			// 分数 = (基础分数+买子+下子) * 倍数
			int nBaseScore = (m_nPlayMode&PlayMode_IsDeFen7) ? SCORE_DI_7 : SCORE_DI_11;
			int nZiScore = m_byBuyZi[wHuPaiChair] + m_byBuyZi[i];
			// 庄胡
			if (wHuPaiChair == m_wBankerUser)
			{
				nBaseScore += SCORE_DI_ZHUANG;
				nZiScore += m_byXiaZi[i];
			}
			else if (i == m_wBankerUser)
			{
				nZiScore += m_byXiaZi[wHuPaiChair];
				nBaseScore += SCORE_DI_ZHUANG;
			}	
			if (i == GameConclude.wPaoUser)
				nBaseScore += SCORE_DI_PAO;

			LONGLONG nScore = (nBaseScore + nZiScore) * nVal * lCellScore;
			if (byChengBaoNum == 0)		//无承包
			{
				// 打捆模式，不能有负数积分
				if (m_nPlayMode & PlayMode_DaKun && pServerUserItem->GetUserScore() < nScore)
					nScore = pServerUserItem->GetUserScore();

				GameConclude.lGameScore[i] = -nScore;
			}			
			nAllScore += nScore;
		}
		if (byChengBaoNum == 0)
		{
			GameConclude.lGameScore[wHuPaiChair] = nAllScore;
		}			
		else
		{
			LONGLONG nAllWin = 0;
			// 承包
			set<WORD>::iterator iter = setChengBao2.begin();
			for (; iter != setChengBao2.end(); ++iter)
			{
				IServerUserItem *pUserItem = m_pITableFrame->GetTableUserItem(*iter);
				if (pUserItem == NULL) continue;
				// 打捆模式，不能有负数积分
				if (m_nPlayMode & PlayMode_DaKun && pUserItem->GetUserScore() < nAllScore)
					nAllScore = pUserItem->GetUserScore();

				GameConclude.bChengBao[*iter] = true;
				GameConclude.lGameScore[*iter] = -nAllScore;
				nAllWin += nAllScore;
			}

			iter = setChengBao3.begin();
			for (; iter != setChengBao3.end(); ++iter)
			{
				IServerUserItem *pUserItem = m_pITableFrame->GetTableUserItem(*iter);
				if (pUserItem == NULL) continue;
				// 打捆模式，不能有负数积分
				if (m_nPlayMode & PlayMode_DaKun && pUserItem->GetUserScore() < nAllScore)
					nAllScore = pUserItem->GetUserScore();

				GameConclude.bChengBao[*iter] = true;
				GameConclude.lGameScore[*iter] = -nAllScore;
				nAllWin += nAllScore;
			}
			GameConclude.lGameScore[wHuPaiChair] = nAllWin;
		}
	}	
	return 1;
}

//设置基数
void CTableFrameSink::SetGameBaseScore(LONG lBaseScore)
{
	//发送数据
	m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_SET_BASESCORE,&lBaseScore,sizeof(lBaseScore));
	m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_SET_BASESCORE,&lBaseScore,sizeof(lBaseScore));
}

void CTableFrameSink::CheckTingMsg(WORD wChairID)
{	
	if (wChairID < m_cbPlayerCount)
	{
		DWORD dwTickFlag = GetTickCount();
		vector<stTingData> vctTingOut;
		BYTE byShowIndex[MAX_INDEX] = {};
		GetShowCardsInfo(wChairID, byShowIndex);
		bool bSuc = CHuPaiMJ::GetTingData(m_cbCardIndex[wChairID], byShowIndex, m_WeaveItemArray[wChairID], m_cbIndexMShow, vctTingOut);
		/*/////////////////////////////性能测试代码///////////////////////////////
		for (int i = 0; i < 1000; ++i)
			bSuc = CHuPaiMJ::GetTingData(m_cbCardIndex[wChairID], byShowIndex, m_WeaveItemArray[wChairID], m_cbIndexMShow, vctTingOut);
		PrintLog(TraceLevel_Warning, "ting cost:%d ms", GetTickCount()-dwTickFlag);
		////////////////////////////////////////////////////////////////////////*/
		if (bSuc)
		{
			int  nLen = 0;
			BYTE byData[2048] = {};
			CMD_S_TING_DATA TingData;
			TingData.byOutCount = vctTingOut.size();
			memcpy(byData + nLen, &TingData, sizeof(TingData));
			nLen += sizeof(TingData);
			if (TingData.byOutCount > 0)
			{
				memcpy(byData + nLen, &vctTingOut[0], sizeof(stTingData)*TingData.byOutCount);
				nLen += sizeof(stTingData)*TingData.byOutCount;
			}
			m_pITableFrame->SendTableData(wChairID, SUB_S_TING_DATA, byData, nLen);
		}			
	}
}

bool CTableFrameSink::OnUserFlee(WORD wChairID)
{
	if( (m_pGameServiceOption->wServerType&GAME_GENRE_PERSONAL) != 0)//房卡模式
		return true;

	if(m_pITableFrame->GetGameStatus() == GAME_SCENE_PLAY)
	{
		m_pITableFrame->KillGameTimer(IDI_OUT_CARD);
		//逃跑		
		CMD_S_GameConclude GameConclude;
		ZeroMemory(&GameConclude, sizeof(GameConclude));
		GameConclude.cbProvideCard = m_cbLastOutCard;
		GameConclude.cbMagicCard = getValByIndex(m_cbIndexMShow);
		GameConclude.dwPlayMode = m_nPlayMode;
		GameConclude.wHuUser = INVALID_CHAIR;
		GameConclude.wPaoUser = INVALID_CHAIR;
		memcpy(GameConclude.byBuyZi, m_byBuyZi, sizeof(GameConclude.byBuyZi));
		memcpy(GameConclude.byXiaZi, m_byXiaZi, sizeof(GameConclude.byXiaZi));

		for(WORD i=0;i<m_cbPlayerCount;i++)
		{
			if(!m_bPlayStatus[i])
				continue;

			GameConclude.cbCardCount[i] = CHuPaiMJ::SwitchToCardData(m_cbCardIndex[i], GameConclude.cbHandCardData[i]);
			GameConclude.cbWeaveNum[i]=m_cbWeaveCount[i];
			memcpy(GameConclude.WeaveItemArray[i], m_WeaveItemArray[i], sizeof(tagWeaveItem)*MAX_WEAVE);
		}

		//积分变量
		tagScoreInfo ScoreInfoArray[MAX_USER_NUM];
		ZeroMemory(&ScoreInfoArray, sizeof(ScoreInfoArray));

		//统计积分
		for (WORD i = 0; i < m_cbPlayerCount; i++)
		{
			if(!m_bPlayStatus[i])
				continue;

			//收税
			if (GameConclude.lGameScore[i]>0 && (m_pGameServiceOption->wServerType&GAME_GENRE_GOLD)!=0)
			{
				GameConclude.lRevenue[i] = m_pITableFrame->CalculateRevenue(i,GameConclude.lGameScore[i]);
				GameConclude.lGameScore[i] -= GameConclude.lRevenue[i];
			}
			ScoreInfoArray[i].lScore = GameConclude.lGameScore[i];
			ScoreInfoArray[i].lRevenue = GameConclude.lRevenue[i];
			ScoreInfoArray[i].cbType = ScoreInfoArray[i].lScore > 0 ? SCORE_TYPE_WIN : SCORE_TYPE_FLEE;
			//历史积分
			m_llScore[wChairID] = GameConclude.lGameScore[i];
			m_llScoreAll[wChairID] += m_llScore[wChairID];
		}
		//发送数据
		m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));
		m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameConclude, sizeof(GameConclude));		
		//写入积分
		m_pITableFrame->WriteTableScore(ScoreInfoArray, m_cbPlayerCount);
		//结束游戏
		m_pITableFrame->ConcludeGame(GAME_SCENE_FREE);
	}
	return true;
}

//获取场景基础信息
void CTableFrameSink::GetGameBaseInfo(stStatusBase &baseInfo)
{
	//设置变量
	baseInfo.byPlayNum = m_cbPlayerCount;
	baseInfo.lCellScore = m_pITableFrame->GetCellScore();
	//自定规则
	baseInfo.cbTimeOutCard = m_pGameCustomRule->cbTimeOutCard;
	baseInfo.cbTimeOperateCard = m_pGameCustomRule->cbTimeOperateCard;
	baseInfo.cbTimeStartGame = m_pGameCustomRule->cbTimeStartGame;
	baseInfo.cbTimeWaitEnd = m_pGameCustomRule->cbTimeWaitEnd;
	memcpy(baseInfo.lCurScore, m_llScore, MAX_USER_NUM*sizeof(LONGLONG));
	memcpy(baseInfo.lAllScore, m_llScoreAll, MAX_USER_NUM*sizeof(LONGLONG));
	memcpy(baseInfo.lAllScoreSpe, m_llScoreAllSpe, MAX_USER_NUM*sizeof(LONGLONG));
	baseInfo.dwPlayMode = m_nPlayMode;
	//买子下子状态
	baseInfo.byXiaZiVal = CHuPaiMJ::GetHuRate(m_dwCurHuFlag);
	memcpy(baseInfo.byBuyZi, m_byBuyZi, sizeof(baseInfo.byBuyZi));
	memcpy(baseInfo.byXiaZi, m_byXiaZi, sizeof(baseInfo.byXiaZi));
}

//获取其玩家可看到的所有牌统计信息
void CTableFrameSink::GetShowCardsInfo(WORD wChairID, BYTE byShowIndex[MAX_INDEX])
{
	memset(byShowIndex, 0, MAX_INDEX);
	if (wChairID < m_cbPlayerCount)
	{
		//弃牌记录
		for (int i = 0; i < min(MAX_USER_NUM, m_cbPlayerCount); ++i)
		{
			for (int n = 0; n < min(60, m_cbOutNum[i]); ++n)
			{
				BYTE byIndex = getIndexByVal(m_cbOutCards[i][n]);
				if (byIndex != INVALID_VAL)
					++byShowIndex[byIndex];
			}
		}
		//桌面牌组
		for (int i = 0; i < min(MAX_USER_NUM, m_cbPlayerCount); ++i)
		{
			for (int n = 0; n < min(MAX_WEAVE, m_cbWeaveCount[i]); ++n)
			{
				//if (m_WeaveItemArray[i][n].dwWeaveKind != WIK_GANG_AN)	//暗杠是否算可见
				{
					for (int k = 0; k < 4; ++k)
					{
						BYTE byIndex = getIndexByVal(m_WeaveItemArray[i][n].cbCardData[k]);
						if (byIndex != INVALID_VAL)
							++byShowIndex[byIndex];
					}
				}
			}
		}
		//自己手牌
		for (int i = 0; i < MAX_INDEX; ++i)
		{
			byShowIndex[i] += m_cbCardIndex[wChairID][i];
		}
		//赖子牌
		if (m_nPlayMode & PlayMode_Magic_3 && m_cbMagicIndex < MAX_INDEX)
			++byShowIndex[m_cbMagicIndex];
	}
}

//从牌库中删牌（发牌）
int	CTableFrameSink::DeleteCardMJ(BYTE byNum, BYTE byIndexOut[])
{	
	if (byNum > 0 && byNum <= m_cbLeftCardCount )
	{
		m_cbLeftCardCount -= byNum;
		memcpy(byIndexOut, m_cbRepertoryCard + m_cbLeftCardCount, byNum);

		//杠后补牌不失败 理论上从末尾摸牌
		if (m_wGangChairID != INVALID_CHAIR)
			return byNum;

		if (m_cbLeftShowCount >= byNum)
		{
			m_cbLeftShowCount -= byNum;
			return byNum;
		}			
	}
	m_cbLeftShowCount = 0;
	return 0;
}

//得到玩家手牌牌面值
int CTableFrameSink::GetHandleCards(WORD wChairID, BYTE byValOut[MAX_COUNT])
{
	if (wChairID >= m_cbPlayerCount)
		return 0;

	int nNum = 0;
	for (int i = 0; i < MAX_INDEX; ++i)
	{
		if (nNum + m_cbCardIndex[wChairID][i] > MAX_COUNT)
			return 0;

		memset(byValOut + nNum, getValByIndex(i), m_cbCardIndex[wChairID][i]);
		nNum += m_cbCardIndex[wChairID][i];
	}
	return nNum;
}

int CTableFrameSink::getTestCards(string strFile, BYTE byCardsOut[], BYTE byMaxNum)
{
	int index = 0;
	char szPath[MAX_PATH] = { 0 };
	GetCurrentDirectoryA(MAX_PATH, szPath);
	char szFileName[MAX_PATH] = { 0 };
	sprintf(szFileName, "%s/%s", szPath, strFile.c_str());

	FILE *fp;
	fp = fopen(szFileName, "r");
	if (fp != NULL)
	{
		char szTemp[1024] = { 0 };
		char szData[1024] = { 0 };
		// 设置文件起始位置
		fseek(fp, 0, SEEK_SET);
		while (!feof(fp))
		{
			int nSize = 0;
			memset(szTemp, 0, sizeof(szTemp));
			memset(szData, 0, sizeof(szData));
			vector<string> vct;
			// 读取一行
			fgets(szTemp, sizeof(szTemp), fp);
			for (int i = 0; i < 1024; ++i)
			{
				if (szTemp[i] == 0)
					break;
				if (szTemp[i] != 32 && szTemp[i] != 10)
					szData[nSize++] = szTemp[i];
			}
			string strData = szData;
			SplitString(strData, vct, "0x");
			for (size_t i = 0; i < vct.size(); ++i)
			{
				if (vct[i].size() > 0 && index < byMaxNum)
				{
					int byTemp = 0;
					sscanf(vct[i].c_str(), "%x", &byTemp);
					byCardsOut[index++] = byTemp;
				}
			}
		}
		fclose(fp);
	}
	return index;
}

void CTableFrameSink::SplitString(const string &s, vector<string> &v, string c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

void CTableFrameSink::PrintLog(enTraceLevel lev, const char* format, ...)
{		
	std::string var_str;
	va_list ap;
	va_start(ap, format);
	int len = _vscprintf(format, ap);
	if (len > 0)
	{
		std::vector<char> buf(len + 1);
		vsprintf(&buf.front(), format, ap);
		var_str.assign(buf.begin(), buf.end() - 1);
	}
	va_end(ap);
	CString	strNoteMsg(var_str.c_str());
	CTraceService::TraceString(strNoteMsg.GetBuffer(), lev);
}
//////////////////////////////////////////////////////////////////////////////////
