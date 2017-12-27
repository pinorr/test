#ifndef GAME_LOGIC_PINO_HEAD
#define GAME_LOGIC_PINO_HEAD

#pragma once

#include <algorithm>
#include <wtypes.h>
#include <map>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
//#define		USE_MAP

#define		MAX_CARD_NUM			28									//单副牌数目

//数值掩码
#define		MASK_COLOR				0xF0								//花色掩码
#define		MASK_VALUE				0x0F								//数值掩码
enum enCor
{
	enCor_block	= 0,				//方块
	enCor_plum,						//梅花
	enCor_heart,					//红桃
	enCor_spade,					//黑桃
	enCor_max,
};

enum enType
{
	enType_null,					//散牌类型
	enType_pair1,					//一对类型
	enType_pair2,					//两对类型
	enType_same3,					//三张类型
	enType_12345,					//顺子类型
	enType_sameCor,					//同花类型
	enType_same3_2,					//葫芦类型
	enType_same4,					//四张类型
	enType_flush,					//同花顺类型
	enType_max,
};

struct stData
{
	DWORD	dwVal;
	BYTE	byMaxCor;				//索引

	stData() { memset(this, 0, sizeof(*this)); }
};

//////////////////////////////////////////////////////////////////////////

//游戏逻辑类
class CGameLogicPino
{
	//变量定义
protected:	
	std::unordered_map<BYTE, BYTE>				m_mapIndexByVal;
	std::unordered_map<DWORD, DWORD>			m_mapKeyData;
	stData										m_KeyData[16384];

	//函数定义
public:
	//构造函数
	CGameLogicPino();
	//析构函数
	virtual ~CGameLogicPino();

	void initData();

	//类型函数
public:
	//由牌组得到key
	DWORD getKeyByCards(const BYTE cbData[], BYTE num);
	//由key得到牌组
	void  getCardsByKey(DWORD dwKey, BYTE cbData[5]);
	//获取牌组权值
	DWORD getLogicVal(const BYTE cbData[], BYTE num);
	//获取牌组权值
	DWORD getLogicVal2(const BYTE cbData[], BYTE num);
	//获取牌组类型
	BYTE  getCardsType(const BYTE cbData[], BYTE num);
	//获取获胜机率
	float get1WinPer(const BYTE cbData1[], const BYTE cbData2[], BYTE num1, BYTE num2);
private:
	DWORD getMapInfo(std::unordered_map<DWORD, DWORD> &mapUnOrder, BYTE byTemp[5], BYTE num, const bool bFlag[MAX_CARD_NUM]);
};

//////////////////////////////////////////////////////////////////////////

#endif
