#include "StdAfx.h"
#include "GameVideoItemSink.h"

#define  ADD_VIDEO_BUF  4096

CGameVideoItemSink::CGameVideoItemSink(void)
{
	//设置变量
	m_iCurPos	= 0;	
	m_iBufferSize	= 0;
	m_pVideoDataBuffer = NULL;
	m_pITableFrame = NULL;
	m_bFangKa = true;
}

CGameVideoItemSink::~CGameVideoItemSink( void )
{

}

bool __cdecl CGameVideoItemSink::StartVideo(ITableFrame	*pTableFrame, bool bFangKa/*true*/)
{
	try
	{
		ResetVideoItem();

		m_bFangKa = bFangKa;
		if (!m_bFangKa) return false;

		m_pITableFrame = pTableFrame;

		m_iCurPos	= 0;
		m_iBufferSize  = ADD_VIDEO_BUF;

		//申请内存
		m_pVideoDataBuffer = new BYTE [m_iBufferSize];
		if (m_pVideoDataBuffer==NULL) return false;	
		memset(m_pVideoDataBuffer,0,m_iBufferSize);
	}
	catch (...) 
	{
		ASSERT (FALSE);
		return false; 
	}
	
	return true;
}

bool __cdecl CGameVideoItemSink::StopAndSaveVideo(WORD wServerID,WORD wTableID)
{
	if (!m_bFangKa) return false;
	//保存到数据库
	if(m_pITableFrame == NULL) return false;
	//保存格式：字符串+唯一标识ID (ID命名规则：年月日时分秒+房间ID+桌子ID) 	
	CHAR   szVideoNumber[22];
	szVideoNumber[0] = 0;
	BuildVideoNumber(szVideoNumber,22,wServerID,wTableID);
	
	bool bAllAndroid = true;
	for (WORD i=0; i<MAX_USER_NUM; i++)
	{
		IServerUserItem *pServerUserItem = m_pITableFrame->GetTableUserItem(i);
		if (pServerUserItem && pServerUserItem->IsAndroidUser() == false)
		{
			bAllAndroid = false;
		}
	}
	if(bAllAndroid) return TRUE;

	for (WORD i=0; i<MAX_USER_NUM; i++)
	{
		IServerUserItem *pServerUserItem = m_pITableFrame->GetTableUserItem(i);
		if (pServerUserItem)
		{
			m_pITableFrame->WriteTableVideoPlayer(pServerUserItem->GetUserID(),szVideoNumber);
		}
	}
	m_pITableFrame->WriteTableVideoData(szVideoNumber,wServerID,wTableID,m_pVideoDataBuffer,(WORD)m_iCurPos);	

	ResetVideoItem();
	return TRUE;
}

bool __cdecl CGameVideoItemSink::AddVideoData(WORD wMsgKind, void *pPack, int nSize, bool bAddID/*true*/)
{
	if (!m_bFangKa) return false;
	//一定要按顺序写
	if (bAddID)
		Write(&wMsgKind, sizeof(WORD));

	Write(pPack, sizeof(BYTE)*nSize);
	return TRUE;
}

size_t	CGameVideoItemSink::Write(const void* data, size_t size)
{   
	if(size + m_iCurPos > m_iBufferSize)
	{
		if(RectifyBuffer(ADD_VIDEO_BUF/2)!=TRUE) return 0;
	}
	
	CopyMemory(m_pVideoDataBuffer+m_iCurPos,data,size);				
	m_iCurPos += size;

	return size;
}

//调整缓冲
bool CGameVideoItemSink::RectifyBuffer(size_t iSize)
{
	try
	{		
		size_t iNewbufSize =  iSize+m_iBufferSize;
		//申请内存
		BYTE * pNewVideoBuffer=new BYTE [iNewbufSize];
		if (pNewVideoBuffer==NULL) return false;
		memset(pNewVideoBuffer,0,iNewbufSize);

		//拷贝数据
		if (m_pVideoDataBuffer!=NULL) 
		{
			CopyMemory(pNewVideoBuffer,m_pVideoDataBuffer,m_iCurPos);				
		}

		//设置变量			
		m_iBufferSize=iNewbufSize;
		SafeDeleteArray(m_pVideoDataBuffer);
		m_pVideoDataBuffer=pNewVideoBuffer;
	}
	catch (...) 
	{
		ASSERT (FALSE);
		return false; 
	}

	return true;
}

//重置数据
void CGameVideoItemSink::ResetVideoItem()
{
	//设置变量
	m_iCurPos	= 0;	
	m_iBufferSize  = 0;
	SafeDeleteArray(m_pVideoDataBuffer);
}

//录像编号
VOID CGameVideoItemSink::BuildVideoNumber(CHAR szVideoNumber[], WORD wLen,WORD wServerID,WORD wTableID)
{
	//获取时间
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime); 

	//格式化编号
	_snprintf(szVideoNumber,wLen,"%04d%02d%02d%02d%02d%02d%03d%03d",SystemTime.wYear,SystemTime.wMonth,SystemTime.wDay,
		SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond,wServerID,wTableID);
}
