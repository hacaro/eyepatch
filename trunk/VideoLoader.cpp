#include "precomp.h"
#include "constants.h"
#include "VideoLoader.h"

CVideoLoaderDialog::CVideoLoaderDialog(CVideoLoader *p) :
	videoRect(10, 35, 330, 275), 
	drawRect(10, 35, 320, 240) {
	m_hMutex = NULL;
	parent = p;
}

CVideoLoaderDialog::~CVideoLoaderDialog() {
	if (m_hMutex) CloseHandle(m_hMutex);
}

LRESULT CVideoLoaderDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow();
	m_hMutex = CreateMutex(NULL,FALSE,NULL);
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
	return TRUE;    // let the system set the focus
}

LRESULT CVideoLoaderDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CVideoLoaderDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {	
	EndDialog(IDCANCEL);
    return 0;
}

LRESULT CVideoLoaderDialog::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
{
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
	Graphics graphics(hdc);
	if (parent->bmpVideo != NULL) {
		graphics.DrawImage(parent->bmpVideo, drawRect);
	}
    EndPaint(&ps);
    return 0;
}

LRESULT CVideoLoaderDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WaitForSingleObject(m_hMutex,INFINITE);
	TerminateThread(m_hThread, 0);
	return 0;
}

DWORD WINAPI CVideoLoaderDialog::ThreadCallback(CVideoLoaderDialog* instance) {
	instance->ConvertFrames();
	return 1L;
}

void CVideoLoaderDialog::ConvertFrames() {
	while (1) {
		WaitForSingleObject(m_hMutex,INFINITE);
		if (parent->currentFrame != NULL) {
			parent->ConvertFrame();

			// Redraw dialog window
			InvalidateRect(&videoRect,FALSE);
		} else {
			ReleaseMutex(m_hMutex);
			EndDialog(0);
			return;
		}
		ReleaseMutex(m_hMutex);
   }
}


CVideoLoader::CVideoLoader() :
	m_hVideoLoaderDialog(this) {
    videoLoaded = FALSE;
    videoCapture = NULL;
    copyFrame = NULL;
    motionHistory = NULL;
    bmpVideo = NULL;
	nFrames = 0;
    currentFrameNumber = 0;
}

CVideoLoader::~CVideoLoader(void) {
    if (videoLoaded) {
        cvReleaseCapture(&videoCapture);
        cvReleaseImage(&copyFrame);
        cvReleaseImage(&motionHistory);
        delete bmpVideo;
    }
}

BOOL CVideoLoader::OpenVideoFile(HWND hwndOwner) {
    WCHAR szFileName[MAX_PATH] = L"";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFilter = L"Video Files\0*.avi;*.mpg;*.mp4;*.wmv;*.flv;*.mpeg;*.m2v;*.mpv;*.mov;*.qt;*.vob;*.rm\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"avi";

    if(!GetOpenFileName(&ofn)) {
	    return FALSE;
    }
    return OpenVideoFile(hwndOwner, szFileName);
}

BOOL CVideoLoader::OpenVideoFile(HWND hwndOwner, LPCWSTR filename) {
    USES_CONVERSION;

    WCHAR szFileName[MAX_PATH] = L"";
    wcscpy(szFileName, filename);

    // Attempt to load the video file and get dimensions
    CvCapture *vc = cvCreateFileCapture(W2A(szFileName));

    if (vc == NULL) {
		MessageBox(GetActiveWindow(), 
			L"Sorry, I'm unable to load this video file.  It may be in a format I can't recognize.", 
			L"Error Loading Video", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if (videoLoaded) { // We already loaded a video, so this will be a new one
		cvReleaseCapture(&videoCapture);
		cvReleaseImage(&copyFrame);
		cvReleaseImage(&motionHistory);
		delete bmpVideo;
		videoLoaded = false;
	}
	videoCapture = vc;

    currentFrame = cvQueryFrame(videoCapture);
    videoX = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_WIDTH);
    videoY = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_HEIGHT);
    nFrames = cvGetCaptureProperty(videoCapture,  CV_CAP_PROP_FRAME_COUNT);

	// create an image to store a copy of the current frame
    copyFrame = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,3);

    // create an image to store the motion history (for motion tracking)
    motionHistory = cvCreateImage(cvSize(videoX,videoY), IPL_DEPTH_32F, 1);

    // Create a bitmap to display video
    bmpVideo = new Bitmap(videoX, videoY, PixelFormat24bppRGB);

	if (nFrames == 0) {
		// this is not a seekable format, so we will convert it and save to a file
		wcscat(szFileName,L".seekable.avi");
		videoWriter = cvCreateVideoWriter(W2A(szFileName), CV_FOURCC('D','I','V','X'), 30, cvSize(videoX, videoY), 1);
		m_hVideoLoaderDialog.DoModal();
		cvReleaseVideoWriter(&videoWriter);
		cvReleaseCapture(&videoCapture);
		videoCapture = cvCreateFileCapture(W2A(szFileName));
	}

    LoadFrame(0);
	videoLoaded = TRUE;

	return TRUE;
}

void CVideoLoader::LoadFrame(long framenum) {

    if (!videoLoaded) return;
    cvSetCaptureProperty(videoCapture, CV_CAP_PROP_POS_FRAMES, framenum);
    currentFrame = cvQueryFrame(videoCapture);
	if (!currentFrame) return;

    if (currentFrame->origin  == IPL_ORIGIN_TL) {
        cvCopy(currentFrame,copyFrame);
    } else {
        cvFlip(currentFrame,copyFrame);
    }

    currentFrameNumber = framenum;

    IplToBitmap(copyFrame, bmpVideo);
    Rect videoBounds(0, 0, videoX, videoY);
}

void CVideoLoader::ConvertFrame() {

	if (currentFrame == NULL) return;

	Rect videoBounds(0,0,videoX,videoY);
	if (currentFrame->origin  == IPL_ORIGIN_TL) {
		cvCopy(currentFrame,copyFrame);
	} else {
		cvFlip(currentFrame,copyFrame);
	}
	cvWriteFrame(videoWriter, copyFrame);
	nFrames++;

    IplToBitmap(copyFrame, bmpVideo);

    // Grab next frame
	currentFrame = cvQueryFrame(videoCapture);
}

IplImage* CVideoLoader::GetMotionHistory() {
    if (!videoLoaded) return NULL;
    cvZero(motionHistory);

    if (currentFrameNumber < MOTION_NUM_IMAGES+1) return motionHistory;

    // Allocate an image history ring buffer
    IplImage* buf[MOTION_NUM_IMAGES];
    memset(buf, 0, MOTION_NUM_IMAGES*sizeof(IplImage*));
    for(int i = 0; i < MOTION_NUM_IMAGES; i++) {
        buf[i] = cvCreateImage(cvSize(copyFrame->width,copyFrame->height), IPL_DEPTH_8U, 1);
        cvZero(buf[i]);
    }

    int last = 0;
    int idx1 = 0;
    int idx2 = 0;
    IplImage* silh = NULL;

    int startFrame = max(0, currentFrameNumber-MOTION_NUM_HISTORY_FRAMES);

    for (int i=startFrame; i<=currentFrameNumber; i++) {

        cvSetCaptureProperty(videoCapture, CV_CAP_PROP_POS_FRAMES, i);
        currentFrame = cvQueryFrame(videoCapture);
	    if (!currentFrame) return motionHistory;

        if (currentFrame->origin  == IPL_ORIGIN_TL) {
            cvCopy(currentFrame,copyFrame);
        } else {
            cvFlip(currentFrame,copyFrame);
        }
        
        // convert frame to grayscale
        cvCvtColor(copyFrame, buf[last], CV_BGR2GRAY);
        idx1 = last;
        idx2 = (last + 1) % MOTION_NUM_IMAGES;
        last = idx2;

        silh = buf[idx2];
        cvAbsDiff( buf[idx1], buf[idx2], silh ); // get difference between frames
        
        cvThreshold(silh, silh, MOTION_DIFF_THRESHOLD, 1, CV_THRESH_BINARY ); // and threshold it
        if (i != startFrame) {
            cvUpdateMotionHistory(silh, motionHistory, (i-startFrame), MOTION_MHI_DURATION); // update MHI
        }
    }

    for(int i = 0; i < MOTION_NUM_IMAGES; i++) {
        cvReleaseImage(&buf[i]);
    }

    return motionHistory;
}

void CVideoLoader::LearnTrajectories() { 
    if (!videoLoaded) return;
    m_blobTracker.LearnTrajectories(videoCapture);
    LoadFrame(currentFrameNumber);
}

void CVideoLoader::GetTrajectoriesInRange(vector<MotionTrack> *trackList, long startFrame, long endFrame) {
    m_blobTracker.GetTrajectoriesInRange(trackList, startFrame, endFrame);
}

void CVideoLoader::GetTrajectoriesAtCurrentFrame(vector<MotionTrack> *trackList) {
    long startFrame = max(0,currentFrameNumber-GESTURE_MIN_TRAJECTORY_LENGTH);
    m_blobTracker.GetTrajectoriesAtFrame(trackList, currentFrameNumber);
}
