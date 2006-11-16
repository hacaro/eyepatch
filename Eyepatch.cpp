#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "HaarClassifier.h"
#include "VideoMarkup.h"
#include "Eyepatch.h"

CEyepatch::CEyepatch() {
}


CEyepatch::~CEyepatch() {
}

LRESULT CEyepatch::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& ) {
    if (HIWORD(wParam) == 0) { // this command came from a menu
        switch (LOWORD(wParam)) {
            case ID_FILE_OPENVIDEO:
                m_videoMarkup.OpenVideoFile();
                break;
            case ID_FILE_RECORDVIDEO:
                m_videoMarkup.RecordVideoFile();
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

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	MSG msg;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CEyepatch wnd;
	wnd.Create(NULL, CRect(0,0,WINDOW_X,WINDOW_Y), APP_CLASS);

	while( GetMessage( &msg, NULL, 0, 0 ) ){
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}
