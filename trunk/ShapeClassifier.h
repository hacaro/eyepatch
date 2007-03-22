#pragma once
#include "Classifier.h"

class ShapeClassifier : public Classifier {
public:
    ShapeClassifier();
    ShapeClassifier(LPCWSTR pathname);
    ~ShapeClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, IplImage*);
    void Save();

private:
    void UpdateContourImage();

    CvMemStorage *templateStorage;
    CvSeq *templateContours;
};