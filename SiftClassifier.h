#pragma once
#include "Classifier.h"

class SiftClassifier : public Classifier {
public:
    SiftClassifier();
    SiftClassifier(LPCWSTR pathname);
    ~SiftClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void Save();

private:
    void UpdateSiftImage();
    IplImage *sampleCopy;
    int numSampleFeatures, numFeatureMatches;
    int sampleWidth, sampleHeight;
    struct feature* sampleFeatures;
};
