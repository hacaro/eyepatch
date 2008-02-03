#pragma once
#include "Classifier.h"

class BrightnessClassifier : public Classifier {
public:
    BrightnessClassifier();
    BrightnessClassifier(LPCWSTR pathname);
    ~BrightnessClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, IplImage*);
    void Save();

private:
    void UpdateHistogramImage();

	CvHistogram *hist;
    int hdims;
	float avg_level;  // the average brightness value (we threshold on this)
};
