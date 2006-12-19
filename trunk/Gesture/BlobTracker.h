#pragma once

#include "TrajectoryList.h"
#include "../resource.h"

class BlobTracker;

class CBlobTrackerDialog : public CDialogImpl<CBlobTrackerDialog> {
public:
	CBlobTrackerDialog(BlobTracker*);
	~CBlobTrackerDialog();

    enum { IDD = IDD_BLOBTRACKER_DIALOG };
    BEGIN_MSG_MAP(CBlobTrackerDialog)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void ConvertFrames();

private:
	CRect videoRect;
	Rect drawRect;
	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
	static DWORD WINAPI ThreadCallback(CBlobTrackerDialog*);
	BlobTracker *parent;
};


class BlobTracker
{
public:
    BlobTracker();
    ~BlobTracker(void);
    void LearnTrajectories(CvCapture* pCap);
    void GetTrajectoriesInRange(vector<MotionTrack> *trackList, long startFrame, long endFrame);
    void GetTrajectoriesAtFrame(vector<MotionTrack> *trackList, long frameNum);
    void ProcessFrame();

    bool isTrained;

protected:
    IplImage *currentFrame, *copyFrame, *maskedCopyFrame, *fgImageCopy;
    Bitmap *bmpVideo;
    CvCapture *videoCapture;

private:

    CvBlobTrackerAutoParam1 param;
    CvBlobTrackerAuto *pTracker;

    TrajectoryList trajectories;

	friend class CBlobTrackerDialog;
	CBlobTrackerDialog m_BlobTrackerDialog;
};
