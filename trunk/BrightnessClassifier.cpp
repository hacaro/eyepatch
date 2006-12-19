#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "BrightnessClassifier.h"

BrightnessClassifier::BrightnessClassifier() :
	Classifier() {

	// allocate histogram
	hdims = 16;
	float hranges_arr[2];
	hranges_arr[0] = 0;	hranges_arr[1] = 255;
	float* hranges = hranges_arr;
	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );

    // set the default "friendly name" and type
    wcscpy(friendlyName, L"Brightness Classifier");
    classifierType = IDC_RADIO_BRIGHTNESS;
    
    // append identifier to directory name
    wcscat(directoryName, FILE_BRIGHTNESS_SUFFIX);
}

BrightnessClassifier::BrightnessClassifier(LPCWSTR pathname) {
    USES_CONVERSION;

	// allocate histogram
	hdims = 16;
	float hranges_arr[2];
	hranges_arr[0] = 0;	hranges_arr[1] = 255;
	float* hranges = hranges_arr;
	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );

    // save the directory name for later
    wcscpy(directoryName, pathname);

    WCHAR filename[MAX_PATH];
    wcscpy(filename, pathname);
    wcscat(filename, FILE_DATA_NAME);

    // load the data from the histogram file
    FILE *datafile = fopen(W2A(filename), "rb");
    for(int i = 0; i < hdims; i++) {
        float val;
        fread(&val, sizeof(float), 1, datafile);
		cvSetReal1D(hist->bins,i,val);
    }
    fclose(datafile);

    // load the "friendly name" and set the type
    wcscpy(filename, pathname);
    wcscat(filename, FILE_FRIENDLY_NAME);
    FILE *namefile = fopen(W2A(filename), "r");
    fgetws(friendlyName, MAX_PATH, namefile);
    fclose(namefile);
    classifierType = IDC_RADIO_BRIGHTNESS;

    UpdateHistogramImage();
    isTrained = true;
    isOnDisk = true;
}

BrightnessClassifier::~BrightnessClassifier() {
	// free histogram
	cvReleaseHist(&hist);
}

BOOL BrightnessClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return (sampleSet->posSampleCount > 0);
}

void BrightnessClassifier::StartTraining(TrainingSet *sampleSet) {

	// clear out the histogram
	cvClearHist(hist);

	// TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample

			// create grayscale copy
			IplImage *brightness = cvCreateImage( cvGetSize(sample->fullImageCopy), IPL_DEPTH_8U, 1);
            cvCvtColor(sample->fullImageCopy, brightness, CV_BGR2GRAY);

			// accumulate into hue histogram
			cvCalcHist(&brightness, hist, 1);

			// free grayscale copy
			cvReleaseImage(&brightness);

		} else if (sample->iGroupId == GROUPID_NEGSAMPLES) { // negative sample
			// TODO: we could potentially subtract this from histogram
        }
    }

    UpdateHistogramImage();

    if (isOnDisk) { // this classifier has been saved so we'll update the files
        Save();        
    }

    // update member variables
	isTrained = true;
}

void BrightnessClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

    IplImage *image = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 3 );
    IplImage *brightness = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 );
	IplImage *backproject = cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 );

    cvCopy(frame, image, 0);
    cvCvtColor(image, brightness, CV_BGR2GRAY);

	// create backprojection image
    cvCalcBackProject(&brightness, backproject, hist);

    // copy back projection into demo image
    cvCvtColor(backproject, image, CV_GRAY2BGR);

	// find contours in backprojection image
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;
    cvFindContours( backproject, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// Loop over the found contours
	objList->clear();
	for (; contours != NULL; contours = contours->h_next)
	{
        double contourArea = fabs(cvContourArea(contours));
		if ((contourArea > COLOR_MIN_AREA) && (contourArea < COLOR_MAX_AREA)) {
			Rect objRect;
			CvRect rect = cvBoundingRect(contours);
			objRect.X = rect.x;
			objRect.Y = rect.y;
			objRect.Width = rect.width;
			objRect.Height = rect.height;
			objList->push_back(objRect);

            // draw contour in demo image
            cvDrawContours(image, contours, CV_RGB(0,255,255), CV_RGB(0,255,255), 0, 2, 8);
        }
	}

    // update bitmap demo image
    cvResize(image, applyImage);
    IplToBitmap(applyImage, applyBitmap);

	cvReleaseMemStorage(&storage);

	cvReleaseImage(&image);
	cvReleaseImage(&brightness);
	cvReleaseImage(&backproject);
}

void BrightnessClassifier::UpdateHistogramImage() {

    // create histogram image
	IplImage *histimg = cvCreateImage(cvSize(320,200), 8, 3);
	float max_val = 0.f;
	cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0 );
	cvConvertScale(hist->bins, hist->bins, max_val ? 255./max_val : 0., 0);
	cvSet(histimg, CV_RGB(220,220,240));
	int bin_w = histimg->width / hdims;
	for(int i = 0; i < hdims; i++)
	{
		int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
		CvScalar color = CV_RGB(i*255.f/hdims,i*255.f/hdims,i*255.f/hdims);
		cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
			cvPoint((i+1)*bin_w,histimg->height - val),
			color, -1, 8, 0 );
		cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
			cvPoint((i+1)*bin_w,histimg->height - val),
			CV_RGB(0,0,0), 1, 8, 0 );
	}
    cvResize(histimg, filterImage);
    IplToBitmap(filterImage, filterBitmap);
    cvReleaseImage(&histimg);
}

void BrightnessClassifier::Save() {
    USES_CONVERSION;
    WCHAR filename[MAX_PATH];

    SHCreateDirectory(NULL, directoryName);
    // save the histogram data
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_DATA_NAME);
    FILE *datafile = fopen(W2A(filename), "wb");
	for(int i = 0; i < hdims; i++) {
		float val = cvGetReal1D(hist->bins,i);
        fwrite(&val, sizeof(float), 1, datafile);
	}
    fclose(datafile);

    // save the "friendly name"
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_FRIENDLY_NAME);
    FILE *namefile = fopen(W2A(filename), "w");
    fputws(friendlyName, namefile);
    fclose(namefile);

    isOnDisk = true;
}
