#pragma once

#include <windows.h>

#define CT_INVALID						0								// 错误类型
#define CT_SINGLE						1								// 单牌类型
#define CT_ONE_DOUBLE					2								// 只有一对
#define CT_TWO_DOUBLE					3								// 两对牌型
#define CT_THREE						4								// 三张牌型
#define CT_FIVE_MIXED_FLUSH_NO_A		6								// 普通顺子
#define CT_FIVE_FLUSH					8								// 同花
#define CT_FIVE_THREE_DEOUBLE			9								// 三条一对
#define CT_FIVE_FOUR_ONE				10								// 四带一张
#define CT_FIVE_STRAIGHT_FLUSH			12								// 同花顺牌
#define CT_FIVE_MAX						14								// 

//扑克数目
#define CARD_COUNT						52								//扑克数目

static BYTE s_cbCardData[CARD_COUNT] =
{
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,	//方块 A - K
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,	//梅花 A - K
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,	//红桃 A - K
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,	//黑桃 A - K
};

//获取数值
static BYTE GetCardValue(BYTE cbCardData)
{
	return cbCardData & 0x0F;
}
//获取花色
static BYTE GetCardColor(BYTE cbCardData)
{
	return (cbCardData & 0xF0) >> 4;
}
//逻辑数值
static BYTE GetCardLogicValue(BYTE cbCardData)
{
	BYTE byVal = GetCardValue(cbCardData);
	if (byVal <= 1) byVal = 14;
	return byVal - 2;
}
//
static BYTE GetCard(BYTE cor, BYTE index)
{
	if (cor > 3 || index > 12) return 0;
	if (index == 12) index = -1;
	return cor * 0x10 + index + 2;
}
