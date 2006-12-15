#pragma once
#include "Classifier.h"

class GestureClassifier : public Classifier {
public:
    GestureClassifier();
    ~GestureClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, list<Rect>*);
    void ClassifyTrack(MotionTrack mt, list<Rect>*);
    void Save();
    void Load(LPCWSTR);

private:
    int nModels;
    int maxModelLength;
    TrajectoryModel** models;
};
