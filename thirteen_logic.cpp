
#include "thirteen_logic.h"

bool	stAnswer::repairNote(BYTE byData[13][5])
{
	if (byNum != 2)
		return false;

	if (byCards[0][4] != 0 && byCards[1][4] != 0)
		return true;

	vector<BYTE> vctCards;
	//单牌 对子 三张 四张
	for (int m = 1; m <= 4; ++m)
		for (int i = 0; i < 13; ++i) {
			if (byData[i][4] == m) {
				for (int n = 0; n < 4; ++n)
					if (byData[i][n] == 1)
						vctCards.push_back(GetCard(n, i));
			}
		}

	int nDel = 0;
	if (byCards[0][4] == 0) {
		if (vctCards.empty()) return false;
		BYTE byTemp = vctCards[nDel++];
		byCards[0][4] = byTemp;
		//dwVal[0] = CGameLogic::GetLogicValue(byCards[0], 5);

		BYTE val = byTemp % 0x10;
		BYTE cor = byTemp / 0x10;
		val -= 2;
		if (val == 255) val = 12;
		--byData[val][4];
		--byData[val][cor];
		bRepair[0] = true;
	}
	if (byCards[1][4] == 0) {
		if (vctCards.empty()) return false;
		BYTE byTemp = vctCards[nDel++];
		byCards[1][4] = byTemp;
		//dwVal[1] = CGameLogic::GetLogicValue(byCards[1], 5);

		BYTE val = byTemp % 0x10;
		BYTE cor = byTemp / 0x10;
		val -= 2;
		if (val == 255) val = 12;
		--byData[val][4];
		--byData[val][cor];
		bRepair[1] = true;
	}
	return true;
}
void	stAnswer::undo_repair(BYTE byData[13][5])
{
	for (int i = 0; i < 2; ++i)
	{
		if (bRepair[i]) {
			BYTE val = byCards[i][4] % 0x10;
			BYTE cor = byCards[i][4] / 0x10;
			val -= 2;
			if (val == 255) val = 12;
			++byData[val][4];
			++byData[val][cor];
			bRepair[i] = false;
			byCards[i][4] = 0;
		}
	}
}
DWORD	stAnswer::push(BYTE cards[5], BYTE byData[13][5])
{
	if (byNum >= 3) return 0;
	DWORD dwTemp = CGameLogic::GetLogicValue(cards, 5);
	for (int i = 0; i < byNum; ++i)
		if (dwVal[i] < dwTemp)
			return 0;

	memcpy(byCards[byNum], cards, 5);
	for (int i = 0; i < 5; ++i)
	{
		if (cards[i] > 0) {
			BYTE val = cards[i] % 0x10;
			BYTE cor = cards[i] / 0x10;
			val -= 2;
			if (val == 255) val = 12;
			--byData[val][4];
			--byData[val][cor];
		}
	}
	dwVal[byNum] = dwTemp;
	++byNum;
	if (dwTemp == 0)
		int nnn = 0;
	return dwTemp;
}
void	stAnswer::pop(BYTE byData[13][5])
{
	if (byNum == 0) return;
	--byNum;
	for (int i = 0; i < 5; ++i)
	{
		if (byCards[byNum][i] > 0) {
			BYTE val = byCards[byNum][i] % 0x10;
			BYTE cor = byCards[byNum][i] / 0x10;
			val -= 2;
			if (val == 255) val = 12;
			++byData[val][4];
			++byData[val][cor];
			if (byData[val][cor] > 1)
				int nnn = 0;
		}
	}
}
void    stAnswer::getData(BYTE cards[13])
{
	if (byNum == 3){
		memcpy(cards, byCards[2], 3);
		memcpy(cards + 3, byCards[1], 5);
		memcpy(cards + 8, byCards[0], 5);
	}
}

void	CGameLogic::TryCards(BYTE byData[13][5], stAnswer &answer, DWORD dwScoreMax, vector<stAnswer> &vctAnswer)
{
	int nCardsNum = 0;
	for (int i = 0; i < 13; ++i)
		nCardsNum += byData[i][4];

	if (answer.byNum < 2)
	{
		if (dwScoreMax >= CT_FIVE_STRAIGHT_FLUSH << 22)	{
			//同花顺
			for (int i = 0; i < 13 - 4; ++i)
			{
				for (int n = 0; n < 4; ++n)
					if (byData[i][n] >= 1 && byData[i + 1][n] >= 1 && byData[i + 2][n] >= 1 && byData[i + 3][n] >= 1 && byData[i + 4][n] >= 1)
					{
						BYTE byCards[5] = { GetCard(n, i), GetCard(n, i + 1), GetCard(n, i + 2), GetCard(n, i + 3), GetCard(n, i + 4) };
						DWORD dwVal = answer.push(byCards, byData);
						if (dwVal > 0) {
							TryCards(byData, answer, dwVal, vctAnswer);
							answer.pop(byData);
						}
					}
			}
		}
		if (dwScoreMax >= CT_FIVE_FOUR_ONE << 22) {
			//四张
			for (int i = 0; i < 13; ++i)
			{
				if (byData[i][4] == 4) {
					BYTE byCards[5] = { GetCard(3, i), GetCard(2, i), GetCard(1, i), GetCard(0, i), 0 };
					DWORD dwVal = answer.push(byCards, byData);
					if (dwVal > 0) {
						TryCards(byData, answer, dwVal, vctAnswer);
						answer.pop(byData);
					}
				}
			}
		}
		if (dwScoreMax >= CT_FIVE_THREE_DEOUBLE << 22) {
			//三带二
			for (int i = 0; i < 13; ++i)
			{
				if (byData[i][4] == 3)
				{
					int j = 0;
					for (; j < 13; ++j)
						if (byData[j][4] == 2) break;

					if (j >= 13) {
						j = 0;
						for (; j < 13; ++j)
							if (j != i && byData[j][4] == 3) break;
					}

					if (j < 13) {
						BYTE index = 0;
						BYTE byCards[5] = {};
						for (int n = 0; n < 4; ++n)
							if (byData[i][n] == 1 && index < 5)
								byCards[index++] = GetCard(n, i);

						for (int n = 0; n < 4; ++n)
							if (byData[j][n] == 1 && index < 5)
								byCards[index++] = GetCard(n, j);

						DWORD dwVal = answer.push(byCards, byData);
						if (dwVal > 0) {
							TryCards(byData, answer, dwVal, vctAnswer);
							answer.pop(byData);
						}
					}
				}
			}
		}
		if (dwScoreMax >= CT_FIVE_FLUSH << 22) {
			//同花
			for (int n = 0; n < 4; ++n)
			{
				vector<BYTE> vctCards;
				for (int i = 0; i < 13; ++i) {
					if (byData[i][n] == 1)
						vctCards.push_back(GetCard(n, i));
				}
				if (vctCards.size() >= 5)
				{
					for (size_t i1 = 0; i1 < vctCards.size(); ++i1)
						for (size_t i2 = i1 + 1; i2 < vctCards.size(); ++i2)
							for (size_t i3 = i2 + 1; i3 < vctCards.size(); ++i3)
								for (size_t i4 = i3 + 1; i4 < vctCards.size(); ++i4)
									for (size_t i5 = i4 + 1; i5 < vctCards.size(); ++i5)
									{
										BYTE byCards[5] = { vctCards[i1], vctCards[i2], vctCards[i3], vctCards[i4], vctCards[i5] };
										DWORD dwVal = answer.push(byCards, byData);
										if (dwVal > 0) {
											TryCards(byData, answer, dwVal, vctAnswer);
											answer.pop(byData);
										}
									}
				}
			}
		}
		if (dwScoreMax >= CT_FIVE_MIXED_FLUSH_NO_A << 22) {
			//顺子
			for (int i = 0; i < 13 - 4; ++i)
			{
				if (byData[i][4] >= 1 && byData[i + 1][4] >= 1 && byData[i + 2][4] >= 1 && byData[i + 3][4] >= 1 && byData[i + 4][4] >= 1)
				{
					BYTE byCards[5] = {};
					for (int x = 0; x < 5; ++x)
						for (int n = 0; n < 4; ++n) {
							if (byData[i + x][n] == 1) {
								byCards[x] = GetCard(n, i + x);
								break;
							}
						}
					DWORD dwVal = answer.push(byCards, byData);
					if (dwVal > 0) {
						TryCards(byData, answer, dwVal, vctAnswer);
						answer.pop(byData);
					}
				}
			}
			// 2 3 4 5 A
			if (byData[0][4] >= 1 && byData[1][4] >= 1 && byData[2][4] >= 1 && byData[3][4] >= 1 && byData[12][4] >= 1)
			{
				BYTE byCards[5] = {};
				for (int x = 0; x < 4; ++x)
					for (int n = 0; n < 4; ++n) {
						if (byData[x][n] == 1) {
							byCards[x] = GetCard(n, x);
							break;
						}
					}
				for (int n = 0; n < 4; ++n) {
					if (byData[12][n] == 1) {
						byCards[4] = GetCard(n, 12);
						break;
					}
				}
				DWORD dwVal = answer.push(byCards, byData);
				if (dwVal > 0) {
					TryCards(byData, answer, dwVal, vctAnswer);
					answer.pop(byData);
				}
			}
		}
		if (dwScoreMax >= CT_THREE << 22) {
			//三张
			for (int i = 12; i >= 0; --i)
			{
				if (byData[i][4] == 3) {
					BYTE index = 0;
					BYTE byCards[5] = {};
					for (int n = 0; n < 4; ++n)
						if (byData[i][n] == 1)
							byCards[index++] = GetCard(n, i);

					//再选两个最小单牌
					for (int i = 0; i < 13; ++i)
					{
						if (byData[i][4] == 1)
							for (int n = 0; n < 4; ++n)
								if (byData[i][n] == 1)
									byCards[index++] = GetCard(n, i);

						if (index >= 5) break;
					}
					if (index == 5) {
						DWORD dwVal = answer.push(byCards, byData);
						if (dwVal > 0){
							TryCards(byData, answer, dwVal, vctAnswer);
							answer.pop(byData);
						}
					}
				}
			}
		}
		vector<BYTE> vctPairs;
		for (int i = 0; i < 13; ++i)
		{
			if (byData[i][4] == 2)
				vctPairs.push_back(i);
		}

		if (dwScoreMax >= CT_TWO_DOUBLE << 22) {
			//两对
			if (vctPairs.size() >= 2)
			{
				//多对子时，最大的对子放到最上面（三张的那组）
				BYTE byMax = vctPairs.size() - 1;
				if (answer.byNum == 0) {
					if (vctPairs.size() >= 5)
						byMax -= 1;
					else if (vctPairs.size() >= 4)
						byMax -= 2;
				}
				else if (vctPairs.size() >= 3)
					byMax -= 1;

				BYTE index = 0;
				BYTE byCards[5] = {};
				for (int n = 0; n < 4; ++n)
					if (byData[vctPairs[byMax]][n] == 1)
						byCards[index++] = GetCard(n, vctPairs[byMax]);

				for (int n = 0; n < 4; ++n)
					if (byData[vctPairs[0]][n] == 1)
						byCards[index++] = GetCard(n, vctPairs[0]);

				//再选一个最小单牌
				for (int i = 0; i < 13; ++i)
				{
					if (byData[i][4] == 1)
						for (int n = 0; n < 4; ++n)
							if (byData[i][n] == 1)
								byCards[index++] = GetCard(n, i);

					if (index >= 5) break;
				}
				DWORD dwVal = answer.push(byCards, byData);
				if (dwVal > 0){
					TryCards(byData, answer, dwVal, vctAnswer);
					answer.pop(byData);
				}
			}
		}

		if (dwScoreMax >= CT_ONE_DOUBLE << 22) {
			//一对
			if (vctPairs.size() > 0)
			{
				BYTE byMax = vctPairs.size() - 1;

				BYTE index = 0;
				BYTE byCards[5] = {};
				for (int n = 0; n < 4; ++n)
					if (byData[vctPairs[byMax]][n] == 1)
						byCards[index++] = GetCard(n, vctPairs[byMax]);

				//再选三个最小单牌
				for (int i = 0; i < 13; ++i)
				{
					if (byData[i][4] == 1)
						for (int n = 0; n < 4; ++n)
							if (byData[i][n] == 1)
								byCards[index++] = GetCard(n, i);

					if (index >= 5) break;
				}
				if (index == 5) {
					DWORD dwVal = answer.push(byCards, byData);
					if (dwVal > 0){
						TryCards(byData, answer, dwVal, vctAnswer);
						answer.pop(byData);
					}
				}
			}
		}

		if (dwScoreMax >= CT_SINGLE << 22) {
			//单牌
			vector<BYTE> vctSingle;
			for (int i = 0; i < 13; ++i)
			{
				if (byData[i][4] == 1)
				{
					for (int n = 0; n < 4; ++n)
						if (byData[i][n] == 1)
							vctSingle.push_back(GetCard(n, i));
				}
			}
			if (vctSingle.size() > 5) {
				BYTE byCards[5] = { vctSingle[vctSingle.size() - 1], vctSingle[0], vctSingle[1], vctSingle[2], vctSingle[3] };
				DWORD dwVal = answer.push(byCards, byData);
				if (dwVal > 0){
					TryCards(byData, answer, dwVal, vctAnswer);
					answer.pop(byData);
				}
			}
		}
	}
	else
	{
		if (answer.repairNote(byData) == false) return;
		if (dwScoreMax >= CT_THREE << 22) {
			//三张
			for (int i = 0; i < 13; ++i)
			{
				if (byData[i][4] == 3) {
					BYTE index = 0;
					BYTE byCards[5] = {};
					for (int n = 0; n < 4; ++n)
						if (byData[i][n] == 1)
							byCards[index++] = GetCard(n, i);

					DWORD dwVal = answer.push(byCards, byData);
					if (dwVal > 0) {
						vctAnswer.push_back(answer);
						answer.pop(byData);
					}
				}
			}
		}

		if (dwScoreMax >= CT_ONE_DOUBLE << 22) {
			vector<BYTE> vctPairs;
			for (int i = 0; i < 13; ++i)
			{
				if (byData[i][4] == 2)
					vctPairs.push_back(i);
			}

			//一对
			if (vctPairs.size() > 0)
			{
				BYTE byMax = vctPairs.size() - 1;

				BYTE index = 0;
				BYTE byCards[5] = {};
				for (int n = 0; n < 4; ++n)
					if (byData[vctPairs[byMax]][n] == 1)
						byCards[index++] = GetCard(n, vctPairs[byMax]);

				//再选一个最小单牌
				for (int i = 0; i < 13; ++i)
				{
					if (byData[i][4] == 1)
						for (int n = 0; n < 4; ++n)
							if (byData[i][n] == 1)
								byCards[index++] = GetCard(n, i);

					if (index >= 3) break;
				}

				DWORD dwVal = answer.push(byCards, byData);
				if (dwVal > 0) {
					vctAnswer.push_back(answer);
					answer.pop(byData);
				}
			}
		}

		if (dwScoreMax >= CT_SINGLE << 22) {
			//单牌
			vector<BYTE> vctSingle;
			for (int i = 0; i < 13; ++i)
			{
				if (byData[i][4] == 1)
				{
					for (int n = 0; n < 4; ++n)
						if (byData[i][n] == 1) {
							vctSingle.push_back(GetCard(n, i));
							break;
						}
				}
			}
			if (vctSingle.size() >= 3) {
				BYTE bySize = vctSingle.size();
				BYTE byCards[5] = { vctSingle[bySize - 1], vctSingle[bySize - 2], vctSingle[bySize - 3], 0, 0 };
				DWORD dwVal = answer.push(byCards, byData);
				if (dwVal > 0) {
					vctAnswer.push_back(answer);
					answer.pop(byData);
				}
			}
		}
		answer.undo_repair(byData);
	}
}
//
void	CGameLogic::JudgeCards(const BYTE cbCardData[], BYTE cbCount, vector<stAnswer> &vctOut)
{
	BYTE byData[13][5] = {};
	for (int i = 0; i < cbCount; ++i){
		BYTE val = cbCardData[i] % 0x10;
		BYTE cor = cbCardData[i] / 0x10;
		val -= 2;
		if (val == 255) val = 12;

		++byData[val][4];
		++byData[val][cor];
	}
	stAnswer answer;
	TryCards(byData, answer, CT_FIVE_MAX << 22, vctOut);
}
//逻辑牌值
DWORD	CGameLogic::GetLogicValue(const BYTE cbCardData[], BYTE cbCardCount)
{
	//变量定义
	int  nValFlag = 0;
	int  nMaxID[5] = {};	//最大牌
	BYTE byMaxCor[5] = {};	//最大牌花色
	if (cbCardCount <= 5 && cbCardCount >= 3)
	{
		BYTE num = 0;
		BYTE byCardVal[13] = {};
		for (int i = 0; i<cbCardCount; ++i)
		{
			if (cbCardData[i] > 0) {
				BYTE byVal = GetCardLogicValue(cbCardData[i]);
				++byCardVal[byVal];
				++num;
			}
		}
		cbCardCount = num;

		BYTE bySameMax = 0;
		vector<BYTE> vctDouble;
		//统计数据
		for (int i = 0; i < 13; ++i)
		{
			bySameMax = max(bySameMax, byCardVal[i]);
			nMaxID[byCardVal[i]] = i;
			if (byCardVal[i] == 2)
				vctDouble.push_back(i);
		}

		//找到指定值最大颜色
		for (int i = 0; i < cbCardCount; ++i)
		{
			BYTE byVal = GetCardLogicValue(cbCardData[i]);
			for (int n = 1; n < 5; ++n) {
				if (byVal == nMaxID[n])
					byMaxCor[n] = max(byMaxCor[n], GetCardColor(cbCardData[i]));
			}
		}

		//变量定义
		bool cbSameColor = true, bLineCard = false;
		//判断是否是顺子
		for (int i = 0; i < 13; ++i)
		{
			if (byCardVal[i] == 1 && i + 4 < 13)
			{
				if (byCardVal[i + 1] == 1 && byCardVal[i + 2] == 1 && byCardVal[i + 3] == 1 && byCardVal[i + 4] == 1)
					bLineCard = true;
			}
		}
		//特殊 2 3 4 5 A
		if (byCardVal[0] == 1 && byCardVal[1] == 1 && byCardVal[2] == 1 && byCardVal[3] == 1 && byCardVal[12] == 1)
			bLineCard = true;

		//判断是否是同花
		BYTE cbFirstColor = GetCardColor(cbCardData[0]);
		for (BYTE i = 1; i<cbCardCount; i++)
			if (GetCardColor(cbCardData[i]) != cbFirstColor) cbSameColor = false;

		if (cbCardCount != 5)
			cbSameColor = false;

		//数值比较标志
		for (int n = 4; n >= 1; --n)
			for (int i = nMaxID[n]; i >= 0; --i) {
				if (byCardVal[i] == n) {
					for (int m = 0; m < n; ++m)
						nValFlag = (nValFlag << 4) + i;
				}
			}

		//顺金类型
		if (cbSameColor && bLineCard)
			return ((int)CT_FIVE_STRAIGHT_FLUSH << 22) + (nValFlag << 2) + (byMaxCor[1] & 0x03);

		//顺子类型
		if (!cbSameColor && bLineCard)
			return ((int)CT_FIVE_MIXED_FLUSH_NO_A << 22) + (nValFlag << 2) + (byMaxCor[1] & 0x03);

		//金花类型
		if (cbSameColor && !bLineCard)
			return ((int)CT_FIVE_FLUSH << 22) + (nValFlag << 2) + (byMaxCor[1] & 0x03);

		//对子和豹子判断
		if (bySameMax == 2) {
			if (vctDouble.size() >= 2)
				return ((int)CT_TWO_DOUBLE << 22) + (nValFlag << 2) + (byMaxCor[1] & 0x03);
			else
				return ((int)CT_ONE_DOUBLE << 22) + (nValFlag << 2) + (byMaxCor[1] & 0x03);
		}
		else if (bySameMax == 3) {
			if (vctDouble.size() == 1)
				return ((int)CT_FIVE_THREE_DEOUBLE << 22) + (nValFlag << 2) + (byMaxCor[3] & 0x03);
			else
				return ((int)CT_THREE << 22) + (nValFlag << 2) + (byMaxCor[3] & 0x03);
		}
		else if (bySameMax == 4)
			return ((int)CT_FIVE_FOUR_ONE << 22) + (nValFlag << 2) + (byMaxCor[4] & 0x03);
	}
	return ((int)CT_SINGLE << 22) + (nValFlag << 2) + (byMaxCor[1] & 0x03);
}
//逻辑大小
DWORD	CGameLogic::GetCardType(const BYTE cbCardData[], BYTE cbCardCount)
{
	DWORD dwLogicVal = GetLogicValue(cbCardData, cbCardCount);
	return BYTE(dwLogicVal >> 22);
}
//大小比较
bool	CGameLogic::CompareCard(const BYTE cbCardData1[], BYTE cbCount1, const BYTE cbCardData2[], BYTE cbCount2)
{
	return GetLogicValue(cbCardData1, cbCount1) > GetLogicValue(cbCardData2, cbCount2);
}

