#include "precomp.h"
#include "../constants.h"
#include "TrajectoryList.h"
#include "BlobTracker.h"

#define GESTURE_NUM_FGTRAINING_FRAMES 10

CBlobTrackerDialog::CBlobTrackerDialog(BlobTracker *p) :
	videoRect(10, 35, 330, 275), 
	drawRect(10, 35, 320, 240) {
	m_hMutex = NULL;
	parent = p;
}

CBlobTrackerDialog::~CBlobTrackerDialog() {
	if (m_hMutex) CloseHandle(m_hMutex);
}

LRESULT CBlobTrackerDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow();
	m_hMutex = CreateMutex(NULL,FALSE,NULL);
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
	return TRUE;    // let the system set the focus
}

LRESULT CBlobTrackerDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CBlobTrackerDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {	
	EndDialog(IDCANCEL);
    return 0;
}

LRESULT CBlobTrackerDialog::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
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

LRESULT CBlobTrackerDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WaitForSingleObject(m_hMutex,INFINITE);
	TerminateThread(m_hThread, 0);
	return 0;
}

DWORD WINAPI CBlobTrackerDialog::ThreadCallback(CBlobTrackerDialog* instance) {
	instance->ConvertFrames();
	return 1L;
}

void CBlobTrackerDialog::ConvertFrames() {
	while (1) {
		WaitForSingleObject(m_hMutex,INFINITE);
		if (parent->currentFrame != NULL) {
			parent->ProcessFrame();

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


BlobTracker::BlobTracker() :
	m_BlobTrackerDialog(this) {

    isTrained = false;

    ZeroMemory(&param, sizeof(CvBlobTrackerAutoParam1));

    // Set number of foreground training frames
    param.FGTrainFrames = GESTURE_NUM_FGTRAINING_FRAMES;

    // Create FG Detection module
    param.pFG = cvCreateFGDetectorBase(CV_BG_MODEL_FGD, NULL);

    // Create Blob Entrance Detection module
//    param.pBD = cvCreateBlobDetectorSimple();
    param.pBD = cvCreateBlobDetectorCC();
    param.pBD->SetParam("MinDistToBorder",1.0);
    param.pBD->SetParam("Latency",5);
    param.pBD->SetParam("HMin",0.08);
    param.pBD->SetParam("WMin",0.08);

    // Create blob tracker module
    param.pBT = cvCreateBlobTrackerCCMSPF();

    // create blob trajectory generation module (currently not needed)
    param.pBTGen = (CvBlobTrackGen*) &trajectories;

    // create blob trajectory post processing module
    param.pBTPP = cvCreateModuleBlobTrackPostProcKalman();

    // create blob trajectory analysis module (currently not needed)
    param.UsePPData = 0;
    param.pBTA = NULL;

    // create a pipeline using these components
    pTracker = cvCreateBlobTrackerAuto1(&param);
}

BlobTracker::~BlobTracker() {

    // Release all the modules
    if(param.pBT)cvReleaseBlobTracker(&param.pBT);
    if(param.pBD)cvReleaseBlobDetector(&param.pBD);
    if(param.pBTGen)cvReleaseBlobTrackGen(&param.pBTGen);
    if(param.pBTA)cvReleaseBlobTrackAnalysis(&param.pBTA);
    if(param.pFG)cvReleaseFGDetector(&param.pFG);
    if(pTracker)cvReleaseBlobTrackerAuto(&pTracker);
}

void BlobTracker::ProcessFrame() {
    // make sure the image isn't upside down
    if (currentFrame->origin  == IPL_ORIGIN_TL) {
        cvCopy(currentFrame,copyFrame);
    } else {
        cvFlip(currentFrame,copyFrame);
    }

    // Process the new frame with the blob tracker
    // the second parameter is an optional mask
    pTracker->Process(copyFrame, NULL);

    // make a color copy of the grayscale foreground mask
    IplImage* fgImage = pTracker->GetFGMask();
    cvCvtColor(fgImage, fgImageCopy, CV_GRAY2BGR );

    // mask the current frame with the foreground mask
    cvAnd(copyFrame, fgImageCopy, maskedCopyFrame);

    // overlay masked image on original image
    cvAddWeighted(copyFrame, 0.3, maskedCopyFrame, 0.7, 0.0, maskedCopyFrame);

    int nBlobs = pTracker->GetBlobNum();
    if (nBlobs>0) { // some blobs were detected in the current frame
        for(int i=0; i<nBlobs; i++) {
            CvSize  TextSize;
            CvBlob* pB = pTracker->GetBlob(i-1);
            CvPoint center = cvPoint(cvRound(pB->x*256),cvRound(pB->y*256));
            CvSize  size = cvSize(MAX(1,cvRound(CV_BLOB_RX(pB)*256)), MAX(1,cvRound(CV_BLOB_RY(pB)*256)));

            cvEllipse(maskedCopyFrame, center, size, 0, 0, 360, CV_RGB(0,255,0), 1, CV_AA, 8);

            // TODO: should we use the blob id (int) or state description (char*) ?
            //  CV_BLOB_ID(pB)
            //  pTracker->GetStateDesc(CV_BLOB_ID(pB));
        }
    }

    // display color foreground image in window
    IplToBitmap(maskedCopyFrame, bmpVideo);

    // grab the next frame of the video
    currentFrame = cvQueryFrame(videoCapture);
}

void BlobTracker::LearnTrajectories(CvCapture* vidCap) {

    videoCapture = vidCap;
    if (!videoCapture) return;

    int nFrames = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_COUNT);
    cvSetCaptureProperty(videoCapture, CV_CAP_PROP_POS_FRAMES, 0);

    // TODO: display an informative error message if there are not enough frames
    if (nFrames < GESTURE_NUM_FGTRAINING_FRAMES+GESTURE_MIN_TRAJECTORY_LENGTH) return;

    currentFrame = cvQueryFrame(videoCapture);
    if(currentFrame == NULL) return;

    copyFrame = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,3);
    maskedCopyFrame = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,3);
    fgImageCopy = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,3);

    // Create a bitmap to display video
    bmpVideo = new Bitmap(currentFrame->width, currentFrame->height, PixelFormat24bppRGB);

    m_BlobTrackerDialog.DoModal();

    cvReleaseImage(&copyFrame);
    cvReleaseImage(&maskedCopyFrame);
    cvReleaseImage(&fgImageCopy);

    isTrained = true;
}

void BlobTracker::GetTrajectoriesInRange(vector<MotionTrack> *trackList, long startFrame, long endFrame) {
    trajectories.GetTracksInRange(trackList, startFrame, endFrame);
}

void BlobTracker::GetTrajectoriesAtFrame(vector<MotionTrack> *trackList, long frameNum) {
    trajectories.GetTracksAtFrame(trackList, frameNum);
}