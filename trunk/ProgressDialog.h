#pragma once

class CProgressDialog : public CDialogImpl<CProgressDialog>
{
public:
    enum { IDD = IDD_PROGRESS_DIALOG };
 
    BEGIN_MSG_MAP(CProgressDialog)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    END_MSG_MAP()
 
	CProgressDialog(IplImage*, CvCapture*, CvVideoWriter*);
	~CProgressDialog();
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	static DWORD WINAPI ThreadCallback(CProgressDialog*);
	void ConvertFrames();

	int frameNum;

private:
	HANDLE hMutex;
	HANDLE m_hThread;
	DWORD threadID;
	int videoX, videoY;
	IplImage *frame, *copyFrame;
	Bitmap *bmpVideo;
	CvCapture *capture;
	CvVideoWriter *writer;
    Rect videoBounds;
	RECT videoRect;
};
