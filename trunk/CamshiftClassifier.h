#pragma once
#include "Classifier.h"

class CamshiftClassifier : public Classifier {
public:
    CamshiftClassifier();
    ~CamshiftClassifier();

	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
	CvScalar hsv2rgb(float);

	IplImage *image, *hsv, *hue, *mask, *backproject, *histimg;
	CvHistogram *hist;

	int hdims;
	float hranges_arr[2];
	float* hranges;
	int vmin, vmax, smin;
};
