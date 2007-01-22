#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "VideoRunner.h"

CVideoRunner::CVideoRunner(CWindow *caller) {
    videoCapture = NULL;
    copyFrame = NULL;
    bmpInput = NULL;
    bmpOutput = NULL;
    processingVideo = false;
    nFrames = 0;
    parent = caller;

	m_hMutex = NULL;
    m_hThread = NULL;
}

CVideoRunner::~CVideoRunner(void) {
    if (processingVideo) {
        StopProcessing();
    }
}

void CVideoRunner::ProcessFrame() {
	if (currentFrame == NULL) return;

    WaitForSingleObject(m_hMutex,INFINITE);

    // load frame and flip if needed
	if (currentFrame->origin  == IPL_ORIGIN_TL) {
		cvCopy(currentFrame,copyFrame);
	} else {
		cvFlip(currentFrame,copyFrame);
	}

    // start with a full mask (all on)
    cvSet(guessMask, cvScalar(255), 0);

    // apply filter chain to frame
    for (list<Classifier*>::iterator i = customClassifiers.begin();
        i != customClassifiers.end(); i++) {
        (*i)->ClassifyFrame(copyFrame, guessMask);
    }

    cvZero(outputFrame);
    cvCopy(copyFrame, outputFrame, guessMask);

    // convert to bitmap
    IplToBitmap(copyFrame, bmpInput);
    IplToBitmap(outputFrame, bmpOutput);

    // invalidate parent rectangle for redraw
    CRect videoRect(400, 0, 800, 700);
    if (parent->IsWindow()) {
        parent->InvalidateRect(&videoRect, FALSE);
    }

    // Grab next frame
    nFrames++;
	currentFrame = cvQueryFrame(videoCapture);

    ReleaseMutex(m_hMutex);
}

DWORD WINAPI CVideoRunner::ThreadCallback(CVideoRunner* instance) {
    while (1) {
        if (instance->currentFrame != NULL) {
	        instance->ProcessFrame();
        } else {
	        return 1L;
        }
    }
    return 1L;
}

void CVideoRunner::StartProcessing() {

    // Attempt to access the camera and get dimensions
    CvCapture *vc = cvCreateCameraCapture(0);

    if (vc == NULL) {
		MessageBox(GetActiveWindow(),
			L"Sorry, I'm unable to connect to a camera.  Please make sure that your camera is plugged in and its drivers are installed.", 
			L"Error Accessing Camera", MB_OK | MB_ICONERROR);
		return;
	}

    // get video capture properties
	videoCapture = vc;
    currentFrame = cvQueryFrame(videoCapture);
    videoX = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_WIDTH);
    videoY = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_HEIGHT);
    fps = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FPS);

	// create images to store a copy of the current frame input and output
    copyFrame = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,3);
    outputFrame = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,3);

    // create a mask to store the results of the processing
    guessMask = cvCreateImage(cvSize(videoX,videoY),IPL_DEPTH_8U,1);

    // Create bitmaps to display video input and output
    bmpInput = new Bitmap(videoX, videoY, PixelFormat24bppRGB);
    bmpOutput = new Bitmap(videoX, videoY, PixelFormat24bppRGB);

    processingVideo = true;

    // Start processing thread
	m_hMutex = CreateMutex(NULL,FALSE,NULL);
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
}

void CVideoRunner::StopProcessing() {
    if (!processingVideo) return;

    processingVideo = false;

    // End processing thread
//	WaitForSingleObject(m_hMutex,INFINITE);
	if (m_hMutex) CloseHandle(m_hMutex);
	TerminateThread(m_hThread, 0);

    cvReleaseCapture(&videoCapture);
    cvReleaseImage(&copyFrame);
    cvReleaseImage(&outputFrame);
	delete bmpInput;
    delete bmpOutput;
}