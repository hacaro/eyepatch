#pragma once
#include "Classifier.h"

class ColorClassifier : public Classifier {
public:
    ColorClassifier();
    ~ColorClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void Save();

private:
	IplImage *image, *hsv, *hue, *mask, *backproject, *histimg;
	CvHistogram *hist;

	int hdims;
	float hranges_arr[2];
	float* hranges;
};
