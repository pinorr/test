#include "GameLogicPino.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
//自定义排序函数  
bool cmp(BYTE a, BYTE b) {
	return a > b;			//从大到小排序，从小到大排序为a<b  
}

//扑克数据
const BYTE	g_cbCardVal[MAX_CARD_NUM] =
{
	0x08, 0x18, 0x28, 0x38, 0x09, 0x19, 0x29, 0x39,
	0x0A, 0x1A, 0x2A, 0x3A, 0x0B, 0x1B, 0x2B, 0x3B,
	0x0C, 0x1C, 0x2C, 0x3C, 0x0D, 0x1D, 0x2D, 0x3D,
	0x01, 0x11, 0x21, 0x31, 
};

//////////////////////////////////////////////////////////////////////////

//构造函数
CGameLogicPino::CGameLogicPino()
{
	initData();
}

//析构函数
CGameLogicPino::~CGameLogicPino()
{
}

void CGameLogicPino::initData()
{	
	for (int i=0; i<MAX_CARD_NUM; ++i)
		m_mapIndexByVal[g_cbCardVal[i]] = i+1;

#ifndef USE_MAP
	return;
#endif
	BYTE byCards[5] = {};
	for (int i1=0; i1<MAX_CARD_NUM; ++i1)
	{
		byCards[0] = g_cbCardVal[i1];
		for (int i2 = i1 + 1; i2 < MAX_CARD_NUM; ++i2)
		{
			byCards[1] = g_cbCardVal[i2];
			getLogicVal2(byCards, 2);
			for (int i3 = i2 + 1; i3 < MAX_CARD_NUM; ++i3)
			{
				byCards[2] = g_cbCardVal[i3];
				getLogicVal2(byCards, 3);
				for (int i4 = i3 + 1; i4 < MAX_CARD_NUM; ++i4)
				{
					byCards[3] = g_cbCardVal[i4];
					getLogicVal2(byCards, 4);
					for (int i5 = i4 + 1; i5 < MAX_CARD_NUM; ++i5)
					{
						byCards[4] = g_cbCardVal[i5];
						getLogicVal2(byCards, 5);
					}
				}
			}
		}
	}
}

//得到牌组key
DWORD CGameLogicPino::getKeyByCards(const BYTE cbData[], BYTE num)
{
	DWORD dwKey = 0;
	if (m_mapIndexByVal.empty())
		initData();

	if (num == 0 || num > 5)
		return 0;

	BYTE cbIndex[5] = {};
	for (int i = 0; i < num; ++i)
		cbIndex[i] = m_mapIndexByVal[cbData[i]];

	sort(cbIndex, cbIndex + 5, cmp);

	for (int i = 0; i < num; ++i)
		dwKey = (dwKey << 5) + (cbIndex[i] & 31);

	return dwKey;
}

//由key得到牌组
void CGameLogicPino::getCardsByKey(DWORD dwKey, BYTE cbData[5])
{
	memset(cbData, 0, 5);
	int n = 0;
	while (dwKey != 0)
	{
		BYTE index = (dwKey & 31) - 1;
		if (index == 0xFF)
		{
			dwKey = dwKey >> 5;
			continue;
		}			
		else if ( index >= MAX_CARD_NUM)
		{
			memset(cbData, 0, 5);
			return;
		}
		cbData[n++] = g_cbCardVal[index];
		dwKey = dwKey >> 5;
	}	
}

//获取牌组权值
DWORD CGameLogicPino::getLogicVal(const BYTE cbData[], BYTE num)
{
#ifdef USE_MAP
	static DWORD s_dwFlag4[7] = { 585, 1170, 1755, 2340, 2925, 3510, 4095 };
	static DWORD s_dwFlag3[7] = { 73, 146, 219, 292, 365, 483, 511 };
	static DWORD s_dwFlag2[7] = { 9, 18, 27, 36, 45, 54, 63 };

	bool bSameCor = false;		// 同花
	BYTE nSameVal4 = 0xFF;
	BYTE byCountCor[4] = {};
	BYTE byCountVal[7] = {};
	BYTE byCountMaxCor[7] = {};
	for (int i = 0; i < num; ++i)
	{
		BYTE index = m_mapIndexByVal[cbData[i]] - 1;
		if (index >= MAX_CARD_NUM)
			return 0;

		BYTE byCor = index % 4;
		BYTE byVal = index / 4;
		if (++byCountCor[byCor] >= 5)
			bSameCor = true;

		++byCountVal[byVal];
		if (byCountVal[byVal] == 4)
			nSameVal4 = byVal;
		if (byCountMaxCor[byVal] < byCor)
			byCountMaxCor[byVal] = byCor;
	}
	if (nSameVal4 != 0xFF)
	{
		DWORD dwValFlag = s_dwFlag4[nSameVal4] << 3;
		return ((DWORD)enType_same4 << 17) + (dwValFlag << 2) + 3;
	}
	else
	{
		DWORD dwKey = 0;
		for (int i = 0; i < 7; ++i)
			dwKey += byCountVal[i] << (2 * i);

		stData &stTemp = m_KeyData[dwKey];
		DWORD dwAddType = 0;
		if (bSameCor)
		{
			DWORD dwType = stTemp.dwVal >> 17;
			if (dwType == enType_null)
				dwAddType = 655360;
			else if (dwType == enType_12345)
				dwAddType = 524288;
		}
		return stTemp.dwVal + byCountMaxCor[stTemp.byMaxCor] + dwAddType;
	}
#else
	return getLogicVal2(cbData, num);
#endif		
	
}
DWORD CGameLogicPino::getLogicVal2(const BYTE cbData[], BYTE num)
{
	static DWORD s_dwFlag4[7] = { 585, 1170, 1755, 2340, 2925, 3510, 4095 };
	static DWORD s_dwFlag3[7] = { 73, 146, 219, 292, 365, 483, 511 };
	static DWORD s_dwFlag2[7] = { 9, 18, 27, 36, 45, 54, 63 };
	
	bool bSameCor = false;		// 同花
	BYTE byCountCor[4] = {};
	BYTE byCountVal[7] = {};
	BYTE byCountMaxCor[7] = {};
	BYTE byMaxVal = 0;
	for (int i=0; i<num; ++i)
	{
		BYTE index = m_mapIndexByVal[cbData[i]] - 1;
		if (index >= MAX_CARD_NUM)
			return 0;
			
		BYTE byCor = index % 4;
		BYTE byVal = index / 4;
		if (++byCountCor[byCor] >= 5)
			bSameCor = true;
				
		++byCountVal[byVal];
		byCountMaxCor[byVal] = max(byCountMaxCor[byVal], byCor);
		byMaxVal = max(byMaxVal, byVal);
	}	

	// 得到 enType、maxCor		
	BYTE nLineValNum = 0;
	BYTE nSame4Val = 0xFF;
	BYTE nSame3Val = 0xFF;
	BYTE nSame2Val[2] = { 0xFF, 0xFF };
	BYTE nSame2Num = 0;
	for (int i = 0; i < 7; ++i)
	{
		switch (byCountVal[i])
		{
		case 0:
			if (nLineValNum == 4) {
				if (byCountVal[0] != 1 || byCountVal[6] != 1)
					nLineValNum = 0;
			}
			else if (nLineValNum < 4)
				nLineValNum = 0;
			break;
		case 1:
			nLineValNum++;
			break;
		case 2:
			nSame2Val[nSame2Num++] = i;
			nLineValNum = 0;
			break;
		case 3:
			nSame3Val = i;
			nLineValNum = 0;
			break;
		case 4:
			nSame4Val = i;
			nLineValNum = 0;
			break;
		default:
			break;
		}			
	}		

#ifdef USE_MAP
	DWORD dwKey = 0;
	for (int i = 0; i < 7; ++i)
		dwKey += byCountVal[i] << (2 * i);
#endif

	BYTE byMaxCor = byMaxVal;
	BYTE byType = enType_null;
	if (nLineValNum == 5 && bSameCor) {
		byType = enType_flush;
		if (byCountVal[6] == 1 && byCountVal[0] == 1)
			byCountVal[6] = 0;
	}
	else if (nSame4Val != 0xFF) {
		byType = enType_same4;
		byMaxCor = nSame4Val;
	}
	else if (nSame3Val != 0xFF && nSame2Num == 1) {
		byType = enType_same3_2;
		byMaxCor = nSame3Val;
	}
	else if (bSameCor) {
		byType = enType_sameCor;
	}
	else if (nLineValNum == 5) {
		byType = enType_12345;
		if (byCountVal[6] == 1 && byCountVal[0] == 1) {
			byMaxCor = 3;
			byCountVal[6] = 0;
		}				
	}
	else if (nSame3Val != 0xFF) {
		byType = enType_same3;
		byMaxCor = nSame3Val;
	}
	else if (nSame2Num == 2) {
		byType = enType_pair2;
		byMaxCor = nSame2Val[1];
	}
	else if (nSame2Num == 1) {
		byType = enType_pair1;
		byMaxCor = nSame2Val[0];
	}
	BYTE  byFlagNum = 0;
	DWORD dwValFlag = 0;
	if (nSame4Val != 0xFF) {
		dwValFlag += s_dwFlag4[nSame4Val];
		byFlagNum += 4;
	}
	if (nSame3Val != 0xFF) {
		dwValFlag += s_dwFlag3[nSame3Val];
		byFlagNum += 3;
	}
	for (int i = 1; i >= 0; --i) {
		if (nSame2Val[i] < 7) {
			dwValFlag = dwValFlag << 6;
			dwValFlag += s_dwFlag2[nSame2Val[i]];
			byFlagNum += 2;
		}
	}		
	for (int i = 6; i >= 0; --i) {
		if (byCountVal[i] == 1)	{
			dwValFlag = dwValFlag << 3;
			dwValFlag += i+1;
			++byFlagNum;
		}
	}

	if (byFlagNum < 5)
		dwValFlag = dwValFlag << (3 * (5 - byFlagNum));

	DWORD dwVal = ((DWORD)byType << 17) + (dwValFlag << 2);
#ifdef USE_MAP
	if (nSame4Val == 0xFF)
	{
		if (m_KeyData[dwKey].dwVal == 0)
		{
			m_KeyData[dwKey].dwVal = dwVal;
			m_KeyData[dwKey].byMaxCor = byMaxCor;
		}
		else if (m_KeyData[dwKey].dwVal > dwVal)
		{
			m_KeyData[dwKey].dwVal = dwVal;
		}
	}	
#endif
	return dwVal + (byCountMaxCor[byMaxCor] & 3);
}

//获取牌组类型
BYTE CGameLogicPino::getCardsType(const BYTE cbData[], BYTE num)
{
	DWORD dwVal = getLogicVal(cbData, num);
	return BYTE((dwVal & 1966080)>>17);
}


DWORD CGameLogicPino::getMapInfo(unordered_map<DWORD, DWORD> &mapUnOrder, BYTE byTemp[5], BYTE num, const bool bFlag[MAX_CARD_NUM])
{
	DWORD dwNum = 0;
	if (num == 1)
	{
		for (int i = 0; i < MAX_CARD_NUM; ++i)
		{
			if (!bFlag[i]) continue;
			byTemp[1] = g_cbCardVal[i];
			for (int j = i + 1; j < MAX_CARD_NUM; ++j)
			{
				if (!bFlag[j]) continue;
				byTemp[2] = g_cbCardVal[j];
				for (int k = j + 1; k < MAX_CARD_NUM; ++k)
				{
					if (!bFlag[k]) continue;
					byTemp[3] = g_cbCardVal[k];
					for (int l = k + 1; l < MAX_CARD_NUM; ++l)
					{
						if (!bFlag[l]) continue;
						byTemp[4] = g_cbCardVal[l];
						++mapUnOrder[getLogicVal(byTemp, 5)];						
						++dwNum;
					}
				}
			}
		}
	}
	if (num == 2)
	{
		for (int i = 0; i < MAX_CARD_NUM; ++i)
		{
			if (!bFlag[i]) continue;
			byTemp[2] = g_cbCardVal[i];
			for (int j = i + 1; j < MAX_CARD_NUM; ++j)
			{
				if (!bFlag[j]) continue;
				byTemp[3] = g_cbCardVal[j];
				for (int k = j + 1; k < MAX_CARD_NUM; ++k)
				{
					if (!bFlag[k]) continue;
					byTemp[4] = g_cbCardVal[k];
					++mapUnOrder[getLogicVal(byTemp, 5)];
					++dwNum;
				}
			}
		}
	}
	else if (num == 3)
	{
		for (int i = 0; i < MAX_CARD_NUM; ++i)
		{
			if (!bFlag[i]) continue;
			byTemp[3] = g_cbCardVal[i];
			for (int j = i + 1; j < MAX_CARD_NUM; ++j)
			{
				if (!bFlag[j]) continue;
				byTemp[4] = g_cbCardVal[j];
				++mapUnOrder[getLogicVal(byTemp, 5)];
				++dwNum;
			}
		}
	}
	else if (num == 4)
	{
		for (int i = 0; i < MAX_CARD_NUM; ++i)
		{
			if (!bFlag[i]) continue;
			byTemp[4] = g_cbCardVal[i];
			++mapUnOrder[getLogicVal(byTemp, 5)];
			++dwNum;
		}
	}
	else if (num == 5)
	{
		++mapUnOrder[getLogicVal(byTemp, 5)];
		++dwNum;
	}
	return dwNum;
}

//获取获胜机率
float CGameLogicPino::get1WinPer(const BYTE cbData1[], const BYTE cbData2[], BYTE num1, BYTE num2)
{
	if (num1 < 1 || num2 < 1)
		return 0;

	BYTE byTemp1[5] = {};
	BYTE byTemp2[5] = {};
	memcpy(byTemp1, cbData1, num1);
	memcpy(byTemp2, cbData2, num2);

	bool bFlag[MAX_CARD_NUM] = {};
	memset(bFlag, true, MAX_CARD_NUM);
	for (int i = 0; i < num1; ++i)
	{
		BYTE index = m_mapIndexByVal[cbData1[i]] - 1;
		if (index < MAX_CARD_NUM)
			bFlag[index] = false;
	}
	for (int i = 0; i < num2; ++i)
	{
		BYTE index = m_mapIndexByVal[cbData2[i]] - 1;
		if (index < MAX_CARD_NUM)
			bFlag[index] = false;
	}
	unordered_map<DWORD, DWORD> mapUnOrder1, mapUnOrder2;
	DWORD dwNum1 = getMapInfo(mapUnOrder1, byTemp1, num1, bFlag);
	DWORD dwNum2 = getMapInfo(mapUnOrder2, byTemp2, num2, bFlag);	
	
	map<DWORD, DWORD> mapOrder1, mapOrder2;
	auto iter = mapUnOrder2.begin();
	for (; iter != mapUnOrder2.end(); ++iter)
		mapOrder2[iter->first] = iter->second;

	DWORD dwAll = 0;
	auto iter2 = mapOrder2.begin();
	for (; iter2 != mapOrder2.end(); ++iter2)
	{
		DWORD dwTemp = iter2->second;
		iter2->second = dwAll;
		dwAll += dwTemp;
	}
	
 	float fWinCount = 0;
	DWORD dwNumAll = dwNum1 * dwNum2;
	auto iter1 = mapUnOrder1.begin();
	for (; iter1 != mapUnOrder1.end(); ++iter1)
	{
		iter2 = mapOrder2.upper_bound(iter1->first);
		if (iter2 != mapOrder2.end())
			fWinCount += iter1->second * iter2->second;
		else
			fWinCount += iter1->second * dwNum2;

		dwNumAll -= iter1->second * mapUnOrder2[iter1->first];
		fWinCount -= iter1->second * mapUnOrder2[iter1->first];
	}	

	return fWinCount / dwNumAll;
}
//////////////////////////////////////////////////////////////////////////


