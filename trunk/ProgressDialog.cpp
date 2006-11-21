#include "precomp.h"
#include "ProgressDialog.h"

CProgressDialog::CProgressDialog(IplImage* f, CvCapture* c, CvVideoWriter* w)
{
	frame = f;
	capture = c;
	writer = w;
	frameNum = 0;

    videoX = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    videoY = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);

	// create an image to store a copy of the current frame
    copyFrame = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,3);

    // Create a bitmap to display video
    bmpVideo = new Bitmap(videoX, videoY, PixelFormat24bppRGB);

	videoBounds = Rect(7,30,320,240);
	videoRect = CRect(7,30,320,240);
}

CProgressDialog::~CProgressDialog(void)
{
	CloseHandle(hMutex);
	cvReleaseImage(&copyFrame);
	delete bmpVideo;
}


DWORD WINAPI CProgressDialog::ThreadCallback(CProgressDialog* instance) {
	instance->ConvertFrames();
	return 1L;
}


void CProgressDialog::ConvertFrames() {
while (1)
   {
		WaitForSingleObject(hMutex,INFINITE);
		if (frame != NULL) {
			if (frame->origin  == IPL_ORIGIN_TL) {
				cvCopy(frame,copyFrame);
			} else {
				cvFlip(frame,copyFrame);
			}
			cvWriteFrame(writer, copyFrame);
			frameNum++;
			Rect videoBounds(0, 0, videoX, videoY);
			BitmapData bmData;
			bmData.Width = videoX;
			bmData.Height = videoY;
			bmData.PixelFormat = PixelFormat24bppRGB;
			bmData.Stride = copyFrame->widthStep;
			bmData.Scan0 = copyFrame->imageData;
			bmpVideo->LockBits(&videoBounds, ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &bmData);
			bmpVideo->UnlockBits(&bmData);

			// Redraw dialog window
			InvalidateRect(&videoRect,FALSE);

			// Grab next frame
			frame = cvQueryFrame(capture);
		} else {
			ReleaseMutex(hMutex);
			EndDialog(0);
			return;
		}
		ReleaseMutex(hMutex);
   }
}

LRESULT CProgressDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow();
	hMutex = CreateMutex(NULL,FALSE,NULL);
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
	return TRUE;    // let the system set the focus
}

LRESULT CProgressDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CProgressDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {	
	EndDialog(IDCANCEL);
    return 0;
}

LRESULT CProgressDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WaitForSingleObject(hMutex,INFINITE);
	TerminateThread(m_hThread, 0);
	return 0;
}


LRESULT CProgressDialog::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
{
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
	Graphics graphics(hdc);
    if (bmpVideo != NULL) graphics.DrawImage(bmpVideo,videoBounds);
    EndPaint(&ps);
    return 0;
}
