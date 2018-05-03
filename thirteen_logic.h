#pragma once

#include <vector>
#include "thirteen_define.h"

using namespace std;

struct stAnswer
{
	BYTE  byNum;
	BYTE  byCards[3][5];
	DWORD dwVal[3];
	bool  bRepair[2];

	stAnswer() { memset(this, 0, sizeof(*this)); }

	bool	repairNote(BYTE byData[13][5]);
	void	undo_repair(BYTE byData[13][5]);
	DWORD	push(BYTE cards[5], BYTE byData[13][5]);
	void	pop(BYTE byData[13][5]);
	void    getData(BYTE cards[13]);
};

class CGameLogic
{
public:
	//
	static void		CGameLogic::TryCards(BYTE byData[13][5], stAnswer &answer, DWORD dwScoreMax, vector<stAnswer> &vctAnswer);
	//
	static void		CGameLogic::JudgeCards(const BYTE cbCardData[], BYTE cbCount, vector<stAnswer> &vctOut);
	//逻辑牌值
	static DWORD	CGameLogic::GetLogicValue(const BYTE cbCardData[], BYTE cbCardCount);
	//逻辑大小
	static DWORD	CGameLogic::GetCardType(const BYTE cbCardData[], BYTE cbCardCount);
	//大小比较
	static bool		CGameLogic::CompareCard(const BYTE cbCardData1[], BYTE cbCount1, const BYTE cbCardData2[], BYTE cbCount2);
};
