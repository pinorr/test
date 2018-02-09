

#pragma once

template <typename T>
void swap_pino(T &a, T &b)
{
	static T temp;
	temp = a;
	a = b;
	b = temp;
}

template <typename T>
void sort_fast(T *arr, int iBegin, int iEnd)
{
	if (iEnd <= iBegin)	return;
	int nRand = rand()%(iEnd-iBegin);
	if (nRand != 0)
		swap_pino(arr[nRand+iBegin], arr[iBegin]);

	int i = iBegin + 1;
	int j = iEnd;
	while (i < j)
	{
		while (i < j && arr[i] <= arr[iBegin])	++i;
		while (i < j && arr[j] >= arr[iBegin])	--j;
		if (i < j)	swap_pino(arr[i], arr[j]);
	}
	int mid = arr[iBegin] < arr[j] ? j-1 : j;
	swap_pino(arr[iBegin], arr[mid]);
	sort_fast(arr, iBegin, mid-1);
	sort_fast(arr, mid+1, iEnd);
}

template <typename T>
T sort_fast_find_n(T *arr, int n, int iBegin, int iEnd)
{
	if (iEnd <= iBegin)	return arr[iBegin];
	int nRand = rand()%(iEnd-iBegin);
	if (nRand != 0)
		swap_pino(arr[nRand+iBegin], arr[iBegin]);

	int i = iBegin + 1;
	int j = iEnd;
	while (i < j)
	{
		while (i < j && arr[i] <= arr[iBegin])	++i;
		while (i < j && arr[j] >= arr[iBegin])	--j;
		if (i < j)	swap_pino(arr[i], arr[j]);
	}
	int mid = arr[iBegin] < arr[j] ? j-1 : j;
	if (mid == n)
		return arr[iBegin];
	else if(mid > n)
	{
		swap_pino(arr[iBegin], arr[mid]);
		return sort_fast_find_n(arr, n, iBegin, mid-1);
	}
	else
	{
		swap_pino(arr[iBegin], arr[mid]);
		return sort_fast_find_n(arr, n, mid+1, iEnd);
	}

// 	swap_pino(arr[iBegin], arr[mid]);
// 	sort_fast(arr, iBegin, mid-1);
// 	sort_fast(arr, mid+1, iEnd);
}


template <typename T>
void adjust_min_heap(T *arr, int i, int len)
{
	int child;
	while (i < len)
	{
		child = i*2 + 1;
		if (child + 1 < len && arr[child] > arr[child+1]) ++child;
		if (child < len && arr[child] < arr[i])	swap_pino(arr[child], arr[i]);
		i = child;
	}
}

template <typename T>
void adjust_max_heap(T *arr, int i, int len)
{
	int child;
	while (i < len)
	{
		child = i*2 + 1;
		if (child + 1 < len && arr[child] < arr[child+1]) ++child;
		if (child < len && arr[child] > arr[i])	swap_pino(arr[child], arr[i]);
		i = child;
	}
}

template <typename T>
void sort_up_heap(T *arr, int len)					//升序
{
	//建立基本堆，为何是len/2是因为这值是达到同一效果的最小值(len也是一样效果)
	for (int i=len/2-1; i>=0; --i)
		adjust_max_heap(arr, i, len);

	//将每次最大值的放到最后面（最大堆）
	for (int i=len-1; i>0; --i)
	{
		swap_pino(arr[i], arr[0]);
		adjust_max_heap(arr, 0, i);
	}
}

template <typename T>
void sort_down_heap(T *arr, int len)				//降序
{
	for (int i=len/2-1; i>=0; --i)
		adjust_min_heap(arr, i, len);

	//将每次最大值的放到最后面（最大堆）
	for (int i=len-1; i>0; --i)
	{
		swap_pino(arr[i], arr[0]);
		adjust_min_heap(arr, 0, i);
	}
}

template <typename T>
void sort_min_n_heap(T *arr, int n, int len)		//得到最小的n个值 放到数组最后
{
	for (int i=len/2-1; i>=0; --i)
		adjust_min_heap(arr, i, len);

	//将每次最大值的放到最后面（最大堆）
	for (int i=len-1; i>=len - n; --i)
	{
		swap_pino(arr[i], arr[0]);
		adjust_min_heap(arr, 0, i);
	}
}

template<typename T>
void merge_arry(T *arr, int iBegin, int iMid, int iEnd)
{
	T *arrTemp = new T[iEnd - iBegin + 1];
	int i = 0;
	int l = iBegin;
	int r = iMid + 1;
	while (i <= iEnd - iBegin && l <= iMid && r <= iEnd)
	{
		if (arr[l] < arr[r])
			arrTemp[i++] = arr[l++];
		else
			arrTemp[i++] = arr[r++];
	}
	if (l <= iMid)
	{
		memcpy(arrTemp + i, arr+l, sizeof(T)*(iMid - l + 1));
		i += iMid - l + 1;
	}
	if (r <= iEnd)
	{
		memcpy(arrTemp + i, arr+r, sizeof(T)*(iEnd - r + 1));
		i += iEnd - r + 1;
	}
	memcpy(arr + iBegin, arrTemp, sizeof(T) * (iEnd - iBegin + 1));
	delete []arrTemp;
}

template<typename T>
void sort_merge(T *arr, int iBegin, int iEnd)
{	
	int iMid = (iBegin + iEnd)/2;
	if (iBegin < iMid)
		sort_merge(arr, iBegin, iMid);
	if (iMid+1 < iEnd)
		sort_merge(arr, iMid+1, iEnd);
	merge_arry(arr, iBegin, iMid, iEnd);
}
