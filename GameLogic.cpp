#pragma   warning(disable: 4996)

#include "GameLogic.h"

set<int>							g_setSingle;		//单个顺子+刻子		50个
set<int>							g_setSingleFZ;		//单个顺子+刻子		22个
set<int>							g_setSingleJiang;	//单个将			19个
set<int>							g_setSingleJiangFZ;	//单个将			15个

hash_map<int, BYTE>			g_mapHuAll[15];
hash_map<int, BYTE>			g_mapHuAllFZ[15];

void CHuPaiMJ::TrainSingle()
{
	BYTE byTemp[MAX_KEY_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 3 };
	g_setSingle.insert(getKeyByIndex(byTemp));
	g_setSingleFZ.insert(getKeyByIndex(byTemp));
	// 1.1 刻子
	for (int i = 0; i < MAX_VAL_NUM; ++i)
	{
		memset(byTemp, 0, MAX_KEY_NUM);
		for (int n = 0; n < 3; ++n)
		{
			byTemp[i] = 3 - n;	byTemp[9] = n;
			g_setSingle.insert(getKeyByIndex(byTemp));
			if (i < 7)	//风、字牌最多7张
				g_setSingleFZ.insert(getKeyByIndex(byTemp));
		}
	}
	// 1.2 顺子 没赖子
	for (int i = 0; i < MAX_VAL_NUM - 2; ++i)
	{
		memset(byTemp, 0, MAX_KEY_NUM);
		byTemp[i] = 1;	byTemp[i + 1] = 1;	byTemp[i + 2] = 1;
		g_setSingle.insert(getKeyByIndex(byTemp));
	}
	// 1.3 顺子 1个赖子 (2个赖子时也就是刻子)
	for (int i = 0; i < MAX_VAL_NUM - 2; ++i)
	{
		for (int n = 0; n < 3; ++n)
		{
			memset(byTemp, 0, MAX_KEY_NUM);
			byTemp[i] = 1;	byTemp[i + 1] = 1;	byTemp[i + 2] = 1;
			byTemp[i + n] = 0;	byTemp[9] = 1;
			g_setSingle.insert(getKeyByIndex(byTemp));
		}
	}
	// 2.1 将牌
	memset(byTemp, 0, MAX_KEY_NUM);
	byTemp[9] = 2;
	g_setSingleJiang.insert(getKeyByIndex(byTemp));
	g_setSingleJiangFZ.insert(getKeyByIndex(byTemp));
	for (int i = 0; i < MAX_VAL_NUM; ++i)
	{
		memset(byTemp, 0, MAX_KEY_NUM);
		for (int n = 0; n < 2; ++n)
		{
			byTemp[i] = 2 - n;	byTemp[9] = n;
			g_setSingleJiang.insert(getKeyByIndex(byTemp));
			if (i < 7)	//风、字牌最多7张
				g_setSingleJiangFZ.insert(getKeyByIndex(byTemp));
		}
	}
};

void CHuPaiMJ::TrainAllComb(set<int> &setSingle, hash_map<int, BYTE> mapOut[])
{
	int nAll = 0;
	int nSingle[100] = {};
	set<int>::iterator iter = setSingle.begin();
	for (; iter != setSingle.end(); ++iter)
		nSingle[nAll++] = *iter;

	for (int i1 = 0; i1 < nAll; ++i1)
	{
		addMap(mapOut, nSingle[i1]);
		for (int i2 = i1; i2 < nAll; ++i2)
		{
			int nTemp = nSingle[i1] + nSingle[i2];
			if (!isValidKey(nTemp))	continue;
			addMap(mapOut, nTemp);
			for (int i3 = i2; i3 < nAll; ++i3)
			{
				int nTemp = nSingle[i1] + nSingle[i2] + nSingle[i3];
				if (!isValidKey(nTemp))	continue;
				addMap(mapOut, nTemp);
				for (int i4 = i3; i4 < nAll; ++i4)
				{
					int nTemp = nSingle[i1] + nSingle[i2] + nSingle[i3] + nSingle[i4];
					if (!isValidKey(nTemp))	continue;
					addMap(mapOut, nTemp);
				}
			}
		}
	}
}

void CHuPaiMJ::TrainAllComb_Jiang(set<int> &setSingle, hash_map<int, BYTE> mapOut[])
{
	int nAll = 0;
	int nSingle[100] = {};

	set<int>::iterator iter = setSingle.begin();
	for (; iter != setSingle.end(); ++iter)
		nSingle[nAll++] = *iter;

	hash_map<int, BYTE> mapTemp[15];
	for (int j = 0; j < 15; ++j)
		mapTemp[j] = mapOut[j];

	for (int i = 0; i < nAll; ++i)
	{
		for (int j = 0; j < 15; ++j)
		{
			addMap(mapOut, nSingle[i]);
			hash_map<int, BYTE>::iterator iter_u = mapTemp[j].begin();
			for (; iter_u != mapTemp[j].end(); ++iter_u)
			{
				int nTemp = nSingle[i] + iter_u->first + (int(iter_u->second & BIT_VAL_FLAG) << 27);
				if (isValidKey(nTemp))
					addMap(mapOut, nTemp);
			}
		}
	}
}

bool CHuPaiMJ::CheckCanHuSingle(BYTE byType, BYTE byCards[], BYTE &byNaiNum, BYTE byNaiMax)
{
	bool bSuc = false;
	int	 nVal = getKeyByIndex(byCards, MAX_VAL_NUM);
	if (byType == enCor_FenZi)
		nVal &= 0x1FFFFF;

	BYTE nNum = getNumByKey(nVal, MAX_VAL_NUM);
	if (nNum >= 15) return false;
	hash_map<int, BYTE> &mapHuAll = (byType == enCor_FenZi) ? g_mapHuAllFZ[nNum] : g_mapHuAll[nNum];
	hash_map<int, BYTE>::iterator iterFind = mapHuAll.find(nVal);
	if (iterFind != mapHuAll.end())
	{		
		if (iterFind->second <= byNaiMax)
		{
			byNaiNum = iterFind->second;
			return true;
		}			
	}
	return false;
}


void CHuPaiMJ::TrainAll()
{
	if (g_setSingle.empty())
	{
		TrainSingle();
		TrainAllComb(g_setSingle, g_mapHuAll);
		TrainAllComb(g_setSingleFZ, g_mapHuAllFZ);
		TrainAllComb_Jiang(g_setSingleJiang, g_mapHuAll);
		TrainAllComb_Jiang(g_setSingleJiangFZ, g_mapHuAllFZ);
	}
}

// 听牌
bool CHuPaiMJ::GetTingData(BYTE byIndexSrc[], BYTE byIndexShow[], tagWeaveItem stWeave[], BYTE byNaiIndex, vector<stTingData> &vctTingOut)
{
	int nAllNum = 0;
	for (int i = 0; i < MAX_INDEX; ++i)
		nAllNum += byIndexSrc[i];

	BYTE byWeaveNum = 0;
	for (int i = 0; i < MAX_WEAVE; ++i)
	{
		if (stWeave[i].dwWeaveKind == WIK_NULL)
			break;
		++byWeaveNum;
	}	

	vctTingOut.clear();
	if (nAllNum % 3 == 2)
	{
		for (int i = 0; i < MAX_INDEX; ++i)
		{
			if (byIndexSrc[i] > 0)
			{
				--byIndexSrc[i];
				stTingData stTing;
				stTing.byOutCard = getValByIndex(i);
				for (int n = 0; n < MAX_INDEX; ++n)
				{
					DWORD nHuFlag = GetHuFlag(byIndexSrc, n, byNaiIndex, false);
					if (nHuFlag > 0)
						stTing.addData(getValByIndex(n), max(0, 4 - (int)byIndexShow[n]), GetHuRate(nHuFlag));
				}
				if (stTing.byHuNum > 0)
					vctTingOut.push_back(stTing);

				++byIndexSrc[i];
			}
		}
		return vctTingOut.size() > 0;
	}
	return false;
}

// 胡
DWORD CHuPaiMJ::CheckHu(BYTE byIndexSrc[], BYTE byIndex, BYTE byNaiShowIndex, bool bZiMo/*true*/, int nHuFlag/**/)
{
	BYTE byCards[MAX_INDEX];
	memcpy(byCards, byIndexSrc, MAX_INDEX);	
	if (byIndex < MAX_INDEX && !bZiMo)
		++byCards[byIndex];
	
	DWORD dwHuFlag = 0;
	if ((nHuFlag&CHR_HU_NORMAL) && GetJiangNum(byCards, 33)==1)
		dwHuFlag |= CHR_HU_NORMAL;
	else if ((nHuFlag&CHR_HU_QI_DUI) && CheckQiDui(byCards, 33))
		dwHuFlag |= CHR_HU_QI_DUI;
	else if ((nHuFlag&CHR_HU_13_LAN) && CheckLan13(byCards, 33, byNaiShowIndex))
		dwHuFlag |= CHR_HU_13_LAN;
	
	return dwHuFlag;
}

int CHuPaiMJ::GetJiangNum(BYTE byIndexSrc[], BYTE byNaiIndex)
{
	if (g_setSingle.empty())
		TrainAll();

	BYTE byCards[MAX_INDEX];
	memcpy(byCards, byIndexSrc, MAX_INDEX);
	int nNaiZiNum = 0;
	if (byNaiIndex < MAX_INDEX)
	{
		nNaiZiNum = byCards[byNaiIndex];
		byCards[byNaiIndex] = 0;
	}

	int nJiangNum = 0;
	BYTE nNaiTry[enCor_Max] = {};
	for (int cor = 0; cor < enCor_Max; ++cor)
	{
		int nMax = (cor == enCor_FenZi) ? 7 : 9;
		int nCardAll = 0;
		for (int i = 0; i < nMax; ++i)
			nCardAll += byCards[9 * cor + i];

		if (nCardAll == 0) continue;
		if (!CheckCanHuSingle(cor, byCards + 9 * cor, nNaiTry[cor], nNaiZiNum))
			return -1;

		nNaiZiNum -= nNaiTry[cor];
		nJiangNum += ((nCardAll + nNaiTry[cor]) % 3 == 2);
		if (nJiangNum > nNaiZiNum + 1)
			return -1;
	}
	
	int nMin = min(nNaiZiNum, nJiangNum);
	nJiangNum -= nMin;
	nNaiZiNum -= nMin;
	nNaiZiNum = nNaiZiNum%3;
	if (nNaiZiNum % 2) return -1;
	return nJiangNum + nNaiZiNum/2;
}

// 七小对
bool CHuPaiMJ::CheckQiDui(BYTE byIndexSrc[], BYTE byNaiIndex, BYTE nNeedCards/*14*/)
{
	BYTE byCards[MAX_INDEX];
	memcpy(byCards, byIndexSrc, MAX_INDEX);
	int nNaiZiNum = 0;
	if (byNaiIndex < MAX_INDEX)
	{
		nNaiZiNum = byCards[byNaiIndex];
		byCards[byNaiIndex] = 0;
	}

	int nNumAll = nNaiZiNum;	//牌总数量
	int nNumDan = 0;			//单牌数量
//	int nNumGang = 0;			//四张的数量
	for (int i = 0; i < MAX_INDEX; ++i)
	{
		nNumAll += byCards[i];
		nNumDan += byCards[i] % 2;
// 		if (byCards[i] == 4)
// 			++nNumGang;
	}
	return (nNumAll == nNeedCards) && (nNaiZiNum >= nNumDan);
}

// 十三烂
bool CHuPaiMJ::CheckLan13(BYTE byIndexSrc[], BYTE byNaiIndex, BYTE byNaiShowIndex)
{
	if (byNaiIndex == INVALID_VAL || byNaiShowIndex == INVALID_VAL)
		return false;
	
	BYTE byCards[MAX_INDEX];
	memcpy(byCards, byIndexSrc, MAX_INDEX);
	int nNaiZiNum = 0;
	if (byNaiIndex < MAX_INDEX)
	{
		nNaiZiNum = byCards[byNaiIndex];
		byCards[byNaiIndex] = 0;
	}
	// 白板（33）本来变为赖子牌原始牌，但在13烂版型时变回本来牌值
	if (byNaiShowIndex != byNaiIndex)
	{
		byCards[byNaiIndex] = byCards[byNaiShowIndex];
		byCards[byNaiShowIndex] = 0;
	}		

	int nNumAll = nNaiZiNum;	//牌总数量
	for (int cor = 0; cor <= enCor_FenZi; ++cor)
	{
		int nDis = 2;
		int nMaxVal = (cor == enCor_FenZi) ? 7 : 9;
		for (int val = 0; val < nMaxVal; ++val)
		{
			BYTE byIndex = cor * 9 + val;
			nNumAll += byCards[byIndex];
			if (byCards[byIndex] > 1)
				return false;
			else if (byCards[byIndex] == 0)
				++nDis;
			else if (byCards[byIndex] == 1)
			{
				if (nDis < 2 && cor != enCor_FenZi)
					return false;
				nDis = 0;
			}
		}
	}
	return nNumAll == 14;
}

// 吃
int CHuPaiMJ::CheckChi(BYTE byIndexSrc[], BYTE byCard, BYTE byNaiIndex, vector<stOptData> &vctOut)
{
	int nChiFlag = WIK_NULL;
	BYTE byIndex = getIndexByVal(byCard);
	if (byIndex == INVALID_VAL) return WIK_NULL;

	BYTE byCor = byIndex / 9;
	BYTE byVal = byIndex % 9;

	// 风字牌无吃
	if (byCor >= enCor_FenZi) return WIK_NULL;
		
	//右吃 WIK_CHI_R
	if (byVal >= 2 && byVal <= 8 && byIndex - 1 != byNaiIndex && byIndex - 2 != byNaiIndex)
	{
		if (byIndexSrc[byIndex - 1] > 0 && byIndexSrc[byIndex - 2] > 0)
		{
			nChiFlag |= WIK_CHI;
			vctOut.push_back(stOptData(WIK_CHI, byCard, byCard - 1, byCard - 2));
		}
	}
	//中吃 WIK_CHI_M
	if (byVal >= 1 && byVal <= 7 && byIndex - 1 != byNaiIndex && byIndex + 1 != byNaiIndex)
	{
		if (byIndexSrc[byIndex - 1] > 0 && byIndexSrc[byIndex + 1] > 0)
		{
			nChiFlag |= WIK_CHI;
			vctOut.push_back(stOptData(WIK_CHI, byCard, byCard - 1, byCard + 1));
		}			
	}	
	//左吃 WIK_CHI_L
	if (byVal >= 0 && byVal <= 6 && byIndex + 1 != byNaiIndex && byIndex + 2 != byNaiIndex)
	{
		if (byIndexSrc[byIndex + 1] > 0 && byIndexSrc[byIndex + 2] > 0)
		{
			nChiFlag |= WIK_CHI;
			vctOut.push_back(stOptData(WIK_CHI, byCard, byCard + 1, byCard + 2));
		}
	}
	return nChiFlag;
}
// 碰
int CHuPaiMJ::CheckPeng(BYTE byIndexSrc[], BYTE byCard, BYTE byNaiIndex)
{
	BYTE byIndex = getIndexByVal(byCard);
	if (byIndex == INVALID_VAL || byIndex == byNaiIndex) 
		return false;
	return byIndexSrc[byIndex] >= 2 ? WIK_PENG : WIK_NULL;
}

// 明杠（点杠）
int CHuPaiMJ::CheckGangDian(BYTE byIndexSrc[], BYTE byCard, BYTE byNaiIndex)
{
	BYTE byIndex = getIndexByVal(byCard);
	if (byIndex == INVALID_VAL || byIndex == byNaiIndex)
		return false;
	return byIndexSrc[byIndex] >= 3 ? WIK_GANG_MING : WIK_NULL;
}

// 杠
DWORD CHuPaiMJ::CheckGangAn(BYTE byIndexSrc[], BYTE byNaiIndex, vector<stOptData> &vctOut)
{
	DWORD dwFlag = 0;
	for (int i = 0; i < MAX_INDEX; ++i)
	{
		if (byIndexSrc[i] >= 4 && byNaiIndex != i)
		{
			dwFlag |= WIK_GANG_AN;
			vctOut.push_back(stOptData(WIK_GANG_AN, getValByIndex(i)));
		}
	}
	return dwFlag;
}
// 
BYTE CHuPaiMJ::SwitchToCardData(BYTE byIndexSrc[MAX_INDEX], BYTE byCardOut[MAX_COUNT])
{
	BYTE byAll = 0;
	for (int i = 0; i < MAX_INDEX; ++i)
	{
		BYTE num = byIndexSrc[i];
		if (num + byAll > MAX_COUNT)
			return false;

		memset(byCardOut + byAll, getValByIndex(i), num);
		byAll += num;
	}
	return byAll;
}

// 由当前牌数据得到胡牌标志
DWORD CHuPaiMJ::GetHuFlag(BYTE byIndexSrc[], BYTE byIndex, BYTE byNaiIndex, bool bZiMo/*true*/, int nHuFlag)
{
	DWORD dwHuFlag = CheckHu(byIndexSrc, byIndex, byNaiIndex, bZiMo, nHuFlag);
	if (dwHuFlag == 0)
		return 0;
	if (byIndex >= MAX_INDEX || (bZiMo && byIndexSrc[byIndex] <= 0))
		return 0;
	
	// 财神头（赖子做将、只能自摸）
	if (bZiMo)
	{	
		BYTE byTemp[MAX_INDEX] = {};
		memcpy(byTemp, byIndexSrc, MAX_INDEX);
		--byTemp[byIndex];
		if (isJiangNaiZi2(byTemp, 33))
			dwHuFlag |= CHR_NAIZI_JIANG;
	}		
	return dwHuFlag;
}

// 由胡牌标志得到胡牌倍率
int CHuPaiMJ::GetHuRate(DWORD dwHuFlag)
{
	if (dwHuFlag == 0)
		return 0;
	
	if (dwHuFlag & CHR_PIAO_NAI2)
		return 16;
	if (dwHuFlag & CHR_PIAO_NAI1)
		return 8;
	if (dwHuFlag & CHR_GANG_KAIHUA)
		return 4;
	if (dwHuFlag & CHR_NAIZI_JIANG)
		return 4;
	if (dwHuFlag & CHR_GANG_QIANG_HU)
		return 2;
	if (dwHuFlag & CHR_HU_ZIMO)
		return 2;

	return 1;
}

// 是否将牌单听(王吊)
bool  CHuPaiMJ::isJiangNaiZi(BYTE byIndexSrc[], BYTE byNaiIndex)
{	
	if (byNaiIndex >= MAX_INDEX) return false;
	BYTE byIndex[MAX_INDEX] = {};
	memcpy(byIndex, byIndexSrc, MAX_INDEX);
	if (byNaiIndex >= MAX_INDEX || byIndex[byNaiIndex] <= 0)
		return false;

	--byIndex[byNaiIndex];
	for (int i = 0; i < MAX_INDEX; ++i)
	{
		if (i == byNaiIndex || byIndex[i] <= 0)
			continue;

		--byIndex[i];
		if (GetJiangNum(byIndex, byNaiIndex) == 0)	return true;
		++byIndex[i];
	}
	return false;
}
// 是否将牌单听(王吊)
bool  CHuPaiMJ::isJiangNaiZi2(BYTE byIndexSrc[], BYTE byNaiIndex)
{
	if (byNaiIndex >= MAX_INDEX) return false;
	BYTE byIndex[MAX_INDEX] = {};
	memcpy(byIndex, byIndexSrc, MAX_INDEX);
	if (byNaiIndex >= MAX_INDEX || byIndex[byNaiIndex] <= 0)
		return false;

	--byIndex[byNaiIndex];
	bool bSuc = GetJiangNum(byIndex, byNaiIndex) == 0;
	// 七小对财神头判断
	if (bSuc == false)
		bSuc = CheckQiDui(byIndex, byNaiIndex, 12);
	return bSuc;
}
