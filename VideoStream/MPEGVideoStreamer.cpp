#include "precomp.h"
#include "MPEGVideoStreamer.h"

DWORD WINAPI MPEGVideoStreamer::ThreadCallback(LPVOID img) {
	IplImage *frame = (IplImage*) img;
	ULONG startTime = GetTickCount();
	ULONG framesSent = 0;

	CvVideoWriter *writer = cvCreateStreamingVideoWriter("rtp://127.0.0.1:10000?localport=10001", CV_FOURCC('M','P','G','2'), 24, cvSize(176, 144));
	while(1) {
		ULONG timeElapsed = GetTickCount()-startTime;
		ULONG targetFramesSent = (ULONG)(24.0*timeElapsed)/1000.0;
		if (framesSent < targetFramesSent) {
			cvWriteStreamingFrame(writer, frame);
			framesSent++;
		}
	}
	cvReleaseVideoWriter(&writer);
	cvReleaseImage(&frame);
}

MPEGVideoStreamer::MPEGVideoStreamer() {
	streaming = false;
	frame = cvCreateImage(cvSize(176,144), 8, 3);
	cvZero(frame);
}

MPEGVideoStreamer::~MPEGVideoStreamer() {
	cvReleaseImage(&frame);
}

void MPEGVideoStreamer::StartStreaming() {
	streaming = true;
	m_hThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
}

void MPEGVideoStreamer::UpdateFrame(IplImage *newFrame) {
	cvResize(newFrame, frame);
}

void MPEGVideoStreamer::StopStreaming() {
	TerminateThread(m_hThread, 0);
	streaming = false;
}
