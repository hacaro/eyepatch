#pragma once
#include "OutputSink.h"

class ClipboardOutput : public OutputSink {
public:
    ClipboardOutput();
    ~ClipboardOutput();
    void ProcessInput(IplImage* image);
	void ProcessOutput(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName);
};
