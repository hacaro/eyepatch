#pragma once

class Classifier
{
public:

	Classifier() {
	    isTrained = false;
        isOnDisk = false;
        classifierType = 0;
        filterImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
        applyImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
        filterBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);
        applyBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);

        // set the standard "friendly name"
        wcscpy(friendlyName, L"Generic Classifier");

        // create a directory to store this in
        WCHAR rootpath[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, rootpath);
        int classifiernum = (int)time(0);
        wsprintf(directoryName, L"%s\\%s\\%s%d", rootpath, APP_CLASS, FILE_CLASSIFIER_PREFIX, classifiernum);
    }
	virtual ~Classifier() {
        cvReleaseImage(&filterImage);
        cvReleaseImage(&applyImage);
        delete filterBitmap;
        delete applyBitmap;
    }

	virtual void StartTraining(TrainingSet*) = 0;
    virtual BOOL ContainsSufficientSamples(TrainingSet*) = 0;
	virtual void ClassifyFrame(IplImage*, list<Rect>*) = 0;
    virtual void Save() = 0;

    void DeleteFromDisk() {
        if (!isOnDisk) return;
        DeleteDirectory(directoryName, true);
        isOnDisk = false;
    }

    Bitmap* GetFilterImage() {
        return filterBitmap;
    }
    Bitmap* GetApplyImage() {
        return applyBitmap;
    }
    LPWSTR GetName() {
        return friendlyName;
    }
    void SetName(LPCWSTR newName) {
        wcscpy(friendlyName, newName);
    }

	bool isTrained;
    bool isOnDisk;
    int classifierType;

protected:
    Bitmap *filterBitmap, *applyBitmap;
    IplImage *filterImage, *applyImage;

    WCHAR friendlyName[MAX_PATH];
    WCHAR directoryName[MAX_PATH];
};
