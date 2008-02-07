#pragma once
#include "OutputSink.h"
#include "MPEGVideoStreamer.h"

class StreamingVideoOutput : public OutputSink {
public:
    StreamingVideoOutput();
    ~StreamingVideoOutput();
    void ProcessInput(IplImage* image);
	void ProcessOutput(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName);
	void StartRunning();
	void StopRunning();
private:
	MPEGVideoStreamer streamer;
};
