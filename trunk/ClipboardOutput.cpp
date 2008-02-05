#include "precomp.h"
#include "constants.h"

#include "OutputSink.h"
#include "ClipboardOutput.h"

ClipboardOutput::ClipboardOutput() :
    OutputSink() {
    SetName(L"Frames to Windows Clipboard");
}

ClipboardOutput::~ClipboardOutput() {
}

void ClipboardOutput::ProcessInput(IplImage *image) {
	CopyImageToClipboard(image);
}

void ClipboardOutput::ProcessOutput(IplImage *image, IplImage *mask, CvSeq* contours, char *filterName) {
}
