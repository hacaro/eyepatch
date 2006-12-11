#pragma once
#include "Classifier.h"

class BrightnessClassifier : public Classifier {
public:
    BrightnessClassifier();
    ~BrightnessClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
	CvHistogram *hist;
    int hdims;
};
