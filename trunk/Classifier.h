#pragma once

class Classifier
{
public:

	Classifier() {
	    isTrained = false;
		nPosSamples = 0;
		nNegSamples = 0;
        filterImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
        applyImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
        filterBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);
        applyBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);
	}
	virtual ~Classifier() {
        cvReleaseImage(&filterImage);
        cvReleaseImage(&applyImage);
        delete filterBitmap;
        delete applyBitmap;
    }

	virtual void StartTraining(TrainingSet*) = 0;
	virtual void ClassifyFrame(IplImage*, list<Rect>*) = 0;
    Bitmap* GetFilterImage() {
        return filterBitmap;
    }
    Bitmap* GetApplyImage() {
        return applyBitmap;
    }

	bool isTrained;

protected:

    int nPosSamples, nNegSamples;

    Bitmap *filterBitmap, *applyBitmap;
    IplImage *filterImage, *applyImage;

};