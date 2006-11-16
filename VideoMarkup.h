#pragma once

#include "precomp.h"

typedef CWinTraits<WS_CHILD|WS_VISIBLE,0> CVideoMarkupTraits;

class CVideoMarkup: public CWindowImpl<CVideoMarkup, CWindow, CVideoMarkupTraits>
{
public:
    CContainedWindow m_slider, m_trainButton, m_showButton, m_sampleListView;
    HIMAGELIST m_hImageList;

    CVideoMarkup();
    ~CVideoMarkup();
    LRESULT OnPaint( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnButtonDown( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnTrack( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnMouseMove( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnButtonUp( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnDestroy( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnCommand(UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnBeginDrag(int, LPNMHDR, BOOL&);
    LRESULT OnCustomDraw(int, LPNMHDR, BOOL&);

    void OpenVideoFile();
    void OpenVideoFile(char *filename);
    void ShowFrame(long);
    void EnableControls(BOOL);

    static CWndClassInfo& GetWndClassInfo()
    {
        static CWndClassInfo wc =
        {
            { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, 
            StartWindowProc,
            0, 0, NULL, NULL, NULL, (HBRUSH)(WHITE_BRUSH), NULL, 
            FILTER_CREATE_CLASS, LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(ID_VIDMARKUP)) },
            NULL, NULL, IDC_CROSS, TRUE, 0, FILTER_CREATE_CLASS
        };
        return wc;
    }

    BEGIN_MSG_MAP(CVideoMarkup)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnButtonDown)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_HSCROLL, OnTrack)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnButtonUp)
        MESSAGE_HANDLER(WM_RBUTTONUP, OnButtonUp)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        NOTIFY_CODE_HANDLER(LVN_BEGINDRAG, OnBeginDrag)
        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
    ALT_MSG_MAP(1)  // video slider
        MESSAGE_HANDLER(TB_THUMBTRACK, OnTrack)
    ALT_MSG_MAP(2)  // sample list
    ALT_MSG_MAP(3)  // train button
    ALT_MSG_MAP(4)  // show button
    END_MSG_MAP()

private:
	HDC hdcmem;
	HBITMAP hbm;
    HCURSOR hTrashCursor, hDropCursor;

	Graphics *graphics;
    GraphicsPath *penPath;
    PointF selectStart, selectCurrent;
    Pen *posPen, *negPen, *posSelectPen, *negSelectPen;
    SolidBrush *posBrush, *negBrush, *hoverBrush;
    bool pathComplete;
    int currentGroupId;
    CRect m_videoRect;

    int WindowX, WindowY;
    long nFrames;
    int videoX, videoY;

    CvCapture* videoCapture;
    IplImage *currentFrame, *copyFrame;
    bool videoLoaded;
    Bitmap *bmpVideo;

    TrainingSet *sampleSet;
    HaarClassifier *classifier;

    // drag and drop stuff
    HIMAGELIST hGroupHeaderImages;
    HIMAGELIST hDragImageList;
    bool dragHover;
    Rect hoverRect;
    bool draggingIcon;

    // list of detected objects
    list <Rect> objGuesses;
    bool showGuesses;
};
