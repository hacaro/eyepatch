#pragma once

class CFilterComposer;

class CVideoRunner
{
public: 
	CVideoRunner(CWindow *caller);
	~CVideoRunner();

    static DWORD WINAPI ThreadCallback(CVideoRunner*);
    void StartProcessing();
    void StopProcessing();
	void ProcessFrame();

    void AddActiveFilter(Classifier*);
    void ClearActiveFilters();

	int videoX, videoY;
    int fps;
    long nFrames;
    bool processingVideo;
    IplImage *copyFrame, *outputFrame;
    Bitmap *bmpInput, *bmpOutput;

private:
    CvCapture *videoCapture;
    IplImage *currentFrame, *guessMask, *motionHistory;
    IplImage* motionBuf[MOTION_NUM_IMAGES];

    // for keeping track of position within circular motion history buffer
    int last;

    // list of classifiers to apply to live video stream
    list<Classifier*> activeClassifiers;

	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
    CWindow *parent;
};
