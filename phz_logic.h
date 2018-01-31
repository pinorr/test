/********************************************************************
created:	pinorr
file base:	logic.h
author:		pinorr	Q 505971450
purpose:	跑胡子基本算法
*********************************************************************/

#include "cmd_define.h"
#include <hash_map>
#include <algorithm>  

using namespace std;

hash_map<LONG64, BYTE>	g_mapKeyAll, g_mapKeyVal, g_mapKeyDui;

struct stAnswer
{
	BYTE	num;
	LONG64	llNode[7];
	BYTE	byNodeXi[7];
	BYTE	byIndex[7];

	stAnswer() { memset(this, 0, sizeof(*this)); }
	LONG64	push(LONG64 llVal, BYTE index)
	{		
		hash_map<LONG64, BYTE>::iterator iter = g_mapKeyAll.find(llVal);
		if (iter != g_mapKeyAll.end() && num < 7)
		{
			byNodeXi[num] = iter->second;
			byIndex[num] = index;
			llNode[num++] = llVal;
			return llVal;
		}
		return 0;
	}
	LONG64	pop()
	{
		if (num > 0)
			return llNode[--num];
		return 0;
	}
	BYTE	getHuXi()
	{		
		BYTE byXi = 0;
		for (int i = 0; i < num; ++i)
			byXi += byNodeXi[i];
		return byXi;
	}
};

class CLogicPHZ
{
public:	
	CLogicPHZ()
	{
		m_nAnswerNum = 0;
		m_nValNum = 0;
		m_nNullNum = 0;
	}
	void	TrainKey()
	{
		// 提跑
		for (BYTE val = 1; val <= 10; ++val)
		{
			BYTE VAL = val + 0x10;
			addMapVal(getKey(val, val, val, val), 9);
			addMapVal(getKey(VAL, VAL, VAL, VAL), 12);
			if (isHavePlayType(enPlayType_WANG))
			{
				addMapVal(getKey(0xFF, val, val, val), 9);
				addMapVal(getKey(0xFF, 0xFF, val, val), 9);
				addMapVal(getKey(0xFF, 0xFF, 0xFF, val), 9);
				addMapVal(getKey(0xFF, VAL, VAL, VAL), 12);
				addMapVal(getKey(0xFF, 0xFF, VAL, VAL), 12);
				addMapVal(getKey(0xFF, 0xFF, 0xFF, VAL), 12);
				addMapVal(getKey(0xFF, 0xFF, 0xFF, 0xFF), 12);
			}
		}
		// 偎碰
		for (BYTE val = 1; val <= 10; ++val)
		{
			BYTE VAL = val + 0x10;
			addMapVal(getKey(val, val, val, 0), 3);
			addMapVal(getKey(VAL, VAL, VAL, 0), 6);
			if (isHavePlayType(enPlayType_XDD))
			{
				addMapVal(getKey(VAL, val, val, 0), 0);
				addMapVal(getKey(VAL, VAL, val, 0), 0);
				if (isHavePlayType(enPlayType_WANG))
					addMapVal(getKey(0xFF, VAL, val, 0), 0);
			}
			if (isHavePlayType(enPlayType_WANG))
			{
				addMapVal(getKey(0xFF, val, val, 0), 3);
				addMapVal(getKey(0xFF, 0xFF, val, 0), 3);
				addMapVal(getKey(0xFF, VAL, VAL, 0), 6);
				addMapVal(getKey(0xFF, 0xFF, VAL, 0), 6);
				addMapVal(getKey(0xFF, 0xFF, 0xFF, 0), 6);
			}
		}
		//27A
		if (isHavePlayType(enPlayType_27A))
		{
			addMapVal(getKey(0x0A, 0x07, 0x02, 0), 3);
			addMapVal(getKey(0x1A, 0x17, 0x12, 0), 6);
			if (isHavePlayType(enPlayType_WANG))
			{
				addMapVal(getKey(0xFF, 0x07, 0x02, 0), 3);
				addMapVal(getKey(0x0A, 0xFF, 0x02, 0), 3);
				addMapVal(getKey(0x0A, 0x07, 0xFF, 0), 3);
				addMapVal(getKey(0xFF, 0xFF, 0x02, 0), 3);
				addMapVal(getKey(0x0A, 0xFF, 0xFF, 0), 3);
				addMapVal(getKey(0xFF, 0x07, 0xFF, 0), 3);

				addMapVal(getKey(0xFF, 0x17, 0x12, 0), 6);
				addMapVal(getKey(0x1A, 0xFF, 0x12, 0), 6);
				addMapVal(getKey(0x1A, 0x17, 0xFF, 0), 6);
				addMapVal(getKey(0xFF, 0xFF, 0x12, 0), 6);
				addMapVal(getKey(0x1A, 0xFF, 0xFF, 0), 6);
				addMapVal(getKey(0xFF, 0x17, 0xFF, 0), 6);
			}
		}
		// 顺子
		for (BYTE val = 1; val <= 8; ++val)
		{
			int xi = 0;
			if (isHavePlayType(enPlayType_123))
				xi = (val == 1) ? 3 : 0;

			BYTE VAL = val + 0x10;
			addMapVal(getKey(val, val + 1, val + 2, 0x00), xi);
			addMapVal(getKey(0x10 + val, 0x10 + val + 1, 0x10 + val + 2, 0x00), xi * 2);
			if (isHavePlayType(enPlayType_WANG))
			{
				addMapVal(getKey(0xFF,	val + 1,	val + 2,	0), xi);
				addMapVal(getKey(val,	0xFF,		val + 2,	0), xi);
				addMapVal(getKey(val,	val + 1,	0xFF,		0), xi);
				addMapVal(getKey(val,	0xFF,		0xFF,		0), xi);
				addMapVal(getKey(0xFF,	val + 1,	0xFF,		0), xi);
				addMapVal(getKey(0xFF,	0xFF,		val + 2,	0), xi);

				addMapVal(getKey(0xFF,	VAL + 1,	VAL + 2,	0), xi * 2);
				addMapVal(getKey(VAL,	0xFF,		VAL + 2,	0), xi * 2);
				addMapVal(getKey(VAL,	VAL + 1,	0xFF,		0), xi * 2);
				addMapVal(getKey(VAL,	0xFF,		0xFF,		0), xi * 2);
				addMapVal(getKey(0xFF,	VAL + 1,	0xFF,		0), xi * 2);
				addMapVal(getKey(0xFF,	0xFF,		VAL + 2,	0), xi * 2);
			}
		}		

		hash_map<LONG64, BYTE>::iterator iter = g_mapKeyAll.begin();
		for (; iter != g_mapKeyAll.end(); ++iter)
		{
			if (iter->second == 0) {
				m_mapKeyIndex[iter->first] = 158 + m_vctKeyNull.size();
				m_vctKeyNull.push_back(iter->first);
			}
			else {
				m_mapKeyIndex[iter->first] = m_vctKeyVal.size();
				m_vctKeyVal.push_back(iter->first);				
			}				
		}

		// 对子
		for (BYTE val = 1; val <= 10; ++val)
		{
			BYTE VAL = val + 0x10;
// 			g_mapKeyDui[getKey(val, val, 0, 0)] = 0;
// 			g_mapKeyDui[getKey(VAL, VAL, 0, 0)] = 0;
			addMapVal(getKey(val, val, 0, 0), 0);
			addMapVal(getKey(VAL, VAL, 0, 0), 0);
			if (isHavePlayType(enPlayType_WANG))
			{
// 				g_mapKeyDui[getKey(val, 0xFF, 0, 0)] = 0;
// 				g_mapKeyDui[getKey(VAL, 0xFF, 0, 0)] = 0;
// 				g_mapKeyDui[getKey(0xFF, 0xFF, 0, 0)] = 0;
				addMapVal(getKey(val, 0xFF, 0, 0), 0);
				addMapVal(getKey(VAL, 0xFF, 0, 0), 0);
				addMapVal(getKey(0xFF, 0xFF, 0, 0), 0);
			}
		}
		
		m_nValNum = m_vctKeyVal.size();
		m_nNullNum = m_vctKeyNull.size();
		cout << g_mapKeyAll.size() << endl;
		cout << m_vctKeyNull.size() << endl;
		cout << m_vctKeyVal.size() << endl;
	}

	int		getHuKeyInit(BYTE *cbCards, BYTE num, BYTE byThreshXi, bool bFindAll = false)
	{		
		LONG64 llVal = 0;
		for (int i = 0; i < num; ++i)
		{
			BYTE index = getIndexByVal(cbCards[i]);
			if (index >= MAX_TYPE) return false;
			llVal += (LONG64)1 << (3 * index);
		}

		vector<BYTE>	vctIndex3;
		stAnswer		answer;
		for (int i = 0; i < 20; ++i)
		{
			BYTE num = getCardsNum(llVal, i);
			if (num == 4)
			{
				LONG64 llNode = (LONG64)4 << (i * 3);
				hash_map<LONG64, BYTE>::iterator iter = m_mapKeyIndex.find(llNode);
				if (iter == m_mapKeyIndex.end())
					continue;						// 不可能到此
				answer.push(llNode, iter->second);
				llVal -= llNode;
			}
			else if (num == 3)
			{
				vctIndex3.push_back(i);	
				llVal -= LONG64((LONG64)3 << (i * 3));
			}
		}
		m_nAnswerNum = 0;
		BYTE nKing = getCardsNum(llVal, 20);
		BYTE nNum3 = vctIndex3.size();
		for (int flag = 0; flag < ((int)1 << nNum3); ++flag)
		{
			if (!bFindAll && m_nAnswerNum > 0)
				return m_nAnswerNum;

			BYTE nNum = 0;
			for (int i = 0; i < 8; ++i)
				nNum += (flag >> i) & 1;
			if (nNum > nKing) continue;

			LONG64		llTemp = llVal;
			stAnswer	stTemp = answer;
			for (size_t i = 0; i < vctIndex3.size(); ++i)
			{
				LONG64 llNode = (LONG64)3 << (vctIndex3[i] * 3);
				if (flag & (1 << i)) {
					llNode += (LONG64)1 << 60;
					llTemp -= (LONG64)1 << 60;
				}					
				hash_map<LONG64, BYTE>::iterator iter = m_mapKeyIndex.find(llNode);
				if (iter == m_mapKeyIndex.end())
					continue;						// 不可能到此
				stTemp.push(llNode, iter->second);
			}
			getHuKey(llTemp, byThreshXi, stTemp, 0, bFindAll);
		}		
		return m_nAnswerNum;
	}

	void	getHuKey(LONG64 llVal, BYTE byThreshXi, stAnswer &answer, int nBegin, bool bFindAll)
	{
		if (m_nAnswerNum >= 1024)
			return;
		if (!bFindAll && m_nAnswerNum > 0)
			return;

		BYTE byLeft = getNumByKey(llVal);
		if (byLeft == 0){
			if (answer.getHuXi() >= byThreshXi)
				m_answer[m_nAnswerNum++] = answer;
			return;
		}
		else if (byLeft % 3 == 1){
			return;
		}
		else if (byLeft == 3 || byLeft == 2){
			hash_map<LONG64, BYTE>::iterator iter = g_mapKeyAll.find(llVal);
			if (iter != g_mapKeyAll.end())
			{
				iter = m_mapKeyIndex.find(llVal);
				BYTE index = iter != m_mapKeyIndex.end() ? iter->second : 0xFF;
				answer.push(llVal, index);
				if (answer.getHuXi() >= byThreshXi)
					m_answer[m_nAnswerNum++] = answer;
				answer.pop();
			}
			return;
		}
		else{
			int i = nBegin;
			for (; i < m_nValNum; ++i)
			{
				if (isContainKey(llVal, m_vctKeyVal[i]))
				{
					llVal -= answer.push(m_vctKeyVal[i], i);
					BYTE byNum = getNumByKey(llVal);
					getHuKey(llVal, byThreshXi, answer, i, bFindAll);
					llVal += answer.pop();
				}
			}

			if (answer.getHuXi() <= byThreshXi)
				return;

			BYTE byNum = getNumByKey(llVal);
			if (byNum == 0)
				m_answer[m_nAnswerNum++] = answer;
			else if (byNum == 1)				
				return;
			else if (byNum == 2)
			{
				hash_map<LONG64, BYTE>::iterator iter = g_mapKeyDui.find(llVal);
				if (iter != g_mapKeyDui.end())
				{
					answer.push(llVal, 0xFF);
					m_answer[m_nAnswerNum++] = answer;
					answer.pop();
				}					
			}
			else
			{
				i -= m_nValNum;
				for (; i < m_nNullNum; ++i)
				{
					if (isContainKey(llVal, m_vctKeyNull[i]))
					{
						llVal -= answer.push(m_vctKeyNull[i], i + m_nValNum);
						BYTE byNum = getNumByKey(llVal);
						getHuKey(llVal, byThreshXi, answer, i + m_nValNum, bFindAll);
						llVal += answer.pop();
					}
				}
			}			
		}
	}

	// 此处需过滤重复的答案
	void	getAnswer(vector<stAnswer> &vctOut)
	{
		if (m_nAnswerNum > 0 && m_nAnswerNum < 1024)
		{
			int			answer_num = 0;
			stAnswer	answer_temp[1024];
			hash_map<string, BYTE> mapFlag;			
			char chrTemp[1024] = {};
			for (int i = 0; i < m_nAnswerNum; ++i) {
				stAnswer temp = m_answer[i];
				sort(temp.byIndex, temp.byIndex+7);
				sprintf_s(chrTemp, "%02X%02X%02X%02X%02X%02X%02X", (int)temp.byIndex[0], (int)temp.byIndex[1], (int)temp.byIndex[2],
					(int)temp.byIndex[3], (int)temp.byIndex[4], (int)temp.byIndex[5], (int)temp.byIndex[6]);

				hash_map<string, BYTE>::iterator iter = mapFlag.find(chrTemp);
				if (iter == mapFlag.end()) {
					mapFlag[chrTemp] = 1;
					answer_temp[answer_num++] = m_answer[i];
				}				
			}
			if (answer_num > 0) {
				vctOut.resize(answer_num);
				memcpy(&vctOut[0], answer_temp, answer_num*sizeof(stAnswer));
			}			
		}
	}

private:
	inline void addMapVal(LONG64 llKey, BYTE val)
	{
		BYTE num = getNumByKey(llKey);
		if (g_mapKeyAll[llKey] < val)
			g_mapKeyAll[llKey] = val;
	}

private:
	//hash_map<LONG64, stNodeNull>	g_mapKeyNullAll;		// 22W+个 500M内存（不带将对）否决
	int						m_nAnswerNum;
	stAnswer				m_answer[1024];
	vector<LONG64>			m_vctKeyVal;					// 158
	vector<LONG64>			m_vctKeyNull;					// 72
	hash_map<LONG64, BYTE>	m_mapKeyIndex;					// 72+158
	//g_mapKeyDui											// 41
	int						m_nValNum;
	int						m_nNullNum;
};
