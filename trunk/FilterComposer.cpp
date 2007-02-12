#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "VideoLoader.h"
#include "VideoRecorder.h"
#include "BrightnessClassifier.h"
#include "ColorClassifier.h"
#include "ShapeClassifier.h"
#include "SiftClassifier.h"
#include "HaarClassifier.h"
#include "MotionClassifier.h"
#include "GestureClassifier.h"
#include "Gesture/BlobTracker.h"
#include "FilterLibrary.h"
#include "VideoRunner.h"
#include "FilterComposer.h"

CFilterComposer::CFilterComposer() :
    m_filterLibrary(this),
    m_videoRunner(this),
    labelFont(L"Verdana", 10),
    whiteBrush(Color(255,255,255)),
    blackBrush(Color(0,0,0)),
    blackPen(Color(0,0,0),4),
    arrowPen(Color(100,255,100), 10) {
    arrowPen.SetEndCap(LineCapArrowAnchor);
}

CFilterComposer::~CFilterComposer() {
}

LRESULT CFilterComposer::OnPaint( UINT, WPARAM, LPARAM, BOOL& ) {
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);

    graphics->Clear(Color(240,240,240));

    graphics->FillRectangle(&whiteBrush, 10, 40, 320, 240);
    graphics->DrawRectangle(&blackPen, 10, 40, 320, 240);
    graphics->DrawString(L"INPUT VIDEO", 11, &labelFont, PointF(15,45), &blackBrush);

    if (m_videoRunner.processingVideo) {
        graphics->DrawImage(m_videoRunner.bmpInput, 10, 40, 320, 240);
    }

    graphics->FillRectangle(&whiteBrush, 10, 420, 320, 240);
    graphics->DrawRectangle(&blackPen, 10, 420, 320, 240);
    graphics->DrawString(L"FILTERED VIDEO", 14, &labelFont, PointF(15,425), &blackBrush);

    if (m_videoRunner.processingVideo) {
        graphics->DrawImage(m_videoRunner.bmpOutput, 10, 420, 320, 240);
    }

    BitBlt(hdc,FILTERLIBRARY_WIDTH,0,WINDOW_X-FILTERLIBRARY_WIDTH,WINDOW_Y,hdcmem,0,0,SRCCOPY);
    EndPaint(&ps);

    return 0;
}

LRESULT CFilterComposer::OnButtonDown( UINT, WPARAM wParam, LPARAM lParam, BOOL& )
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

    return 0;
}

LRESULT CFilterComposer::OnMouseMove( UINT, WPARAM wParam, LPARAM lParam, BOOL& )
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

	return 0;
}

LRESULT CFilterComposer::OnButtonUp( UINT, WPARAM, LPARAM lParam, BOOL&)
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

	return 0;
}

LRESULT CFilterComposer::OnCreate(UINT, WPARAM, LPARAM, BOOL& )
{
	HDC hdc = GetDC();
	hdcmem = CreateCompatibleDC(hdc);
	hbm = CreateCompatibleBitmap(hdc,WINDOW_X-FILTERLIBRARY_WIDTH,WINDOW_Y);
	SelectObject(hdcmem,hbm);
	ReleaseDC(hdc);
    graphics = new Graphics(hdcmem);
	graphics->SetSmoothingMode(SmoothingModeAntiAlias);
    graphics->Clear(Color(240,240,240));

    // Create the filter library dialog
    m_filterLibrary.Create(m_hWnd, WS_CHILD | WS_VISIBLE);
    m_filterLibrary.MoveWindow(0, 0, FILTERLIBRARY_WIDTH, WINDOW_Y);
    m_filterLibrary.ShowWindow(TRUE);

    return 0;
}


LRESULT CFilterComposer::OnDestroy( UINT, WPARAM, LPARAM, BOOL& ) {

    if (m_videoRunner.processingVideo) {
        m_videoRunner.StopProcessing();
    }
    
    delete graphics;
	DeleteDC(hdcmem);
	DeleteObject(hbm);

    m_filterLibrary.DestroyWindow();

    PostQuitMessage( 0 );
	return 0;
}

LRESULT CFilterComposer::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HWND listView;
    HMENU hMenu;
    switch(wParam) {
        case IDC_RADIO_COLOR:
        case IDC_RADIO_SHAPE:
        case IDC_RADIO_FEATURES:
        case IDC_RADIO_BRIGHTNESS:
        case IDC_RADIO_APPEARANCE:
        case IDC_RADIO_MOTION:
        case IDC_RADIO_GESTURE:
            m_videoRunner.AddActiveFilter((Classifier*)lParam);
            listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_FILTER_LIST);
            m_filterLibrary.AddCustomFilter(listView, (Classifier*)lParam);
            InvalidateRect(NULL, FALSE);
            break;
        case IDC_RUNLIVE:
            hMenu = ::GetMenu(this->GetParent());
            if (!m_videoRunner.processingVideo) {
                m_videoRunner.StartProcessing();
                if (m_videoRunner.processingVideo) {
                    m_filterLibrary.GetDlgItem(IDC_RUNRECORDED).EnableWindow(FALSE);
                    m_filterLibrary.GetDlgItem(IDC_RUNLIVE).SetWindowText(L"Stop Running");

                    // disable the menu
                    EnableMenuItem(hMenu, 0, MF_BYPOSITION | MF_GRAYED);
                    EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_GRAYED);
                    EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED);
                    ::DrawMenuBar(GetParent());
                }
            } else {
                m_videoRunner.StopProcessing();
                m_filterLibrary.GetDlgItem(IDC_RUNRECORDED).EnableWindow(TRUE);
                m_filterLibrary.GetDlgItem(IDC_RUNLIVE).SetWindowText(L"Run on Live Video!");

                // reenable the menu
                EnableMenuItem(hMenu, 0, MF_BYPOSITION | MF_ENABLED);
                EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
                EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED);
                ::DrawMenuBar(GetParent());
            }
            break;
        case IDC_RESET:
            ClearActiveClassifiers();
            break;
        default:
            break;
    }
    return 0;
}

void CFilterComposer::LoadCustomClassifier(LPWSTR pathname) {

    Classifier *newclassifier = NULL;

    if (wcsstr(pathname, FILE_BRIGHTNESS_SUFFIX) != NULL) {
        newclassifier = new BrightnessClassifier(pathname);
    } else if (wcsstr(pathname, FILE_COLOR_SUFFIX) != NULL) { 
        newclassifier = new ColorClassifier(pathname);
    } else if (wcsstr(pathname, FILE_GESTURE_SUFFIX) != NULL) { 
        newclassifier = new GestureClassifier(pathname);
    } else if (wcsstr(pathname, FILE_HAAR_SUFFIX) != NULL) { 
        newclassifier = new HaarClassifier(pathname);
    } else if (wcsstr(pathname, FILE_MOTION_SUFFIX) != NULL) { 
        newclassifier = new MotionClassifier(pathname);
    } else if (wcsstr(pathname, FILE_SHAPE_SUFFIX) != NULL) { 
        newclassifier = new ShapeClassifier(pathname);
    } else if (wcsstr(pathname, FILE_SIFT_SUFFIX) != NULL) { 
        newclassifier = new SiftClassifier(pathname);
    }

    if (newclassifier != NULL) {
        customClassifiers.push_back(newclassifier);
        HWND listView = m_filterLibrary.GetDlgItem(IDC_MY_FILTER_LIST);
        m_filterLibrary.AddCustomFilter(listView, newclassifier);
    }
}

void CFilterComposer::ClearCustomClassifiers() {
    for (list<Classifier*>::iterator i = customClassifiers.begin(); i!= customClassifiers.end(); i++) {
        delete (*i);
    }
    HWND listView = m_filterLibrary.GetDlgItem(IDC_MY_FILTER_LIST);
    m_filterLibrary.ClearCustomFilters(listView);
    customClassifiers.clear();
}

void CFilterComposer::ClearActiveClassifiers() {
    HWND listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_FILTER_LIST);
    m_filterLibrary.ClearCustomFilters(listView);
    m_videoRunner.ClearActiveFilters();
}
