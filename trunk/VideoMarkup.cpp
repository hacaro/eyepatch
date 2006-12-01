#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "FilterSelect.h"
#include "VideoLoader.h"
#include "CamshiftClassifier.h"
#include "ShapeClassifier.h"
#include "SiftClassifier.h"
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
    m_filterSelect(this),
    m_videoRect(0,0,VIDEO_X,VIDEO_Y),
    m_filterRect(0, 0, VIDEO_X, FILTERIMAGE_Y+FILTERIMAGE_HEIGHT),
    posSelectPen(Color(100,100,255,100),2),
    negSelectPen(Color(100,255,100,100),2),
    guessPen(Color(100,255,100),4),
    posBrush(Color(50,100,255,100)),
    negBrush(Color(50,255,100,100)),
    hoverBrush(Color(25, 50, 150, 255)),
    grayBrush(Color(150, 0, 0, 0)),
    ltgrayBrush(Color(240,240,240)) {
	guessPen.SetLineJoin(LineJoinRound);

    // TODO: all non window-related variables should be initialized here instead of in OnCreate
    classifier = new CamshiftClassifier();
    showGuesses = false;
    selectingRegion = false;
    draggingIcon = false;
	scrubbingVideo = false;
	currentGroupId = 0;
}


CVideoMarkup::~CVideoMarkup() {
    delete classifier;
}

void CVideoMarkup::EnableControls(BOOL enabled) {
    m_filterSelect.EnableWindow(enabled);
    if (enabled && !classifier->isTrained) {
        m_filterSelect.GetDlgItem(IDC_SHOWBUTTON).EnableWindow(FALSE);
    }
	m_slider.EnableWindow(enabled);
    m_sampleListView.EnableWindow(enabled);
}

LRESULT CVideoMarkup::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
{
	PAINTSTRUCT ps;

    HDC hdc = BeginPaint(&ps);
    Rect drawBounds(0,0,VIDEO_X,VIDEO_Y);
    Rect videoBounds(0,0,m_videoLoader.videoX,m_videoLoader.videoY);
    
    if (m_videoLoader.videoLoaded) {
        graphics->SetClip(drawBounds);
        if (m_videoLoader.bmpVideo != NULL) graphics->DrawImage(m_videoLoader.bmpVideo,drawBounds);

        if (showGuesses && !scrubbingVideo) { // highlight computer's guesses
			REAL scaleX = ((REAL)VIDEO_X) / ((REAL)m_videoLoader.videoX);
			REAL scaleY = ((REAL)VIDEO_Y) / ((REAL)m_videoLoader.videoY);
			graphics->ScaleTransform(scaleX, scaleY);
			Region guessRegion;
			GraphicsPath guessPath;
			guessRegion.MakeEmpty();
            for (list<Rect>::iterator i = objGuesses.begin(); i != objGuesses.end(); i++) {
				guessPath.AddRectangle(*i);
				guessRegion.Union(*i);
            }
			guessRegion.Complement(videoBounds);
			graphics->FillRegion(&grayBrush, &guessRegion);
			graphics->SetClip(&guessRegion, CombineModeReplace);
			graphics->DrawPath(&guessPen, &guessPath);
			graphics->ResetTransform();
			graphics->SetClip(drawBounds, CombineModeReplace);
        }

        Rect selectRect;
        selectRect.X = (INT) min(selectStart.X, selectCurrent.X);
        selectRect.Y = (INT) min(selectStart.Y, selectCurrent.Y);
        selectRect.Width = (INT) abs(selectStart.X - selectCurrent.X);
        selectRect.Height = (INT) abs(selectStart.Y - selectCurrent.Y);

        if (selectingRegion) {
            if (currentGroupId == 0) {
                graphics->FillRectangle(&posBrush, selectRect);
                graphics->DrawRectangle(&posSelectPen, selectRect);
            } else {
                graphics->FillRectangle(&negBrush, selectRect);
                graphics->DrawRectangle(&negSelectPen, selectRect);
            }
        }
        graphics->ResetClip();
    }

    if (classifier->isTrained) {
        graphics->DrawImage(classifier->GetFilterImage(), FILTERIMAGE_X+1, FILTERIMAGE_Y+1);
    } else {
        graphics->FillRectangle(&ltgrayBrush, Rect(FILTERIMAGE_X, FILTERIMAGE_Y, WINDOW_X, WINDOW_Y));
    }
    if (showGuesses) {
        graphics->DrawImage(classifier->GetApplyImage(), FILTERIMAGE_X+FILTERIMAGE_WIDTH+10, FILTERIMAGE_Y+1);
    } else {
        graphics->FillRectangle(&ltgrayBrush, Rect(FILTERIMAGE_X+FILTERIMAGE_WIDTH, FILTERIMAGE_Y, WINDOW_X, WINDOW_Y));
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

    if (!m_videoLoader.videoLoaded) return 0;
    if (!m_videoRect.PtInRect(p)) return 0;

    selectingRegion = true;

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

    if (!m_videoLoader.videoLoaded) return 0;

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

    if (!m_videoLoader.videoLoaded) return 0;

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
        lvhti.pt = p;
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
            sampleSet.SetSampleGroup(iSelection, newGroupId);
        }

    } else if (selectingRegion) { // we just finished drawing a path
        ClipCursor(NULL);   // restore full cursor movement
        if (!m_videoRect.PtInRect(p)) {
            InvalidateRect(&m_videoRect,FALSE);
            return 0;
        }
        selectingRegion = false;

        Rect selectRect;
        selectRect.X = (INT) min(selectStart.X, selectCurrent.X);
        selectRect.Y = (INT) min(selectStart.Y, selectCurrent.Y);
        selectRect.Width = (INT) abs(selectStart.X - selectCurrent.X);
        selectRect.Height = (INT) abs(selectStart.Y - selectCurrent.Y);

        Rect drawBounds(0,0,VIDEO_X,VIDEO_Y);
        selectRect.Intersect(drawBounds);
        double scaleX = ((double)m_videoLoader.videoX) / ((double)VIDEO_X);
        double scaleY = ((double)m_videoLoader.videoY) / ((double)VIDEO_Y);
        selectRect.X = (INT) (scaleX * selectRect.X);
        selectRect.Y = (INT) (scaleY * selectRect.Y);
        selectRect.Width = (INT) (scaleX * selectRect.Width);
        selectRect.Height = (INT) (scaleY * selectRect.Height);

        // discard tiny samples since they won't help
        if ((selectRect.Width > 10) && (selectRect.Height > 10)) {
            TrainingSample *sample = new TrainingSample(m_videoLoader.copyFrame, m_sampleListView, m_hImageList, selectRect, currentGroupId);
            sampleSet.AddSample(sample);
        }
        InvalidateRect(&m_videoRect, FALSE);
    }
	return 0;
}

LRESULT CVideoMarkup::OnTrack( UINT, WPARAM wParam, LPARAM, BOOL& ) {
    long sliderPosition = (long) SendMessage(m_slider, TBM_GETPOS, 0, 0);
    selectingRegion = false;
	if (LOWORD(wParam) == SB_THUMBTRACK) {
		scrubbingVideo = true;
	} else {
		scrubbingVideo = false;
	}
    selectStart.X = 0;
    selectStart.Y = 0;
    selectCurrent = selectStart;

    m_videoLoader.LoadFrame(sliderPosition);
    if (showGuesses && !scrubbingVideo) {
        HCURSOR hOld = SetCursor(LoadCursor(0, IDC_WAIT));
        classifier->ClassifyFrame(m_videoLoader.copyFrame, &objGuesses);
        SetCursor(hOld);
    }
    InvalidateRect(&m_filterRect, FALSE);
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
    hTrashCursor = LoadCursor(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_TRASHCURSOR));
    hDropCursor = LoadCursor(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDC_DROPCURSOR));

    // Create the list of samples
    m_sampleListView.Create(m_hWnd, CRect(VIDEO_X,0,WINDOW_X-5,VIDEO_Y-50), _T(""), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_BORDER | LVS_ICON | LVS_AUTOARRANGE);

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

    // Create the video slider
    m_slider.Create(m_hWnd, CRect(5,VIDEO_Y+5,VIDEO_X-5,VIDEO_Y+SLIDER_Y), _T(""), WS_CHILD | WS_VISIBLE | WS_DISABLED | TBS_NOTICKS | TBS_BOTH );
	
    // Create the filter selector
    m_filterSelect.Create(m_hWnd, CRect(VIDEO_X,VIDEO_Y-50,WINDOW_X-5,WINDOW_Y), WS_CHILD | WS_VISIBLE | WS_DISABLED);
    m_filterSelect.MoveWindow(VIDEO_X+1, VIDEO_Y-50, WINDOW_X-VIDEO_X, WINDOW_Y-VIDEO_Y+50);
    m_filterSelect.CheckRadioButton(IDC_RADIO_COLOR, IDC_RADIO_APPEARANCE, IDC_RADIO_COLOR);
    m_filterSelect.ShowWindow(TRUE);
    m_filterSelect.EnableWindow(FALSE);

    return 0;
}


LRESULT CVideoMarkup::OnDestroy( UINT, WPARAM, LPARAM, BOOL& ) {
    ImageList_RemoveAll(m_hImageList);
    ImageList_Destroy(m_hImageList);
	delete graphics;
	DeleteDC(hdcmem);
	DeleteObject(hbm);
    DeleteObject(hTrashCursor);
    DeleteObject(hDropCursor);
    m_sampleListView.DestroyWindow();
    m_slider.DestroyWindow();
    m_filterSelect.DestroyWindow();
    // TODO: non window-related variables should be deleted in destructor instead of here
    PostQuitMessage( 0 );
	return 0;
}

LRESULT CVideoMarkup::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& ) {
    switch(wParam) {
        case IDC_TRAINBUTTON:
            if (sampleSet.posSampleCount < 1) {
                // TODO: display informative error message
                break;
            }
            EnableControls(FALSE);
		    classifier->StartTraining(&sampleSet);
            EnableControls(TRUE);
            break;
        case IDC_SHOWBUTTON:
            showGuesses = !showGuesses;
            break;
        case IDC_RADIO_COLOR:
            delete classifier;
            classifier = new CamshiftClassifier();
            m_filterSelect.CheckDlgButton(IDC_SHOWBUTTON, FALSE);
            m_filterSelect.GetDlgItem(IDC_SHOWBUTTON).EnableWindow(FALSE);
            objGuesses.clear();
            showGuesses = false;
            break;
        case IDC_RADIO_SHAPE:
            delete classifier;
            classifier = new ShapeClassifier();
            m_filterSelect.CheckDlgButton(IDC_SHOWBUTTON, FALSE);
            m_filterSelect.GetDlgItem(IDC_SHOWBUTTON).EnableWindow(FALSE);
            objGuesses.clear();
            showGuesses = false;
            break;
        case IDC_RADIO_FEATURES:
            delete classifier;
            classifier = new SiftClassifier();
            m_filterSelect.CheckDlgButton(IDC_SHOWBUTTON, FALSE);
            m_filterSelect.GetDlgItem(IDC_SHOWBUTTON).EnableWindow(FALSE);
            objGuesses.clear();
            showGuesses = false;
            break;
        case IDC_RADIO_APPEARANCE:
            delete classifier;
            classifier = new HaarClassifier();
            m_filterSelect.CheckDlgButton(IDC_SHOWBUTTON, FALSE);
            m_filterSelect.GetDlgItem(IDC_SHOWBUTTON).EnableWindow(FALSE);
            objGuesses.clear();
            showGuesses = false;
            break;
    }
    if (showGuesses) {
        HCURSOR hOld = SetCursor(LoadCursor(0, IDC_WAIT));
        classifier->ClassifyFrame(m_videoLoader.copyFrame, &objGuesses);
        SetCursor(hOld);
    }
    InvalidateRect(&m_filterRect,FALSE);

    return 0;
}

LRESULT CVideoMarkup::OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL&) {
    LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pnmh;
    if (lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT) {
        if (draggingIcon) {
            HDC hdc = lplvcd->nmcd.hdc;
            Graphics gListView(hdc);
            if (dragHover) {
                gListView.FillRectangle(&hoverBrush, hoverRect);
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

void CVideoMarkup::OpenVideoFile() {
    m_videoLoader.OpenVideoFile(m_hWnd);
	if (m_videoLoader.videoLoaded) {
		EnableControls(TRUE);
		SendMessage(m_slider, TBM_SETRANGEMIN, FALSE, 0);
		SendMessage(m_slider, TBM_SETRANGEMAX, FALSE, m_videoLoader.nFrames-1);
		SendMessage(m_slider, TBM_SETPOS, TRUE, 0);
		m_videoLoader.LoadFrame(0);
		InvalidateRect(&m_filterRect,FALSE);
	}
}