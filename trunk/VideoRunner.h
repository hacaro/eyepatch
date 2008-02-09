#pragma once
#include "SimpleFlowTracker.h"

class CFilterComposer;

class CVideoRunner
{
public: 
	CVideoRunner(CWindow *caller);
	~CVideoRunner();

    static DWORD WINAPI ThreadCallback(CVideoRunner*);
    void StartProcessing(bool isLive);
    void StopProcessing();
	void ProcessFrame();

    void AddActiveFilter(Classifier*);
    void ClearActiveFilters();
	void ResetActiveFilterRunningStates();

    bool AddActiveOutput(OutputSink *o);
    void ClearActiveOutputs();

	BOOL LoadRecordedVideo(HWND hwndOwner, CvCapture** capture);

	int videoX, videoY;
    int fps;
    long nFrames, framesAvailable;
    bool processingVideo, runningLive;
    IplImage *copyFrame, *outputFrame, *outputAccImage;
	Bitmap *bmpInput, *bmpOutput, *bmpMotion, *bmpGesture;

    //  keep track of the number of active filters that require motion or blob tracking
    int trackingMotion;
    int trackingGesture;

private:
    CvCapture *videoCapture;
    IplImage *currentFrame, *guessMask, *motionHistory;
    IplImage* motionBuf[MOTION_NUM_IMAGES];
    CvMemStorage *contourStorage;

    // functions that may be called by ProcessFrame (if motion/gesture filters are active)
    void ProcessMotionFrame();
    void ProcessGestureFrame();

    // for keeping track of position within circular motion history buffer
    int last;

	// for tracking optical flow for gesture tracking
	SimpleFlowTracker m_flowTracker;

    // list of classifiers to apply to live video stream
    list<Classifier*> activeClassifiers;

    // list of outputs to which we will send video data
    list<OutputSink*> activeOutputs;

	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
    CWindow *parent;
};
