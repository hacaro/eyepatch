#pragma once
#include "Classifier.h"

class GestureClassifier : public Classifier {
public:
    GestureClassifier();
	GestureClassifier(LPCWSTR pathname);
    ~GestureClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void ClassifyTrack(MotionTrack mt, list<Rect>*);
    void Save();

private:
	void UpdateTrajectoryImage();

    int nModels;
    int maxModelLength;
    TrajectoryModel** models;
};
