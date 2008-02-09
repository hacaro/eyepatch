#pragma once
#include "Classifier.h"

class SiftClassifier : public Classifier {
public:
    SiftClassifier();
    SiftClassifier(LPCWSTR pathname);
    ~SiftClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, IplImage*);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
    void UpdateSiftImage();
    IplImage *sampleCopy;
    int numSampleFeatures, numFeatureMatches;
    int sampleWidth, sampleHeight;
    struct feature* sampleFeatures;
};
