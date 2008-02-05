#pragma once

class OutputSink {
public:
    OutputSink() { }
    virtual ~OutputSink() { }

    LPWSTR GetName() {
        return friendlyName;
    }
    void SetName(LPCWSTR newName) {
        wcscpy(friendlyName, newName);
    }

    virtual void ProcessInput(IplImage* image) = 0;
    virtual void ProcessOutput(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName) = 0;

private:
    WCHAR friendlyName[MAX_PATH];
};
