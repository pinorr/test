#pragma once

class CGameVideoItemSink
{
public:
	CGameVideoItemSink(void);
	virtual ~CGameVideoItemSink(void);

public:		
	//开始录像
	virtual bool __cdecl	StartVideo(ITableFrame	*pTableFrame, bool bFangKa=true);
	//停止和保存
	virtual bool __cdecl	StopAndSaveVideo(WORD wServerID,WORD wTableID);
	//增加录像数据
	virtual bool __cdecl    AddVideoData(WORD wMsgKind, void *pPack,int nSize, bool bAddID = true);
protected:
	void					ResetVideoItem();
	bool					RectifyBuffer(size_t iSize);	
	VOID					BuildVideoNumber(CHAR szVideoNumber[], WORD wLen,WORD wServerID,WORD wTableID);

	size_t					Write(const void* data, size_t size);	

	//数据变量
private:
	ITableFrame	*					m_pITableFrame;						//框架接口
	size_t							m_iCurPos;							//数据位置	
	size_t							m_iBufferSize;						//缓冲长度
	LPBYTE							m_pVideoDataBuffer;					//缓冲指针
	bool							m_bFangKa;
};
