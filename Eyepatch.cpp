#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "FilterSelect.h"
#include "VideoControl.h"
#include "VideoLoader.h"
#include "VideoRecorder.h"
#include "Classifier.h"
#include "VideoMarkup.h"
#include "Eyepatch.h"

CEyepatch::CEyepatch() {
}


CEyepatch::~CEyepatch() {
}

LRESULT CEyepatch::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& ) {
    CvCapture *capture;
    if (HIWORD(wParam) == 0) { // this command came from a menu
        switch (LOWORD(wParam)) {
            case ID_FILE_OPENVIDEO:
                m_videoMarkup.OpenVideoFile();
                break;
            case ID_FILE_RECORDVIDEO:
                m_videoMarkup.RecordVideoFile();
                break;
            case ID_FILE_OPENSAMPLE:
                LoadSampleFromFile();
                break;
            case ID_FILE_EMPTYTRASH:
                m_videoMarkup.EmptyTrash();
                break;
            case ID_SETTINGS_CAPTURESOURCE:
                capture = cvCreateCameraCapture(0);
                cvSetCaptureProperty(capture, CV_CAP_PROP_DIALOG_SOURCE, 0 );
                cvReleaseCapture(&capture);
                break;
            case ID_SETTINGS_CAPTUREFORMAT:
                capture = cvCreateCameraCapture(0);
                cvSetCaptureProperty(capture, CV_CAP_PROP_DIALOG_FORMAT, 0 );
                cvReleaseCapture(&capture);
                break;
            case ID_FILE_EXIT:
                PostMessage(WM_CLOSE, 0, 0);
                break;
        }
    }
    return 0;
}

LRESULT CEyepatch::OnCreate(UINT, WPARAM, LPARAM, BOOL& )
{
    m_videoMarkup.Create(m_hWnd, CRect(0,0,WINDOW_X,WINDOW_Y), FILTER_CREATE_CLASS, WS_CHILD | WS_VISIBLE);

    // Create the menu
    hMenu = LoadMenu(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU));
    SetMenu(hMenu);

    return 0;
}


LRESULT CEyepatch::OnDestroy( UINT, WPARAM, LPARAM, BOOL& )
{
    m_videoMarkup.DestroyWindow();
    DestroyMenu(hMenu);
    PostQuitMessage( 0 );
	return 0;
}

void CEyepatch::LoadSampleFromFile() {
    USES_CONVERSION;
    WCHAR szFileNameList[MAX_PATH*16] = L"";
    WCHAR szPathName[MAX_PATH] = L"";
    WCHAR szFileName[MAX_PATH] = L"";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"Image Files\0*.jpg;*.png;*.bmp;*.dib;*.jpeg;*.pbm;*.ppm;*.pgm;*.sr;*.ras;*.tiff;*.tif\0";
    ofn.lpstrFile = szFileNameList;
    ofn.nMaxFile = MAX_PATH*16;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"jpg";

    if(!GetOpenFileName(&ofn)) {
        return;
    }

    WCHAR *strPtr = szFileNameList;
    wcscpy(szPathName, strPtr);
    strPtr += (wcslen(strPtr) + 1);

    if (wcslen(strPtr) == 0) { // only one filename selected
        m_videoMarkup.OpenSampleFile(W2A(szPathName));
    } else { // step through all the filenames
        while(wcslen(strPtr) != 0) {
            wcscpy(szFileName, szPathName);
            wcscat(szFileName, L"\\" );
            wcscat(szFileName, strPtr);
            m_videoMarkup.OpenSampleFile(W2A(szFileName));
            strPtr += (wcslen(strPtr) + 1);
        }
    }
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	MSG msg;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CEyepatch *wnd = new CEyepatch();
	wnd->Create(NULL, CRect(0,0,WINDOW_X,WINDOW_Y), APP_CLASS);
	while( GetMessage( &msg, NULL, 0, 0 ) ){
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	delete wnd;

	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}