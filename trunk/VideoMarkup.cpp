#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "HaarClassifier.h"
#include "VideoMarkup.h"

void AddListViewGroup(HWND hwndList, WCHAR *szText, int iGroupId) {
    LVGROUP lvgrp = { sizeof(lvgrp) };
    lvgrp.state = LVGS_NORMAL;
    lvgrp.mask      = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
    lvgrp.pszHeader = szText;
    lvgrp.cchHeader = (int)wcslen(lvgrp.pszHeader);
    lvgrp.iGroupId  = iGroupId;
    lvgrp.uAlign    = LVGA_HEADER_CENTER;
    ListView_InsertGroup(hwndList, iGroupId, &lvgrp);
}

CVideoMarkup::CVideoMarkup() :
    m_slider(TRACKBAR_CLASS, this, 1),
    m_sampleListView(WC_LISTVIEW, this, 2),
    m_trainButton(WC_BUTTON, this, 3),
    m_showButton(WC_BUTTON, this, 4),
    m_videoRect(0,0,VIDEO_X,VIDEO_Y) {

    // TODO: all non window-related variables should be initialized here instead of in OnCreate
    videoLoaded = false;
    showGuesses = false;
    videoCapture = NULL;
    copyFrame = NULL;
    bmpVideo = NULL;

    sampleSet = new TrainingSet();
    classifier = new HaarClassifier();
}


CVideoMarkup::~CVideoMarkup() {
    delete classifier;
    if (videoLoaded) {
        cvReleaseCapture(&videoCapture);
        cvReleaseImage(&copyFrame);
    }
}


void CVideoMarkup::OpenVideoFile() {

    if (videoLoaded) { // already loaded a video so this will be a new one
        cvReleaseCapture(&videoCapture);
        cvReleaseImage(&copyFrame);
        delete bmpVideo;
        videoLoaded = false;
        // TODO: clear out example gallery... ask if user wants to save classifier?
    }

    USES_CONVERSION;
    OPENFILENAMEW ofn;
    WCHAR szFileName[MAX_PATH] = L"";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"AVI Files (*.avi)\0*.avi\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"avi";

    if(GetOpenFileName(&ofn))
    {
        // Load the video file and get dimensions
        videoCapture = cvCreateFileCapture(W2A(szFileName));

        if (videoCapture != NULL) {
            videoLoaded = TRUE;
            videoX = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_WIDTH);
            videoY = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_HEIGHT);
            nFrames = cvGetCaptureProperty(videoCapture,  CV_CAP_PROP_FRAME_COUNT);

            // create an image to store a copy of the current frame
            copyFrame = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,3);

            // Create a bitmap to display video
            bmpVideo = new Bitmap(videoX, videoY, PixelFormat24bppRGB);

            EnableControls(TRUE);
            SendMessage(m_slider, TBM_SETRANGEMIN, FALSE, 0);
            SendMessage(m_slider, TBM_SETRANGEMAX, FALSE, nFrames-1);
            SendMessage(m_slider, TBM_SETPOS, FALSE, 0);
            ShowFrame(0);
            InvalidateRect(m_videoRect,FALSE);
        }
    }
}


void CVideoMarkup::RecordVideoFile() {
    if (videoLoaded) { // already loaded a video so this will be a new one
        cvReleaseCapture(&videoCapture);
        cvReleaseImage(&copyFrame);
        delete bmpVideo;
        videoLoaded = false;
        // TODO: clear out example gallery... ask if user wants to save classifier?
    }

    // TODO: set video props, select a camera, handle case where there is no camera
    videoCapture = cvCreateCameraCapture(0);
    
    // TODO: write frames to file and make a stop button
    if (videoCapture != NULL) {
        videoLoaded = TRUE;
        videoX = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_WIDTH);
        videoY = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_HEIGHT);
        nFrames = cvGetCaptureProperty(videoCapture,  CV_CAP_PROP_FRAME_COUNT);

        // create an image to store a copy of the current frame
        copyFrame = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,3);

        // Create a bitmap to display video
        bmpVideo = new Bitmap(videoX, videoY, PixelFormat24bppRGB);

        // TODO: disable menu until recording completes, show video while recording, stretch to display
    }
    // then if recording completes successfully (put this in separate function
    //{
    //    EnableControls(TRUE);
    //    SendMessage(m_slider, TBM_SETRANGEMIN, FALSE, 0);
    //    SendMessage(m_slider, TBM_SETRANGEMAX, FALSE, nFrames-1);
    //    SendMessage(m_slider, TBM_SETPOS, FALSE, 0);
    //    ShowFrame(0);
    //    InvalidateRect(m_videoRect,FALSE);
    //}
}

void CVideoMarkup::EnableControls(BOOL enabled) {
    m_trainButton.EnableWindow(enabled);
    m_showButton.EnableWindow(enabled);
    m_slider.EnableWindow(enabled);
    m_sampleListView.EnableWindow(enabled);
}

LRESULT CVideoMarkup::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
{
	PAINTSTRUCT ps;

    HDC hdc = BeginPaint(&ps);
    Rect videoBounds(0,0,VIDEO_X,VIDEO_Y);
    graphics->SetClip(videoBounds);
    
    if (videoLoaded) {
        if (bmpVideo != NULL) graphics->DrawImage(bmpVideo,0,0);

        if (showGuesses) { // highlight computer's guesses
            for (list<Rect>::iterator i = objGuesses.begin(); i != objGuesses.end(); i++) {
                graphics->DrawRectangle(posPen,*i);
            }
        }

        // to draw path:
//      graphics->DrawPath(posPen,penPath);
//      graphics->FillPath(posBrush,penPath);
        Rect selectRect;
        selectRect.X = min(selectStart.X, selectCurrent.X);
        selectRect.Y = min(selectStart.Y, selectCurrent.Y);
        selectRect.Width = abs(selectStart.X - selectCurrent.X);
        selectRect.Height = abs(selectStart.Y - selectCurrent.Y);

        if (!pathComplete) {
            if (currentGroupId == 0) {
                graphics->FillRectangle(posBrush, selectRect);
                graphics->DrawRectangle(posSelectPen, selectRect);
            } else {
                graphics->FillRectangle(negBrush, selectRect);
                graphics->DrawRectangle(negSelectPen, selectRect);
            }
        }
        graphics->ResetClip();
    }

    BitBlt(hdc,0,0,WINDOW_X,WINDOW_Y,hdcmem,0,0,SRCCOPY);
    EndPaint(&ps);

    return 0;
}

LRESULT CVideoMarkup::OnButtonDown( UINT, WPARAM wParam, LPARAM lParam, BOOL& )
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

    if (!videoLoaded) return 0;
    if (!m_videoRect.PtInRect(p)) return 0;

    penPath->Reset();
    pathComplete = false;

    // If the right button is down, we consider this a negative sample
    // TODO: do this some better way
    if (MK_RBUTTON & wParam) currentGroupId = 1;
    else currentGroupId = 0;

    selectStart.X = (REAL) p.x;
    selectStart.Y = (REAL) p.y;
    selectCurrent = selectStart;

    // restrict mouse movement to within the video window
    CRect videoRect = m_videoRect;
    ClientToScreen(&videoRect);
    ClipCursor(videoRect);

    return 0;
}

LRESULT CVideoMarkup::OnMouseMove( UINT, WPARAM wParam, LPARAM lParam, BOOL& )
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

    if (!videoLoaded) return 0;

    if ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)) { // mouse is down
        if (draggingIcon) { // we are dragging in listview

            // Determine which group the cursor is over
            LVHITTESTINFO lvhti;
            lvhti.pt = p;
            ClientToScreen(&lvhti.pt);
            ::ScreenToClient(m_sampleListView, &lvhti.pt);
            ListView_HitTestEx(m_sampleListView, &lvhti);
            CRect posRect, negRect;
            ListView_GetGroupRect(m_sampleListView, 0, LVGGR_GROUP, &posRect);
            ListView_GetGroupRect(m_sampleListView, 1, LVGGR_GROUP, &negRect);
            if (posRect.PtInRect(lvhti.pt)) { // highlight positive group
                SetCursor(hDropCursor);
                dragHover = true;
                hoverRect.X = posRect.left; hoverRect.Y = posRect.top;
                hoverRect.Width = posRect.Width();  hoverRect.Height = posRect.Height();
            } else if (negRect.PtInRect(lvhti.pt)) { // highlight negative group
                SetCursor(hDropCursor);
                dragHover = true;
                hoverRect.X = negRect.left; hoverRect.Y = negRect.top;
                hoverRect.Width = negRect.Width();  hoverRect.Height = negRect.Height();
            } else {
                SetCursor(hTrashCursor);
                dragHover = false;
            }

            // update listview to highlight group on hover
            m_sampleListView.RedrawWindow();

            // draw drag icon
            ClientToScreen(&p);
            ImageList_DragMove(p.x, p.y);

         } else { // we are drawing highlights
            if (!m_videoRect.PtInRect(p)) return 0;

            penPath->AddLine(selectCurrent, PointF(p.x, p.y));
            selectCurrent.X = (REAL) p.x;
            selectCurrent.Y = (REAL) p.y;
           
            InvalidateRect(&m_videoRect, FALSE);
        }
    }
	return 0;
}

LRESULT CVideoMarkup::OnButtonUp( UINT, WPARAM, LPARAM lParam, BOOL&)
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

    if (!videoLoaded) return 0;

    if (draggingIcon) { // we just completed an icon drag
        // End the drag-and-drop process
        draggingIcon = FALSE;
        ImageList_DragLeave(m_sampleListView);
        ImageList_EndDrag();
        ImageList_Destroy(hDragImageList);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        ReleaseCapture();

        // Determine the position of the drop point
        LVHITTESTINFO lvhti;
        lvhti.pt.x = p.x;
        lvhti.pt.y = p.y;
        ClientToScreen(&lvhti.pt);
        ::ScreenToClient(m_sampleListView, &lvhti.pt);
        ListView_HitTestEx(m_sampleListView, &lvhti);
        CRect posRect, negRect;
        ListView_GetGroupRect(m_sampleListView, 0, LVGGR_GROUP, &posRect);
        ListView_GetGroupRect(m_sampleListView, 1, LVGGR_GROUP, &negRect);

        int newGroupId;
        if (posRect.PtInRect(lvhti.pt)) newGroupId = 0;
        else if (negRect.PtInRect(lvhti.pt)) newGroupId = 1;
        else newGroupId = 2;

        // update group membership of selected items based on drop location
        int numSelected = ListView_GetSelectedCount(m_sampleListView);
        int iSelection = -1;
        for (int iIndex=0; iIndex<numSelected; iIndex++) {
            LVITEM lvi;
            iSelection = ListView_GetNextItem(m_sampleListView, iSelection, LVNI_SELECTED);
            lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_GROUPID;
            lvi.state = 0;
            lvi.stateMask = 0;
            lvi.iItem = iSelection;
            lvi.iSubItem = 0;
            ListView_GetItem(m_sampleListView, &lvi);
            lvi.iGroupId = newGroupId;
            ListView_DeleteItem(m_sampleListView, iSelection);
            ListView_InsertItem(m_sampleListView, &lvi);

            // update sample in training set too
            sampleSet->SetSampleGroup(iSelection, newGroupId);
        }

    } else { // we just finished drawing a path
        ClipCursor(NULL);   // restore full cursor movement
        if (!m_videoRect.PtInRect(p)) {
            penPath->Reset();
            InvalidateRect(m_videoRect,FALSE);
            return 0;
        }
        pathComplete = true;

        Rect bounds;
        Rect videoBounds(0,0,VIDEO_X,VIDEO_Y);
        penPath->GetBounds(&bounds);
        bounds.Intersect(videoBounds);

        TrainingSample *sample = new TrainingSample(copyFrame, m_sampleListView, m_hImageList, bounds, currentGroupId);
        sampleSet->AddSample(sample);

        InvalidateRect(&m_videoRect, FALSE);
    }
	return 0;
}

LRESULT CVideoMarkup::OnTrack( UINT, WPARAM, LPARAM, BOOL& )
{
    int dwPosition  = SendMessage(m_slider, TBM_GETPOS, 0, 0);
    penPath->Reset();
    pathComplete = false;
    selectStart.X = 0;
    selectStart.Y = 0;
    selectCurrent = selectStart;
    ShowFrame(dwPosition);
    InvalidateRect(&m_videoRect, FALSE);
    return 0;
}

LRESULT CVideoMarkup::OnCreate(UINT, WPARAM, LPARAM, BOOL& )
{
    // create graphics structures
	HDC hdc = GetDC();
	hdcmem = CreateCompatibleDC(hdc);
	hbm = CreateCompatibleBitmap(hdc,WINDOW_X,WINDOW_Y);
	SelectObject(hdcmem,hbm);
	ReleaseDC(hdc);
	graphics = new Graphics(hdcmem);
	graphics->SetSmoothingMode(SmoothingModeAntiAlias);
    graphics->Clear(Color(255,255,255));
    penPath = new GraphicsPath();
    posPen = new Pen(Color(100,100,255,100),4);
    posPen->SetLineJoin(LineJoinRound);
    posSelectPen = new Pen(Color(100,100,255,100),2);
    negPen = new Pen(Color(100,255,100,100),4);
    negPen->SetLineJoin(LineJoinRound);
    negSelectPen = new Pen(Color(100,255,100,100),2);
    Matrix penMatrix(1.77, -.71, .71, .71, 0, 0);  // scale and rotate for diagonal wide-tip effect
    posPen->SetTransform(&penMatrix);
    negPen->SetTransform(&penMatrix);
    posBrush = new SolidBrush(Color(50,100,255,100));
    negBrush = new SolidBrush(Color(50,255,100,100));
    hoverBrush = new SolidBrush(Color(25, 50, 150, 255));
    pathComplete = false;
    draggingIcon = false;
    hTrashCursor = LoadCursor(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_TRASHCURSOR));
    hDropCursor = LoadCursor(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_DROPCURSOR));

    // Create the list of samples
    m_sampleListView.Create(m_hWnd, CRect(VIDEO_X,0,WINDOW_X-5,VIDEO_Y), _T(""), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_BORDER | LVS_ICON | LVS_AUTOARRANGE);

    // create the list of example images
    m_hImageList = ImageList_Create(SAMPLE_X, SAMPLE_Y, ILC_COLOR32 | ILC_MASK, 0, MAX_SAMPLES);
    ListView_SetImageList(m_sampleListView, m_hImageList, LVSIL_NORMAL);
    ListView_SetIconSpacing(m_sampleListView, SAMPLE_X*2, SAMPLE_Y*1.5);
    ListView_EnableGroupView(m_sampleListView, TRUE);
    ListView_SetExtendedListViewStyle(m_sampleListView, LVS_EX_DOUBLEBUFFER | LVS_EX_BORDERSELECT | LVS_EX_FLATSB);

    // create an image list for the group header images
    //hGroupHeaderImages = ImageList_Create(400,50,ILC_COLOR32 | ILC_MASK,2,2);
    //HBITMAP hbmPos = LoadBitmap(_Module.get_m_hInst(), MAKEINTRESOURCE(IDB_THUMBSUP));
    //HBITMAP hbmNeg = LoadBitmap(_Module.get_m_hInst(), MAKEINTRESOURCE(IDB_THUMBSDOWN));
    //ImageList_Add(hGroupHeaderImages, hbmPos, NULL);
    //ImageList_Add(hGroupHeaderImages, hbmNeg, NULL);
    //DeleteObject(hbmPos);
    //DeleteObject(hbmNeg);
    //ListView_SetGroupHeaderImageList(m_sampleListView, hGroupHeaderImages);

    // add the "positive" and "negative" groups
    AddListViewGroup(m_sampleListView, L"Positive Examples", 0);
    AddListViewGroup(m_sampleListView, L"Negative Examples", 1);
    AddListViewGroup(m_sampleListView, L"Trash", 2);

    // Create the "train" button
    m_trainButton.Create(m_hWnd, CRect(VIDEO_X+25,VIDEO_Y+5,VIDEO_X+225,VIDEO_Y+SLIDER_Y), _T("Learn from Examples"), WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON);

    // Create the "show" button
    m_showButton.Create(m_hWnd, CRect(VIDEO_X+250,VIDEO_Y+5,VIDEO_X+450,VIDEO_Y+SLIDER_Y), _T("Show Some Guesses!"), WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON);

    // Create the video slider
    m_slider.Create(m_hWnd, CRect(5,VIDEO_Y+5,VIDEO_X-5,VIDEO_Y+SLIDER_Y), _T(""), WS_CHILD | WS_VISIBLE | WS_DISABLED | TBS_NOTICKS | TBS_BOTH );

    return 0;
}


LRESULT CVideoMarkup::OnDestroy( UINT, WPARAM, LPARAM, BOOL& )
{
    ImageList_RemoveAll(m_hImageList);
    ImageList_Destroy(m_hImageList);
	delete graphics;
    delete penPath;
    delete posPen;
    delete posSelectPen;
    delete posBrush;
    delete negPen;
    delete negSelectPen;
    delete negBrush;
    delete hoverBrush;
    // TODO: figure out how to delete bmpVideo when data buffer has already been cleared (unlock bits?)
    //    delete bmpVideo;
	DeleteDC(hdcmem);
	DeleteObject(hbm);
    DeleteObject(hTrashCursor);
    DeleteObject(hDropCursor);
    m_sampleListView.DestroyWindow();
    m_slider.DestroyWindow();
    m_showButton.DestroyWindow();
    m_trainButton.DestroyWindow();
    sampleSet->DeleteAllSamples();
    delete sampleSet;
    
    // TODO: non window-related variables should be deleted in destructor instead of here

    PostQuitMessage( 0 );
	return 0;
}

LRESULT CVideoMarkup::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& ) {
    HWND hwndControl = (HWND) lParam;
    if (hwndControl == m_trainButton) {

        // TODO: training should make a progress bar
        EnableControls(FALSE);
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        if (!classifier->isTrained) {
            classifier->Train(sampleSet);
        } else {
            classifier->AddStage(sampleSet);
        }
        EnableControls(TRUE);
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        m_trainButton.SetWindowTextW(L"Think Harder!");
    } else if (hwndControl == m_showButton) {
        showGuesses = true;
        EnableControls(FALSE);
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        classifier->ClassifyFrame(copyFrame, &objGuesses);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        InvalidateRect(NULL,FALSE);
        EnableControls(TRUE);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        m_showButton.EnableWindow(FALSE);
    }
    return 0;
}

LRESULT CVideoMarkup::OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL&) {
    LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pnmh;
    if (lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT) {
        if (draggingIcon) {
            HDC hdc = lplvcd->nmcd.hdc;
            Graphics gListView(hdc);
            if (dragHover) {
                gListView.FillRectangle(hoverBrush, hoverRect);
            }
        }
    }
    return CDRF_DODEFAULT;
}

LRESULT CVideoMarkup::OnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL&) {
    POINT p;
    HIMAGELIST hImageListSingle, hImageListMerged;

    int numSelected = ListView_GetSelectedCount(m_sampleListView);
    int iSelection = -1;
    for (int iIndex=0; iIndex<numSelected; iIndex++) {
        iSelection = ListView_GetNextItem(m_sampleListView, iSelection, LVNI_SELECTED);
        if (iIndex == 0) { // first selected icon
            hDragImageList = ListView_CreateDragImage(m_sampleListView, iSelection, &p);
        } else { // subsequent icons
            hImageListSingle = ListView_CreateDragImage(m_sampleListView, iSelection, &p);
            hImageListMerged = ImageList_Merge(hDragImageList, 0, hImageListSingle, 0, iIndex*3, iIndex*3);
            ImageList_Destroy(hDragImageList);
            ImageList_Destroy(hImageListSingle);
            hDragImageList = hImageListMerged;
        }
    }

    ImageList_BeginDrag(hDragImageList, 0, SAMPLE_X/2, SAMPLE_Y/2);
    POINT pt = ((NM_LISTVIEW*)pnmh)->ptAction;
    RECT listViewRect;
    m_sampleListView.GetClientRect(&listViewRect);
    m_sampleListView.ClientToScreen(&pt);
    m_sampleListView.ClientToScreen(&listViewRect);

    ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
    draggingIcon = TRUE;
    SetCapture();
    return 0;
}

void CVideoMarkup::ShowFrame(long framenum) {
    if (!videoLoaded) return;
    cvSetCaptureProperty(videoCapture, CV_CAP_PROP_POS_FRAMES, framenum);
    currentFrame = cvQueryFrame(videoCapture);
    cvFlip(currentFrame,copyFrame);

    showGuesses = false;
    if (classifier->isTrained) {
        m_showButton.EnableWindow(TRUE);
    }

    Rect videoBounds(0, 0, VIDEO_X, VIDEO_Y);
    BitmapData bmData;
    bmData.Width = videoX;
    bmData.Height = videoY;
    bmData.PixelFormat = PixelFormat24bppRGB;
    bmData.Stride = copyFrame->widthStep;
    bmData.Scan0 = copyFrame->imageData;
    bmpVideo->LockBits(&videoBounds, ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &bmData);
    bmpVideo->UnlockBits(&bmData);
}
