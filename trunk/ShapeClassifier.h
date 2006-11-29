#pragma once
#include "Classifier.h"

class ShapeClassifier : public Classifier {
public:
    ShapeClassifier();
    ~ShapeClassifier();

	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);

private:
    CvMemStorage *templateStorage;
    CvSeq *templateContours;
};
