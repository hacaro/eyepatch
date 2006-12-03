#pragma once
#include "Classifier.h"

class MotionClassifier : public Classifier {
public:
    MotionClassifier();
    ~MotionClassifier();

	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
};
