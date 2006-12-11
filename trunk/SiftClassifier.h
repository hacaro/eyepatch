#pragma once
#include "Classifier.h"

class SiftClassifier : public Classifier {
public:
    SiftClassifier();
    ~SiftClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
    IplImage *sampleCopy;
    int numSampleFeatures, numFeatureMatches;
    int sampleWidth, sampleHeight;
    struct feature* sampleFeatures;
};
