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
    int nFrames;
    bool processingVideo;
    IplImage *copyFrame, *outputFrame;
    Bitmap *bmpInput, *bmpOutput;

    list<Classifier*> customClassifiers;

private:
    CvCapture *videoCapture;
    IplImage *currentFrame, *guessMask;

	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
    CWindow *parent;
};
