#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "MotionClassifier.h"
#include "GestureClassifier.h"
#include "VideoRunner.h"

CVideoRunner::CVideoRunner(CWindow *caller) {
    videoCapture = NULL;
    copyFrame = NULL;
    motionHistory = NULL;
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

    // convert frame to grayscale for motion history computation
    cvCvtColor(copyFrame, motionBuf[last], CV_BGR2GRAY);
    int idx1 = last;
    int idx2 = (last + 1) % MOTION_NUM_IMAGES;
    last = idx2;

    // get difference between frames
    IplImage* silh = motionBuf[idx2];
    cvAbsDiff(motionBuf[idx1], motionBuf[idx2], silh);
    
    // threshold difference image and use it to update motion history image
    cvThreshold(silh, silh, MOTION_DIFF_THRESHOLD, 1, CV_THRESH_BINARY); 
    cvUpdateMotionHistory(silh, motionHistory, nFrames, MOTION_MHI_DURATION); // update MHI

    // start with a full mask (all on)
    cvSet(guessMask, cvScalar(0xFF));

    // apply filter chain to frame
    for (list<Classifier*>::iterator i=customClassifiers.begin(); i!=customClassifiers.end(); i++) {
        if ((*i)->classifierType == IDC_RADIO_MOTION) {
            ((MotionClassifier*)(*i))->ClassifyMotion(motionHistory, nFrames, guessMask);
        } else if ((*i)->classifierType == IDC_RADIO_GESTURE) {
            // TODO: trackList is a list of MotionTrack objects storing
            // all the trajectories at current frame
//            vector<MotionTrack> trackList;
//            m_videoLoader.GetTrajectoriesAtCurrentFrame(&trackList);
//            for (int i=0; i<trackList.size(); i++) {
//                MotionTrack mt = trackList[i];
//                ((GestureClassifier*)(*i))->ClassifyTrack(mt, guessMask);
//            }
        } else {
            (*i)->ClassifyFrame(copyFrame, guessMask);
        } 
    }

    cvZero(outputFrame);
    cvCopy(copyFrame, outputFrame, guessMask);

    // convert to bitmap
    IplToBitmap(copyFrame, bmpInput);
    IplToBitmap(outputFrame, bmpOutput);

    // invalidate parent rectangle for redraw
    CRect videoRect(FILTERLIBRARY_WIDTH, 0, WINDOW_X, WINDOW_Y);
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

    // create image to store motion history
    motionHistory = cvCreateImage(cvSize(videoX,videoY), IPL_DEPTH_32F, 1);
    cvZero(motionHistory);

    // Allocate an image history ring buffer
    memset(motionBuf, 0, MOTION_NUM_IMAGES*sizeof(IplImage*));
    for(int i = 0; i < MOTION_NUM_IMAGES; i++) {
        motionBuf[i] = cvCreateImage(cvSize(copyFrame->width,copyFrame->height), IPL_DEPTH_8U, 1);
        cvZero(motionBuf[i]);
    }
    last = 0;

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
    cvReleaseImage(&motionHistory);
    for(int i = 0; i < MOTION_NUM_IMAGES; i++) {
        cvReleaseImage(&motionBuf[i]);
    }
    delete bmpInput;
    delete bmpOutput;
}