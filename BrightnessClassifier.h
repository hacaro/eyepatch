#pragma once
#include "Classifier.h"

class BrightnessClassifier : public Classifier {
public:
    BrightnessClassifier();
    BrightnessClassifier(LPCWSTR pathname);
    ~BrightnessClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void Save();

private:
    void UpdateHistogramImage();

	CvHistogram *hist;
    int hdims;
};
