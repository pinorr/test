
#include <iostream>
#include <vector>
#include <windows.h>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <iomanip>

#include "thirteen_logic.h"

using namespace std;

//��������
void printCards(BYTE *byTemp)
{
	//��ӡ������ֵ
	BYTE byData[13][5] = {};
	for (int i = 0; i < 13; ++i) {
		BYTE val = byTemp[i] % 0x10;
		BYTE cor = byTemp[i] / 0x10;
		val -= 2;
		if (val == 255) val = 12;
		++byData[val][4];
		++byData[val][cor];
	}
	for (int i = 0; i < 13; ++i) {
		if (byData[i][4] > 0) {
			for (int n = 0; n < 4; ++n) {
				if (byData[i][n] > 0)
					cout << hex << "0x" << int(GetCard(n, i)) << ", ";
			}
		}
	}
	cout << endl;
}
//��������
bool isSameCards(BYTE byCards[13], stAnswer answer)
{
	BYTE bySrc[13] = {};
	BYTE byTemp[13] = {};
	memcpy(bySrc, byCards, 13);
	answer.getData(byTemp);
	for (int i = 0; i < 13; ++i) {
		bool bFind = false;
		for (int n = 0; n < 13; ++n)
			if (byTemp[n] == byCards[i]) {
				bFind = true;
				break;
			}
		if (bFind == false) 
			return false;
	}
	return true;
}
//��������
bool cmp(const stAnswer &temp1, const stAnswer &temp2)
{
	//static float fVal[3][CT_FIVE_MAX * 16] = {
	//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.06, 0.17, 0.32, 0.64, 1.14, 1.76, 2.50, 3.28, 4.13, 4.95, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 5.47, 8.21, 9.19, 10.18, 11.22, 12.17, 13.16, 14.05, 14.87, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.40, 16.74, 17.82, 19.80, 23.03, 27.69, 33.94, 41.63, 50.63, 50.63, 50.63, 50.63, 54.60, 58.47, 62.34, 66.07, 69.61, 73.04, 76.34, 79.67, 82.90, 86.03, 89.19, 92.14, 95.16, 95.16, 95.16, 95.16, 95.41, 95.67, 95.92, 96.19, 96.42, 96.66, 96.94, 97.18, 97.44, 97.70, 97.93, 98.21, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.49, 98.69, 98.87, 99.03, 99.18, 99.33, 99.51, 99.66, 99.83, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.01, 0.14, 0.91, 3.60, 11.14, 11.14, 11.14, 11.14, 12.57, 14.06, 15.69, 17.41, 19.32, 21.56, 23.95, 26.70, 29.77, 33.30, 37.46, 42.51, 48.39, 48.39, 48.39, 48.39, 48.39, 48.71, 49.34, 50.31, 51.60, 53.22, 55.24, 57.55, 60.44, 63.89, 68.13, 73.20, 79.14, 79.14, 79.14, 79.14, 79.18, 79.27, 79.44, 79.66, 79.93, 80.29, 80.69, 81.15, 81.71, 82.36, 83.10, 83.97, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 84.84, 85.72, 86.47, 87.28, 88.12, 89.00, 89.99, 91.04, 92.23, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.38, 94.39, 94.47, 94.67, 95.04, 95.66, 96.56, 97.74, 99.17, 99.17, 99.17, 99.17, 99.24, 99.32, 99.40, 99.47, 99.54, 99.61, 99.68, 99.75, 99.79, 99.83, 99.89, 99.94, 99.98, 99.98, 99.98, 99.98, 99.99, 99.99, 99.99, 99.99, 99.99, 99.99, 99.99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	//	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.07, 0.24, 0.59, 1.25, 2.41, 4.33, 8.50, 16.88, 31.86, 55.32, 85.92, 85.92, 85.92, 85.92, 86.00, 86.35, 86.89, 87.57, 88.38, 89.19, 90.03, 90.84, 91.76, 93.00, 94.57, 96.81, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.85, 99.86, 99.88, 99.89, 99.90, 99.92, 99.94, 99.95, 99.97, 99.98, 99.99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	//};
	static float fVal[3][CT_FIVE_MAX * 16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.01, 0.02, 0.05, 0.09, 0.17, 0.26, 0.37, 0.49, 0.60, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.68, 0.78, 0.82, 0.88, 0.93, 0.98, 1.03, 1.10, 1.16, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.26, 1.33, 1.50, 1.82, 2.24, 2.78, 3.39, 4.05, 4.76, 4.76, 4.76, 4.76, 11.42, 17.58, 23.23, 28.25, 32.63, 36.36, 39.60, 42.41, 44.81, 46.84, 48.70, 50.24, 51.57, 51.57, 51.57, 51.57, 55.02, 58.68, 62.40, 66.10, 69.66, 73.21, 76.75, 80.10, 83.33, 86.66, 90.06, 93.41, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.49, 96.79, 97.09, 97.46, 97.90, 98.35, 98.79, 99.21, 99.65, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.01, 0.08, 0.34, 1.19, 1.19, 1.19, 1.19, 1.35, 1.56, 1.77, 2.00, 2.27, 2.59, 2.93, 3.30, 3.71, 4.19, 4.73, 5.42, 6.24, 6.24, 6.24, 6.24, 6.24, 6.50, 7.08, 8.02, 9.43, 11.28, 13.58, 16.18, 19.14, 22.18, 25.34, 28.39, 30.97, 30.97, 30.97, 30.97, 31.03, 31.30, 31.82, 32.63, 33.74, 35.18, 36.92, 39.15, 41.50, 44.23, 47.32, 50.62, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 53.86, 54.10, 54.32, 54.60, 54.93, 55.31, 55.81, 56.42, 57.20, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.32, 58.36, 58.46, 58.69, 59.09, 59.83, 60.93, 62.60, 64.88, 64.88, 64.88, 64.88, 67.89, 70.87, 73.79, 76.69, 79.52, 82.20, 84.71, 87.10, 89.29, 91.23, 92.98, 94.47, 95.75, 95.75, 95.75, 95.75, 96.38, 97.00, 97.58, 98.10, 98.55, 98.92, 99.23, 99.48, 99.68, 99.83, 99.92, 99.98, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.01, 0.06, 0.19, 0.51, 1.16, 2.36, 4.55, 8.73, 15.70, 27.06, 45.62, 45.62, 45.62, 45.62, 45.86, 46.30, 46.98, 47.98, 49.31, 51.12, 53.45, 56.39, 60.30, 65.35, 71.65, 79.88, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.65, 90.71, 90.94, 91.36, 91.97, 92.75, 93.72, 94.74, 95.79, 96.79, 97.73, 98.59, 99.37, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	};
	static float fPer[3][CT_FIVE_MAX] = {
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 5, 5, 5,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 8, 10, 10, 10,
		0, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	};
	DWORD t1[3] = { temp1.dwVal[0] >> 22, temp1.dwVal[1] >> 22, temp1.dwVal[2] >> 22 };
	DWORD t2[3] = { temp2.dwVal[0] >> 22, temp2.dwVal[1] >> 22, temp2.dwVal[2] >> 22 };
	DWORD type1[3] = { temp1.dwVal[0] >> 18, temp1.dwVal[1] >> 18, temp1.dwVal[2] >> 18 };
	DWORD type2[3] = { temp2.dwVal[0] >> 18, temp2.dwVal[1] >> 18, temp2.dwVal[2] >> 18 };
	float fFlag1 = 0, fFlag2 = 0;
	for (int i = 0; i < 3; ++i)
	{
		fFlag1 += fPer[i][t1[i]] * fVal[i][type1[i]];
		fFlag2 += fPer[i][t2[i]] * fVal[i][type2[i]];
	}
	return fFlag1 > fFlag2;
}

void main()
{
	srand(GetTickCount());
	vector<stAnswer> vctOut;

	BYTE byTest[13] = { 0x07, 0x27, 0x22, 0x11, 0x21, 0x33, 0x14, 0x0A, 0x1B, 0x2B, 0x05, 0x35, 0x32 };	
	BYTE m_cbCardDataTemp[52] = {};
	random_shuffle(s_cbCardData, s_cbCardData + CARD_COUNT);

	DWORD dwFlag = GetTickCount();
	CGameLogic::JudgeCards(s_cbCardData, 13, vctOut);
	cout << GetTickCount() - dwFlag << " ms" << "\t" << vctOut.size() << endl;	

	vector<stAnswer> vctOut2 = vctOut;
/*	for (size_t i = 0; i < vctOut.size(); ++i)
	{
		size_t j = 0;
		int nFlag = (vctOut[i].dwVal[0] >> 22) * 10000 + (vctOut[i].dwVal[1] >> 22) * 100 + (vctOut[i].dwVal[2] >> 22);
		for (; j < vctOut2.size(); ++j)
		{
			int nFlag2 = (vctOut2[j].dwVal[0] >> 22) * 10000 + (vctOut2[j].dwVal[1] >> 22) * 100 + (vctOut2[j].dwVal[2] >> 22);
			if (nFlag == nFlag2) {
				if (cmp(vctOut[i], vctOut2[j]))
					vctOut2[j] = vctOut[i];
				break;
			}
		}
		if (j >= vctOut2.size())
			vctOut2.push_back(vctOut[i]);
	}	
	//*/
	sort(vctOut2.begin(), vctOut2.end(), cmp);

	//��ӡ����
	for (size_t i = 0; i < vctOut2.size(); ++i) {
		cout << endl;
		stAnswer &answer = vctOut2[i];
		for (int id = 0; id < 3; ++id)
			cout << hex << "type=" << (answer.dwVal[id] >> 22) << "->" << answer.dwVal[id] << '\t' << (int)answer.byCards[id][0] << '\t' << (int)answer.byCards[id][1] <<
			'\t' << (int)answer.byCards[id][2] << '\t' << (int)answer.byCards[id][3] << '\t' << (int)answer.byCards[id][4] << endl;
	}

	Sleep(1000000);
}