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

	for (int i1=0; i1<MAX_CARD_NUM; ++i1)
	{
		for (int i2 = i1; i2 < MAX_CARD_NUM; ++i2)
		{
			BYTE byCards2[2] = { g_cbCardVal[i2], g_cbCardVal[i1] };
			getLogicVal(byCards2, 2);
			for (int i3 = i2; i3 < MAX_CARD_NUM; ++i3)
			{
				BYTE byCards3[3] = { g_cbCardVal[i3], g_cbCardVal[i2], g_cbCardVal[i1] };
				getLogicVal(byCards3, 3);
        for (int i4 = i3; i4 < MAX_CARD_NUM; ++i4)
				{
					BYTE byCards4[4] = { g_cbCardVal[i4], g_cbCardVal[i3], g_cbCardVal[i2], g_cbCardVal[i1] };
					getLogicVal(byCards4, 4);
					for (int i5 = i4; i5 < MAX_CARD_NUM; ++i5)
					{
						BYTE byCards5[5] = { g_cbCardVal[i5], g_cbCardVal[i4], g_cbCardVal[i3], g_cbCardVal[i2], g_cbCardVal[i1] };
						getLogicVal(byCards5, 5);
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
	for (int i = 0; i<num; ++i)
		cbIndex[i] = m_mapIndexByVal[cbData[i]];

	sort(cbIndex, cbIndex + 5, cmp);

	for (int i=0; i<num; ++i)
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

//获取牌值
BYTE CGameLogicPino::GetCardVal(BYTE cbData)
{
// 	if (m_mapIndexByVal[cbData] > 0)
// 		return (m_mapIndexByVal[cbData] - 1) / 4;
// 
// 	return 0xFF;
	return cbData & MASK_VALUE;
}

//获取牌颜色
BYTE  CGameLogicPino::GetCardCor(BYTE cbData)
{
// 	if (m_mapIndexByVal[cbData] > 0)
// 		return (m_mapIndexByVal[cbData] - 1) % 4;
// 
// 	return 0xFF;
	return (cbData & MASK_COLOR)>>4;
}

//获取牌组权值
DWORD CGameLogicPino::getLogicVal(const BYTE cbData[], BYTE num)
{
	DWORD dwKey = getKeyByCards(cbData, num);
	auto iter = m_mapKeyData.find(dwKey);
	if (iter != m_mapKeyData.end())
	{
		return iter->second;
	}
	else if (dwKey > 0)
	{
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
			++byCountCor[byCor];
			++byCountVal[byVal];
			byCountMaxCor[byVal] = max(byCountMaxCor[byVal], byCor);
			byMaxVal = max(byMaxVal, byVal);
		}		
		// 得到 enType、maxCor
		bool bSameCor = false;		// 同花
		for (int i=0; i<4; ++i)
		{
			if (byCountCor[i] == 5)
			{
				bSameCor = true;
				break;
			}
		}
    
		BYTE nInitLine = 0;
		if (byCountVal[6] == 1 && (byCountVal[0] == 1 || byCountVal[5] == 1))
			nInitLine = 1;
		
		BYTE nLineValNum = nInitLine;
		BYTE nSame4Val = 0xFF;
		BYTE nSame3Val = 0xFF;
		BYTE nSame2Val[2] = { 0xFF, 0xFF };
		BYTE nSame2Num = 0;
		for (int i = 0; i < 7; ++i)
		{
			if (byCountVal[i] == 1)
			{
				if (i != 6)
					nLineValNum++;
			}
      
			else
			{
				if (nLineValNum < 5)
					nLineValNum = nInitLine;

				if (byCountVal[i] == 4)
					nSame4Val = i;
				else if (byCountVal[i] == 3)
					nSame3Val = i;
				else if (byCountVal[i] == 2)
				{
					nSame2Val[nSame2Num++] = i;
				}
			}
		}		
    
		BYTE byMaxCor = byCountMaxCor[byMaxVal];
		BYTE byType = enType_null;
		if (nSame2Num == 1)
		{
			byType = enType_pair1;
			byMaxCor = byCountMaxCor[nSame2Val[0]];
		}
		if (nSame2Num == 2)
		{
			byType = enType_pair2;
			BYTE byMax = max(nSame2Val[0], nSame2Val[1]);
			byMaxCor = byCountMaxCor[byMax];
		}
		if(nSame3Val != 0xFF)
		{
			byType = enType_same3;
			byMaxCor = byCountMaxCor[nSame3Val];
		}
		if (nLineValNum == 5)
		{
			byType = enType_12345;
		}
    
		if (bSameCor)
		{
			byType = enType_sameCor;
		}
		if (nSame3Val != 0xFF && nSame2Num == 1)
		{
			byType = enType_same3_2;
			byMaxCor = byCountMaxCor[nSame3Val];
		}
		if (nSame4Val != 0xFF)
		{
			byType = enType_same4;
			byMaxCor = byCountMaxCor[nSame4Val];
		}
		if (nLineValNum == 5 && bSameCor)
		{
			byType = enType_flush;
		}

		BYTE  byFlagNum = 0;
		DWORD dwValFlag = 0;
		if (nSame4Val != 0xFF)
		{
			for (int n = 0; n < 4; ++n)
			{
				dwValFlag = dwValFlag << 3;
				dwValFlag += nSame4Val + 1;
				++byFlagNum;
			}
		}
		if (nSame3Val != 0xFF)
		{
			for (int n = 0; n < 3; ++n)
			{
				dwValFlag = dwValFlag << 3;
				dwValFlag += nSame3Val + 1;
				++byFlagNum;
			}
		}
    
		if (nSame2Num == 2)
		{
			for (int n = 0; n < 2; ++n)
			{
				dwValFlag = dwValFlag << 3;
				dwValFlag += nSame2Val[1] + 1;
				++byFlagNum;
			}
		}
		if (nSame2Num >= 1)
		{
			for (int n = 0; n < 2; ++n)
			{
				dwValFlag = dwValFlag << 3;
				dwValFlag += nSame2Val[0] + 1;
				++byFlagNum;
			}
		}
    
		for (int i = 6; i >= 0; --i)
		{
			if (byCountVal[i] == 1)
			{
				dwValFlag = dwValFlag << 3;
				dwValFlag += i+1;
				++byFlagNum;
			}
		}
		for (int i=byFlagNum; i<5; ++i)
			dwValFlag = dwValFlag << 3;

		DWORD dwVal = ((DWORD)byType << 17) + (dwValFlag << 2) + (byMaxCor & 3);
		m_mapKeyData[dwKey] = dwVal;
		return dwVal;
	}
	return 0;
}

//获取牌组类型
BYTE CGameLogicPino::GetCardType(const BYTE cbData[], BYTE num)
{
	DWORD dwVal = getLogicVal(cbData, num);
	return BYTE((dwVal & 1966080)>>17);
}

//对比扑克
bool CGameLogicPino::CompareCard(const BYTE cbData1[], const BYTE cbData2[], BYTE num1, BYTE num2)
{
	return getLogicVal(cbData1, num1) > getLogicVal(cbData2, num2);
}
//////////////////////////////////////////////////////////////////////////
