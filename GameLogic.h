/********************************************************************
created:	pinorr
file base:	HuPaiMJ.h
author:		pinorr github
purpose:	麻将胡牌提示算法(第3版)
*********************************************************************/
#pragma   once  
#ifndef __GAME_LOGIC_H__
#define __GAME_LOGIC_H__
#include <set>
#include <string>
#include <vector>
#include <windows.h>
#include <hash_map>

#include "..\消息定义\CMD_Game.h"

using namespace std;

inline int getKeyByIndex(BYTE byIndex[MAX_KEY_NUM], BYTE byNum = MAX_KEY_NUM)
{
	int nKey = 0;
	for (int i = 0; i < byNum; ++i)
		nKey |= (int)(byIndex[i] & BIT_VAL_FLAG) << (BIT_VAL_NUM*i);
	return nKey;
}
inline bool isValidKey(int llVal)
{
	BYTE byIndex[MAX_KEY_NUM] = {};
	for (int i = 0; i < MAX_KEY_NUM; ++i)
		byIndex[i] = (llVal >> (BIT_VAL_NUM*i))&BIT_VAL_FLAG;

	if (byIndex[9] > MAX_NAI_NUM)	return false;
	int nNum = 0;
	for (int i = 0; i < MAX_KEY_NUM; ++i)
	{
		nNum += byIndex[i];
		if (byIndex[i] > 4 || nNum > 14)	//
			return false;
	}
	return nNum > 0;
}
inline BYTE getNumByKey(int llVal, BYTE byNum = MAX_KEY_NUM)
{
	BYTE byIndex[MAX_KEY_NUM] = {};
	for (int i = 0; i < MAX_KEY_NUM; ++i)
		byIndex[i] = (llVal >> (BIT_VAL_NUM*i))&BIT_VAL_FLAG;

	BYTE nNum = 0;
	for (int i = 0; i < byNum; ++i)
		nNum += byIndex[i];
	return nNum;
}
inline void addMap(hash_map<int, BYTE> mapTemp[], int llVal)
{
	BYTE nNum = getNumByKey(llVal, MAX_VAL_NUM);
	BYTE byNum = (llVal >> (BIT_VAL_NUM * 9))&BIT_VAL_FLAG;
	int  val = (llVal & 0x7FFFFFF);
	hash_map<int, BYTE>::iterator iter = mapTemp[nNum].find(val);
	if (iter != mapTemp[nNum].end())
		iter->second = min(byNum, iter->second);
	else
		mapTemp[nNum][val] = byNum;
}
class CHuPaiMJ
{
private:
	static void TrainSingle();

	static void TrainAllComb(set<int> &setSingle, hash_map<int, BYTE> mapOut[]);

	static void TrainAllComb_Jiang(set<int> &setSingle, hash_map<int, BYTE> mapOut[]);

	static bool CheckCanHuSingle(BYTE byType, BYTE byCards[], BYTE &byNaiNum, BYTE byNaiMax);	
	// 得到将牌数(将牌数==1 -> 一般胡)
	static int  GetJiangNum(BYTE byIndexSrc[], BYTE byNaiIndex);
	// 七小对
	static bool CheckQiDui(BYTE byIndexSrc[], BYTE byNaiIndex, BYTE nNeedCards=14);
	// 十三烂
	static bool CheckLan13(BYTE byIndexSrc[], BYTE byNaiIndex, BYTE byNaiShowIndex);

public:
	static void TrainAll();	
	// 听牌
	static bool GetTingData(BYTE byIndexSrc[], BYTE byIndexShow[], tagWeaveItem stWeave[], BYTE byNaiIndex, vector<stTingData> &vctTingOut);
	// 胡
	static DWORD CheckHu(BYTE byIndexSrc[], BYTE byTouchIndex, BYTE byNaiIndex, bool bZiMo=true, int nHuFlag = CHR_HU_NORMAL | CHR_HU_QI_DUI | CHR_HU_13_LAN);
	// 吃
	static int  CheckChi(BYTE byIndexSrc[], BYTE byCard, BYTE byNaiIndex, vector<stOptData> &vctOut);
	// 碰
	static int  CheckPeng(BYTE byIndexSrc[], BYTE byCard, BYTE byNaiIndex);
	// 明杠（点杠）
	static int  CheckGangDian(BYTE byIndexSrc[], BYTE byCard, BYTE byNaiIndex);
	// 暗杠
	static DWORD CheckGangAn(BYTE byIndexSrc[], BYTE byNaiIndex, vector<stOptData> &vctOut);
	// 
	static BYTE SwitchToCardData(BYTE byIndexSrc[MAX_INDEX], BYTE byCardOut[MAX_COUNT]);

	// 不同玩法的麻将特别处理接口
public:
	// 由当前牌数据得到胡牌标志
	static DWORD GetHuFlag(BYTE byIndexSrc[], BYTE byIndex, BYTE byNaiIndex, bool bZiMo = true, int nHuFlag = CHR_HU_NORMAL | CHR_HU_QI_DUI | CHR_HU_13_LAN);
	// 由胡牌标志得到胡牌倍率
	static int  GetHuRate(DWORD dwHuFlag);

	// 是否将牌单听(王吊)
	static bool isJiangNaiZi(BYTE byIndexSrc[], BYTE byNaiIndex);
	static bool isJiangNaiZi2(BYTE byIndexSrc[], BYTE byNaiIndex);
		
};

#endif //__GAME_LOGIC_H__
