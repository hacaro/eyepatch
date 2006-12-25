#pragma once
#include "Classifier.h"

class ColorClassifier : public Classifier {
public:
    ColorClassifier();
    ColorClassifier(LPCWSTR pathname);
    ~ColorClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, IplImage*);
    void Save();

private:
    void UpdateHistogramImage();

    CvHistogram *hist;
	int hdims;
};
