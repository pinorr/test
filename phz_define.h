
/********************************************************************
created:	pinorr
file base:	logic.h
author:		pinorr	Q 505971450
purpose:	定义
*********************************************************************/

#include <windows.h>

#define		MAX_CARDS_NUM		84			// 
#define		MAX_TYPE			21			// 20张牌 + 赖子
#define		INVALID_VAL			0x00

//结点类型
enum enNode
{
	enNode_error,
	enNode_line,
	enNode_27A,
	enNode_DDd,
	enNode_num2,
	enNode_num3,
	enNode_num4,
	enNode_max,
};

//规则类型
enum enPlayType
{
	enPlayType_NORMAL = 0,		//一般
	enPlayType_XDD = 1,			//小大搭
	enPlayType_27A = 2,			//二七十
	enPlayType_123 = 3,			//一二三
	enPlayType_WANG = 4,		//王
	enPlayType_max,
};

#define TY_MY	(1+(1<<enPlayType_WANG)+(1<<enPlayType_123)+(1<<enPlayType_27A)+(1<<enPlayType_XDD))
//#define TY_MY	(1+(1<<enPlayType_WANG)+(1<<enPlayType_123)+(1<<enPlayType_XDD))

static BYTE s_byIndex[MAX_TYPE] =
{
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,		// 一 ~ 十
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,		// 壹 ~ 拾
	0xFF,															// 赖子
};

static bool		isHavePlayType(enPlayType type)
{
	if (type >= enPlayType_max)
		return false;
	return (TY_MY & (1 << type)) != 0;
}

static BYTE		getIndexByVal(BYTE byCard)
{
	if (byCard == 0xFF) 
		return 20;
	BYTE byCor = byCard & 0xF0;
	BYTE byIndex = (byCor >> 4) * 10 + (byCard & 0x0F) - 1;
	if (byIndex >= MAX_TYPE)
		return 0xFF;
	return byIndex;
}

static BYTE		getValByIndex(BYTE byIndex)
{
	if (byIndex < MAX_TYPE)
		return s_byIndex[byIndex];
	return INVALID_VAL;
}

static LONG64	getFlagByCards(BYTE byCards[4])
{
	LONG64	llFlag = 0;		// (0~7) 3 * 10
	for (int i = 0; i < 4; ++i)
	{
		BYTE byIndex = getIndexByVal(byCards[i]);
		if (byIndex >= 20) break;
		llFlag += (LONG64)1 << (byIndex * 3);
	}
	return llFlag;
}
static void		getCardsByVal(LONG64 llVal, BYTE byOut[4])
{
	BYTE byAll = 0;
	for (int i = 0; i < MAX_TYPE; ++i)
	{
		BYTE num = ((llVal >> (i * 3)) & 7);
		if (num > 0)
		{
			memset(&byOut[byAll], getValByIndex(i), min(4-byAll, num));
			byAll += num;
		}
		if (byAll >= 4)
			break;
	}
}

static BYTE		getNodeIndex(BYTE byCards[4])
{
	BYTE byFlag = 0;
	BYTE byMin = 0xFF;
	BYTE byBig = 0;
	BYTE byCount = 0;
	BYTE byNum[10] = {};
	for (int i = 0; i < 4; ++i)
	{
		BYTE byIndex = getIndexByVal(byCards[i]);
		if (byIndex >= 20) break;
		if (byIndex >= 10) ++byBig;
		byMin = min(byMin, byIndex % 10);
		++byNum[byIndex % 10];
		++byCount;
	}
	switch (byCount)
	{
	case 2:
		{
			if (byNum[byMin] == 2 && byBig != 1)
				return (enNode_num2 << 5) + ((byBig >= 2) << 4) + byMin;
		}
		break;
	case 3:
		{
			if (byNum[byMin] == 3)
			{
				if (byBig == 3 || byBig == 0)
					return (enNode_num3 << 5) + ((byBig >= 2) << 4) + byMin;
				else
					return (enNode_DDd << 5) + ((byBig >= 2) << 4) + byMin;
			}
			else if (byMin < 8 && byNum[byMin] == 1 && byNum[byMin+1] == 1 && byNum[byMin+2] == 1)
			{
				return (enNode_line << 5) + ((byBig >= 2) << 4) + byMin;
			}
			else if (byNum[1] == 1 && byNum[6] == 1 && byNum[9] == 1)		//27A
			{
				if (byBig == 3 || byBig == 0)
					return (enNode_27A << 5) + ((byBig >= 2) << 4) + byMin;
			}
		}
		break;
	case 4:
		{
			if (byNum[byMin] == 4 && (byBig == 0 || byBig == 4))
			{
				return (enNode_num4 << 5) + ((byBig >= 2) << 4) + byMin;
			}
		}
		break;
	}
	return 0;
}
static int		getNodeCards(BYTE byIndex, BYTE byOut[4] = NULL)
{
	BYTE byNode = (byIndex & 224) >> 5;
	bool bBig = (byIndex & 16) > 0;
	BYTE byMin = byIndex & 15;
	if (byNode > enNode_error && byNode < enNode_max && byMin < 10)
	{
		switch (byNode)
		{
		case enNode_line:
		{
			if (byMin < 8)	{
				if (byOut)	{
					for (int i = 0; i < 3; ++i)
						byOut[i] = getValByIndex(byMin + i + bBig * 10);
				}

				if (byMin == 0)
					return bBig ? 6 : 3;
				return 0;
			}
		}
		break;
		case enNode_27A:
		{
			if (byMin == 1)	{
				if (byOut)	{
					byOut[0] = getValByIndex(1 + bBig * 10);
					byOut[1] = getValByIndex(6 + bBig * 10);
					byOut[2] = getValByIndex(9 + bBig * 10);
				}
				return bBig ? 6 : 3;
			}
		}
		break;
		case enNode_DDd:
		{
			if (byOut)	{
				byOut[0] = getValByIndex(byMin);
				byOut[1] = getValByIndex(byMin + bBig * 10);
				byOut[2] = getValByIndex(byMin + 10);
			}
			return 0;
		}
		break;
		case enNode_num2:
		{
			if (byOut)
				memset(byOut, getValByIndex(byMin + bBig * 10), 2);
			return 0;
		}
		break;
		case enNode_num3:
		{
			if (byOut)
				memset(byOut, getValByIndex(byMin + bBig * 10), 3);
			return bBig ? 6 : 3;
		}
		break;
		case enNode_num4:
		{
			if (byOut)
				memset(byOut, getValByIndex(byMin + bBig * 10), 4);
			return bBig ? 12 : 9;
		}
		break;
		}
	}
	return -1;
}

static LONG64	getKey(BYTE n1, BYTE n2, BYTE n3 = 0, BYTE n4 = 0)
{
	LONG64 llKey = 0;
	BYTE index = getIndexByVal(n1);
	if (index < MAX_TYPE)
		llKey += (LONG64)1 << (index * 3);
	index = getIndexByVal(n2);
	if (index < MAX_TYPE)
		llKey += (LONG64)1 << (index * 3);
	index = getIndexByVal(n3);
	if (index < MAX_TYPE)
		llKey += (LONG64)1 << (index * 3);
	index = getIndexByVal(n4);
	if (index < MAX_TYPE)
		llKey += (LONG64)1 << (index * 3);
	return llKey;
}
static LONG64	getKeyIndex(BYTE index1, BYTE index2, BYTE index3 = MAX_TYPE, BYTE index4 = MAX_TYPE)
{
	LONG64 llKey = 0;
	if (index1 < MAX_TYPE)
		llKey += (LONG64)1 << (index1 * 3);
	if (index2 < MAX_TYPE)
		llKey += (LONG64)1 << (index2 * 3);
	if (index3 < MAX_TYPE)
		llKey += (LONG64)1 << (index3 * 3);
	if (index4 < MAX_TYPE)
		llKey += (LONG64)1 << (index4 * 3);
	return llKey;
}

inline int		getCardsNum(LONG64 llVal, BYTE n)
{
	if (n < MAX_TYPE)
		return BYTE((llVal >> (n * 3)) & 7);

	return 0;
}

inline BYTE		getNumByKey(LONG64 llVal)
{
	BYTE nNum = 0;
	for (int i = 0; i < MAX_TYPE; ++i)
		nNum += (llVal >> (3 * i)) & 7;
	return nNum;
}

inline bool		isValidNullVal(LONG64 llVal)
{
	if (getNumByKey(llVal) > 15) return false;
	for (int i = 0; i < 20; ++i)
	{
		if (getCardsNum(llVal, i) > 2)
			return false;
	}
	return true;
}

inline bool		isContainKey(LONG64 llVal, LONG64 llKey)
{
	for (int i = 0; i < MAX_TYPE; ++i)
	{
		BYTE key = (llKey >> (3 * i)) & 7;
		if (key > 0 && key > ((llVal >> (3 * i)) & 7))
			return false;
	}
	return true;
}
