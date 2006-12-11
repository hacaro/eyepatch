#pragma once
#include "Classifier.h"

class GestureClassifier : public Classifier {
public:
    GestureClassifier();
    ~GestureClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
//    list<double> motionAngles;
};
