#pragma once

class Classifier
{
public:

	Classifier() {
	    isTrained = false;
        isOnDisk = false;
        classifierType = 0;
		threshold = 0.5;
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

	Classifier(LPCWSTR pathname) {
		USES_CONVERSION;

		isTrained = true;
        isOnDisk = true;
        classifierType = 0;
		threshold = 0.5;

		filterImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
        applyImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
        filterBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);
        applyBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);

		// save the directory name for later
		wcscpy(directoryName, pathname);

		// load the "friendly name"
		WCHAR filename[MAX_PATH];
		wcscpy(filename, pathname);
		wcscat(filename, FILE_FRIENDLY_NAME);
		FILE *namefile = fopen(W2A(filename), "r");
		fgetws(friendlyName, MAX_PATH, namefile);
		fclose(namefile);

		// load the threshold
		wcscpy(filename, pathname);
		wcscat(filename, FILE_THRESHOLD_NAME);
		FILE *threshfile = fopen(W2A(filename), "r");
		fread(&threshold, sizeof(float), 1, threshfile);
		fclose(threshfile);
	}

	virtual ~Classifier() {
        cvReleaseImage(&filterImage);
        cvReleaseImage(&applyImage);
        delete filterBitmap;
        delete applyBitmap;
    }

	virtual void StartTraining(TrainingSet*) = 0;
    virtual BOOL ContainsSufficientSamples(TrainingSet*) = 0;
	virtual void ClassifyFrame(IplImage*, IplImage*) = 0;
	virtual void ResetRunningState() = 0;

	virtual void Save() {
		USES_CONVERSION;
		WCHAR filename[MAX_PATH];

		// make sure the directory exists 
	    SHCreateDirectory(NULL, directoryName);

		// save the "friendly name"
		wcscpy(filename,directoryName);
		wcscat(filename, FILE_FRIENDLY_NAME);
		FILE *namefile = fopen(W2A(filename), "w");
		fputws(friendlyName, namefile);
		fclose(namefile);

		// save the threshold
		wcscpy(filename,directoryName);
		wcscat(filename, FILE_THRESHOLD_NAME);
		FILE *threshfile  = fopen(W2A(filename), "w");
		fwrite(&threshold, sizeof(float), 1, threshfile);
		fclose(threshfile);

		isOnDisk = true;
	}

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
	float threshold;

protected:
    Bitmap *filterBitmap, *applyBitmap;
    IplImage *filterImage, *applyImage;

    WCHAR friendlyName[MAX_PATH];
    WCHAR directoryName[MAX_PATH];
};
