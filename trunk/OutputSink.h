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

    virtual void OutputData(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName) = 0;

private:
    WCHAR friendlyName[MAX_PATH];
};
