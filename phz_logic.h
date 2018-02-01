/********************************************************************
created:	pinorr
file base:	logic.h
author:		pinorr	Q 505971450
purpose:	跑胡子基本算法
*********************************************************************/

#include "cmd_define.h"
#include <hash_map>
#include <algorithm>  
#include <cmath>

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

struct stAnswerOut
{
	BYTE	num;				// 有效牌组数
	BYTE	byNodeXi[7];		// 每组手牌胡息数
	LONG64	llNode[7];			// 原始手牌
	LONG64	llNodeTry[7];		// 变牌后的手牌

	stAnswerOut(){ memset(this, 0, sizeof(*this)); }
	stAnswerOut(const stAnswerOut &temp){ memcpy(this, &temp, sizeof(*this)); }
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

	// 训练（对象初始化）
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

				addNodeList(getKey(0xFF, val, val, val), getKey(val, val, val, val));
				addNodeList(getKey(0xFF, 0xFF, val, val), getKey(val, val, val, val));
				addNodeList(getKey(0xFF, 0xFF, 0xFF, val), getKey(val, val, val, val));
				addNodeList(getKey(0xFF, 0xFF, 0xFF, 0xFF), getKey(val, val, val, val));
				addNodeList(getKey(0xFF, VAL, VAL, VAL), getKey(VAL, VAL, VAL, VAL));
				addNodeList(getKey(0xFF, 0xFF, VAL, VAL), getKey(VAL, VAL, VAL, VAL));
				addNodeList(getKey(0xFF, 0xFF, 0xFF, VAL), getKey(VAL, VAL, VAL, VAL));
				addNodeList(getKey(0xFF, 0xFF, 0xFF, 0xFF), getKey(VAL, VAL, VAL, VAL));
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
				if (isHavePlayType(enPlayType_WANG)) {
					addMapVal(getKey(0xFF, VAL, val, 0), 0);
					addNodeList(getKey(0xFF, VAL, val, 0), getKey(VAL, VAL, val, 0));
					addNodeList(getKey(0xFF, VAL, val, 0), getKey(VAL, val, val, 0));
				}
			}
			if (isHavePlayType(enPlayType_WANG))
			{
				addMapVal(getKey(0xFF,	val,	val,	0), 3);
				addMapVal(getKey(0xFF,	0xFF,	val,	0), 3);
				addMapVal(getKey(0xFF,	VAL,	VAL,	0), 6);
				addMapVal(getKey(0xFF,	0xFF,	VAL,	0), 6);
				addMapVal(getKey(0xFF,	0xFF,	0xFF,	0), 6);

				addNodeList(getKey(0xFF, val,	val,	0), getKey(VAL, val, val, 0));
				addNodeList(getKey(0xFF, val,	val,	0), getKey(val, val, val, 0));
				addNodeList(getKey(0xFF, VAL,	VAL,	0), getKey(VAL, VAL, VAL, 0));
				addNodeList(getKey(0xFF, VAL,	VAL,	0), getKey(val, VAL, VAL, 0));

				addNodeList(getKey(0xFF, 0xFF,	val,	0), getKey(VAL, val, val, 0));
				addNodeList(getKey(0xFF, 0xFF,	val,	0), getKey(VAL, VAL, val, 0));
				addNodeList(getKey(0xFF, 0xFF,	val,	0), getKey(val, val, val, 0));
				addNodeList(getKey(0xFF, 0xFF,	VAL,	0), getKey(VAL, val, VAL, 0));
				addNodeList(getKey(0xFF, 0xFF,	VAL,	0), getKey(VAL, VAL, VAL, 0));
				addNodeList(getKey(0xFF, 0xFF,	VAL,	0), getKey(val, val, VAL, 0));

				addNodeList(getKey(0xFF, 0xFF,	0xFF,	0), getKey(VAL, VAL, VAL, 0));
				addNodeList(getKey(0xFF, 0xFF,	0xFF,	0), getKey(VAL, VAL, val, 0));
				addNodeList(getKey(0xFF, 0xFF,	0xFF,	0), getKey(VAL, val, val, 0));
				addNodeList(getKey(0xFF, 0xFF,	0xFF,	0), getKey(val, val, val, 0));
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

				addNodeList(getKey(0xFF, 0x07, 0x02, 0), getKey(0x0A, 0x07, 0x02, 0));
				addNodeList(getKey(0x0A, 0xFF, 0x02, 0), getKey(0x0A, 0x07, 0x02, 0));
				addNodeList(getKey(0x0A, 0x07, 0xFF, 0), getKey(0x0A, 0x07, 0x02, 0));
				addNodeList(getKey(0xFF, 0xFF, 0x02, 0), getKey(0x0A, 0x07, 0x02, 0));
				addNodeList(getKey(0x0A, 0xFF, 0xFF, 0), getKey(0x0A, 0x07, 0x02, 0));
				addNodeList(getKey(0xFF, 0x07, 0xFF, 0), getKey(0x0A, 0x07, 0x02, 0));
				addNodeList(getKey(0xFF, 0xFF, 0xFF, 0), getKey(0x0A, 0x07, 0x02, 0));

				addNodeList(getKey(0xFF, 0x17, 0x12, 0), getKey(0x1A, 0x17, 0x12, 0));
				addNodeList(getKey(0x1A, 0xFF, 0x12, 0), getKey(0x1A, 0x17, 0x12, 0));
				addNodeList(getKey(0x1A, 0x17, 0xFF, 0), getKey(0x1A, 0x17, 0x12, 0));
				addNodeList(getKey(0xFF, 0xFF, 0x12, 0), getKey(0x1A, 0x17, 0x12, 0));
				addNodeList(getKey(0x1A, 0xFF, 0xFF, 0), getKey(0x1A, 0x17, 0x12, 0));
				addNodeList(getKey(0xFF, 0x17, 0xFF, 0), getKey(0x1A, 0x17, 0x12, 0));
				addNodeList(getKey(0xFF, 0xFF, 0xFF, 0), getKey(0x1A, 0x17, 0x12, 0));
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

				addNodeList(getKey(0xFF,	val + 1,	val + 2,	0), getKey(val,		val + 1,	val + 2, 0));
				addNodeList(getKey(val,		0xFF,		val + 2,	0), getKey(val,		val + 1,	val + 2, 0));
				addNodeList(getKey(val,		val + 1,	0xFF,		0), getKey(val,		val + 1,	val + 2, 0));
				addNodeList(getKey(val,		0xFF,		0xFF,		0), getKey(val,		val + 1,	val + 2, 0));
				addNodeList(getKey(0xFF,	val + 1,	0xFF,		0), getKey(val,		val + 1,	val + 2, 0));
				addNodeList(getKey(0xFF,	0xFF,		val + 2,	0), getKey(val,		val + 1,	val + 2, 0));
				addNodeList(getKey(0xFF,	0xFF,		0xFF,		0), getKey(val,		val + 1,	val + 2, 0));

				addNodeList(getKey(0xFF,	VAL + 1,	VAL + 2,	0), getKey(VAL,		VAL + 1,	VAL + 2, 0));
				addNodeList(getKey(VAL,		0xFF,		VAL + 2,	0), getKey(VAL,		VAL + 1,	VAL + 2, 0));
				addNodeList(getKey(VAL,		VAL + 1,	0xFF,		0), getKey(VAL,		VAL + 1,	VAL + 2, 0));
				addNodeList(getKey(VAL,		0xFF,		0xFF,		0), getKey(VAL,		VAL + 1,	VAL + 2, 0));
				addNodeList(getKey(0xFF,	VAL + 1,	0xFF,		0), getKey(VAL,		VAL + 1,	VAL + 2, 0));
				addNodeList(getKey(0xFF,	0xFF,		VAL + 2,	0), getKey(VAL,		VAL + 1,	VAL + 2, 0));				
				addNodeList(getKey(0xFF,	0xFF,		0xFF,		0), getKey(VAL,		VAL + 1,	VAL + 2, 0));

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

				addNodeList(getKey(val, 0xFF, 0, 0), getKey(val, val, 0, 0));
				addNodeList(getKey(VAL, 0xFF, 0, 0), getKey(VAL, VAL, 0, 0));
				addNodeList(getKey(0xFF, 0xFF, 0, 0), getKey(val, val, 0, 0));
				addNodeList(getKey(0xFF, 0xFF, 0, 0), getKey(VAL, VAL, 0, 0));
			}
		}
		m_nValNum = m_vctKeyVal.size();
		m_nNullNum = m_vctKeyNull.size();
		cout << g_mapKeyAll.size() << endl;
		cout << m_vctKeyNull.size() << endl;
		cout << m_vctKeyVal.size() << endl;
	}

	// cbCards: 手牌列表指针
	// num:		手牌数量
	// byThreshXi: 胡息阀值（最小胡息）
	// bFindAll：是否找出所有解
	int		getHuKeyInit(BYTE *cbCards, BYTE num, BYTE byThreshXi, bool bFindAll = false)
	{
		m_byThreshXi = byThreshXi;
		LONG64 llVal = 0;
		for (int i = 0; i < num; ++i)
		{
			BYTE index = getIndexByVal(cbCards[i]);
			if (index >= MAX_TYPE) return false;
			llVal += (LONG64)1 << (3 * index);
		}

		if (getCardsNum(llVal, 20) > 4)
			return 0;

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

	// 胡牌逻辑接口
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

	// 获取胡牌结果
	// 此处需过滤重复的答案
	void	getAnswer(vector<stAnswerOut> &vctOut)
	{
		if (m_nAnswerNum > 0 && m_nAnswerNum < 1024)
		{
			int			answer_num = 0;
			stAnswer	answer_temp[1024];
			hash_map<string, BYTE> mapFlag;
			char chrTemp[1024] = {};
			for (int i = 0; i < m_nAnswerNum; ++i) {
				stAnswer temp = m_answer[i];
				sort(temp.byIndex, temp.byIndex + 7);
				sprintf_s(chrTemp, "%02X%02X%02X%02X%02X%02X%02X", (int)temp.byIndex[0], (int)temp.byIndex[1], (int)temp.byIndex[2],
					(int)temp.byIndex[3], (int)temp.byIndex[4], (int)temp.byIndex[5], (int)temp.byIndex[6]);

				hash_map<string, BYTE>::iterator iter = mapFlag.find(chrTemp);
				if (iter == mapFlag.end()) {
					mapFlag[chrTemp] = 1;
					answer_temp[answer_num++] = m_answer[i];
				}
			}
			if (answer_num > 0) {
				int num_out = 0;
				stAnswerOut	answer_out[2048];
				for (int i = 0; i < answer_num; ++i)
				{
					int nTryLen = 0;
					vector<LONG64> vctTry[7];
					BYTE byTryLen[7] = {};
					stAnswer &stTemp = answer_temp[i];
					stAnswerOut stTempOut;
					stTempOut.num = stTemp.num;
					for (int n = 0; n < stTemp.num; ++n)
					{
						stTempOut.byNodeXi[n] = stTemp.byNodeXi[n];
						stTempOut.llNode[n] = stTemp.llNode[n];
						hash_map<LONG64, vector<LONG64> >::iterator iter = m_mapNodeList.find(stTemp.llNode[n]);
						if (iter != m_mapNodeList.end()) {
							if (iter->second.size() <= 1)
								stTempOut.llNodeTry[n] = iter->second[0];
							else {
								vctTry[n] = iter->second;
								byTryLen[n] = BYTE(log(vctTry[n].size()) / log(2) + 0.99999999);
								nTryLen += byTryLen[n];
							}
						}
						else {
							stTempOut.llNodeTry[n] = stTemp.llNode[n];
						}
					}
					int nAll = (int)pow(2, nTryLen);
					for (int j = 0; j < nAll; ++j)
					{
						int index = j;
						bool bError = false;
						for (int n = 0; n < stTemp.num; ++n)
						{
							if (vctTry[n].empty()) continue;
							int ii = index & ((1 << byTryLen[n])-1);
							if (ii >= (int)vctTry[n].size()) {
								bError = true;
								break;
							}
							stTempOut.llNodeTry[n] = vctTry[n][ii];
							hash_map<LONG64, BYTE>::iterator iterTry = g_mapKeyAll.find(vctTry[n][ii]);
							if (iterTry != g_mapKeyAll.end()) {
								stTempOut.byNodeXi[n] = iterTry->second;
							}
							
							index = index >> byTryLen[n];
						}
						if (!bError && stTempOut.getHuXi() >= m_byThreshXi)
							answer_out[num_out++] = stTempOut;
						if (num_out >= 1024) break;
					}					
				}				
				vctOut.resize(num_out);
				memcpy(&vctOut[0], answer_out, num_out*sizeof(stAnswerOut));
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
	inline void addNodeList(LONG64 llKeyX, LONG64 llKeyReal)
	{
		hash_map<LONG64, vector<LONG64> >::iterator iter = m_mapNodeList.find(llKeyX);
		if (iter != m_mapNodeList.end())
			iter->second.push_back(llKeyReal);
		else
			m_mapNodeList[llKeyX].push_back(llKeyReal);
	}

private:
	//hash_map<LONG64, stNodeNull>	g_mapKeyNullAll;		// 22W+个 500M内存（不带将对）否决
	int						m_nAnswerNum;
	stAnswer				m_answer[1024];					// 存放结果的数组
	vector<LONG64>			m_vctKeyVal;					// (有胡息的结果集)158
	vector<LONG64>			m_vctKeyNull;					// (无胡息的结果集)72
	hash_map<LONG64, BYTE>	m_mapKeyIndex;					// 72+158
	hash_map<LONG64, vector<LONG64> >	m_mapNodeList;		// 带有赖子的牌组集
	//g_mapKeyDui											// 41
	int						m_nValNum;						// 有胡息结果集数
	int						m_nNullNum;						// 无胡息结果集数
	BYTE					m_byThreshXi;					// 胡牌阀值
};
