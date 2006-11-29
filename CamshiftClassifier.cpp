#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "CamshiftClassifier.h"

CamshiftClassifier::CamshiftClassifier() :
	Classifier() {
	image = NULL;
	hsv = NULL;
	hue = NULL;
	mask = NULL;
	backproject = NULL;
	histimg = NULL;
	hist = NULL;

	hdims = 16;
	hranges_arr[0] = 0;	hranges_arr[1] = 180;
	hranges = hranges_arr;
	vmin = 10;
	vmax = 256;
	smin = 30;

	// allocate histogram
	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
}

CamshiftClassifier::~CamshiftClassifier() {
	// free histogram
	cvReleaseHist(&hist);
}

CvScalar CamshiftClassifier::hsv2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}

void CamshiftClassifier::StartTraining(TrainingSet *sampleSet) {

	// clear out the histogram
	cvClearHist(hist);

	// TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) { // positive sample
			// for now just take from first pos sample
			// this should really create a color histogram across all positive samples

			// allocate image buffers
			hsv = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 3 );
			hue = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 1 );
			mask = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 1 );

			// convert to hsv space
			cvCvtColor(sample->fullImageCopy, hsv, CV_BGR2HSV);

			// clip max and min range and split out hue channel
			cvInRangeS(hsv, cvScalar(0,smin,vmin,0),cvScalar(180,256,vmax,0), mask);
			cvSplit(hsv, hue, 0, 0, 0);

			// accumulate into hue histogram
			cvCalcHist(&hue, hist, 1, mask);

			// free image buffers
			cvReleaseImage(&hsv);
			cvReleaseImage(&hue);
			cvReleaseImage(&mask);

		} else if (sample->iGroupId == 1) { // negative sample
			// TODO: we could potentially subtract this from histogram
        }
    }

	// create histogram image
	float max_val = 0.f;
	cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
	cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
	histimg = cvCreateImage( cvSize(320,200), 8, 3 );
	cvZero( histimg );
	cvZero( histimg );
	int bin_w = histimg->width / hdims;
	for(int i = 0; i < hdims; i++)
	{
		int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
		CvScalar color = hsv2rgb(i*180.f/hdims);
		cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
			cvPoint((i+1)*bin_w,histimg->height - val),
			color, -1, 8, 0 );
	}

	// update member variables
	nPosSamples = sampleSet->posSampleCount;
	nNegSamples = sampleSet->negSampleCount;
	isTrained = true;
}

void CamshiftClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

    image = cvCreateImage( cvGetSize(frame), 8, 3 );
    hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
    hue = cvCreateImage( cvGetSize(frame), 8, 1 );
    mask = cvCreateImage( cvGetSize(frame), 8, 1 );
	backproject = cvCreateImage( cvGetSize(frame), 8, 1 );

    cvCopy( frame, image, 0 );
    cvCvtColor( image, hsv, CV_BGR2HSV );

	// create mask to clip out pixels outside of specified range
	cvInRangeS(hsv, cvScalar(0,smin,vmin,0), cvScalar(180,256,vmax,0), mask);
    cvSplit(hsv, hue, 0, 0, 0 );

	// create backprojection image and clip with mask
    cvCalcBackProject(&hue, backproject, hist);
    cvAnd(backproject, mask, backproject, 0);

	// find contours in backprojection image
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;
    cvFindContours( backproject, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// Loop over the found contours
	objList->clear();
	for (; contours != NULL; contours = contours->h_next)
	{
		if (fabs(cvContourArea(contours)) > COLOR_MIN_AREA) {
			Rect objRect;
			CvRect rect = cvBoundingRect(contours);
			objRect.X = rect.x;
			objRect.Y = rect.y;
			objRect.Width = rect.width;
			objRect.Height = rect.height;
			objList->push_back(objRect);
	}	
	}
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&image);
	cvReleaseImage(&hsv);
	cvReleaseImage(&hue);
	cvReleaseImage(&mask);
	cvReleaseImage(&backproject);
}
