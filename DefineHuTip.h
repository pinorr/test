#pragma once

#include <algorithm>
#include <stdio.h>
#include <string>
#include <string.h>
#include <time.h>

using namespace std;

#pragma pack(1)

#define INVALID_VAL						0xFF			//�Ƿ�����
#define MAX_TOTAL_TYPE					34				//�齫������
#define MIN_HU_HUAN_CUN					5				//��С���泤��

enum enPlayMode
{
	enPlayMode_ZhuangFen,		//ׯ���
	enPlayMode_7Dui,			//�ɺ��߶�
	enPlayMode_NaiZi,			//��������
	enPlayMode_Max,
};

//������ʾ���
enum enCountFlag
{
	enCountFlag_7Dui		= 0x0001,
	enCountFlag_KeZiFirst	= 0x0002,
	enCountFlag_13Lan		= 0x0004,
};

//һ��������
enum BLOCK_TYPE_HU
{	
	BLOCK_22 = 1,				    //����
	BLOCK_234,						//˳�� ���� ���� �ṹ
	BLOCK_222,						//���� ���� ���� �ṹ, 3����ͬ
	BLOCK_2222_DIAN,   				//����--���,4����ͬ
	BLOCK_2222_BU,   				//����--����,4����ͬ
	BLOCK_2222_AN ,					//����--����,4����ͬ	
	BLOCK_MAX,
};


#define MAX_VAL_NUM						9
#define MAX_KEY_NUM						(MAX_VAL_NUM+1)		//9+����
//�齫��ɫ�����ࣩ����
enum enColorMJ
{
	enColorMJ_WAN = 0,  //��
	enColorMJ_TONG,     //Ͳ
	enColorMJ_TIAO,     //��
	enColorMJ_FenZi,    //�硢�� ��
	enColorMJ_Max,
};
//////////////////////////////////////////////////////////////
//����������ֵ
static const BYTE s_HuCardType[] = 
{
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,				//��
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,				//��
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,				//��
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37                            //���������з���
};

//////////////////////////////////////////////////////////////
//����ֵ�õ�����ֵ
inline BYTE getIndexByVal(BYTE byCard)
{
	BYTE cor = byCard / 0x10;
	BYTE val = byCard % 0x10;

	BYTE byIndex = INVALID_VAL;
	if (cor < 3)
	{
		if (val > 0 && val <= 9) 
			byIndex = (cor)*9 + val-1;
	}
	else if (cor == 3)	
	{
		if (val > 0 && val <= 4) 
			byIndex = 27 + val-1;
	}	
	return byIndex;	
}
//������ֵ�õ���ֵ
inline BYTE getValByIndex(BYTE byIndex)
{
	if (byIndex < MAX_TOTAL_TYPE)
	{
		return s_HuCardType[byIndex];
	}
	return INVALID_VAL;
}
//���Ƿ��ǺϷ���ֵ
inline bool IsValidCard(BYTE bCard)
{	
	return getIndexByVal(bCard) != INVALID_VAL;
}

struct stColorData
{
	BYTE	byCorType;					// enColorMJ
	BYTE	byNum;						// ������
	BYTE	byCount[MAX_KEY_NUM];		// ͳ������

	stColorData(){memset(this, 0, sizeof(*this));}	
	stColorData(BYTE byType, BYTE byVal[MAX_KEY_NUM]):byCorType(byType),byNum(0)
	{ 
		memcpy(byCount, byVal, MAX_KEY_NUM);
		for (int i=0; i<MAX_KEY_NUM; ++i)
			byNum += byCount[i];
	}
};

struct stCardData
{
	BYTE	byCardNum[MAX_TOTAL_TYPE];			// ͳ������
	BYTE	byNum;								// ����
	BYTE	byHuCard;							// ���������ƣ�������;��

	stCardData()
	{
		memset(this, 0, sizeof(*this));
	}	
	stCardData(BYTE byCards[], BYTE num):byNum(num)
	{
		memcpy(byCardNum, byCards, MAX_TOTAL_TYPE);
	}

	void clear()
	{
		memset(this, 0, sizeof(*this));
	}
	void getCardsData(BYTE byCardOut[], BYTE num, bool bAddHuCard = false)
	{
		BYTE byIndex = 0;
		for (int i=0; i<MAX_TOTAL_TYPE; ++i)
		{
			if (byCardNum[i] > 0 && byCardNum[i]+byIndex <= num)
			{
				memset(byCardOut+byIndex, getValByIndex(i), byCardNum[i]);
				byIndex += byCardNum[i];
			}
		}
		if (bAddHuCard && byHuCard != 0 && byIndex<num)
			byCardOut[byIndex] = byHuCard;
	}	
};

#pragma pack(1)