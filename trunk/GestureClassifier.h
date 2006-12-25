#pragma once
#include "Classifier.h"

class GestureClassifier : public Classifier {
public:
    GestureClassifier();
	GestureClassifier(LPCWSTR pathname);
    ~GestureClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, IplImage*);
    void ClassifyTrack(MotionTrack mt, IplImage*);
    void Save();

private:
	void UpdateTrajectoryImage();

    int nModels;
    int maxModelLength;
    TrajectoryModel** models;
};
