#pragma once
#include "Classifier.h"

class MotionClassifier : public Classifier {
public:
    MotionClassifier();
    MotionClassifier(LPCWSTR pathname);
    ~MotionClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void Save();

private:
    list<double> motionAngles;
};
