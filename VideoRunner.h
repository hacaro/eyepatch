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

	int videoX, videoY;
    int fps;
    long nFrames;
    bool processingVideo;
    IplImage *copyFrame, *outputFrame;
    Bitmap *bmpInput, *bmpOutput;

    list<Classifier*> customClassifiers;

private:
    CvCapture *videoCapture;
    IplImage *currentFrame, *guessMask, *motionHistory;
    IplImage* motionBuf[MOTION_NUM_IMAGES];

    // for keeping track of position within circular motion history buffer
    int last;

	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
    CWindow *parent;
};
