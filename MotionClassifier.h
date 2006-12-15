#pragma once
#include "Classifier.h"

class MotionClassifier : public Classifier {
public:
    MotionClassifier();
    ~MotionClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void Save();
    void Load(LPCWSTR);

private:
    list<double> motionAngles;
};
