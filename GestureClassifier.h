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

    // run classifier on an entire motion track
    void ClassifyTrack(MotionTrack mt, IplImage*);

    // update model one sample at a time (for running live, frame-by-frame)
    void UpdateRunningModel(vector<MotionTrack> *trackList);
    void ResetRunningModel();
    void GetMaskFromRunningModel(IplImage*);

    void Save();

private:
	void UpdateTrajectoryImage();

    int nModels;
    int maxModelLength;
    TrajectoryModel** models;

    // TODO: rather than limiting to three simultaneous tracks, it would be better
    // to place this ConDens updating code in the blob tracker output model
    // (by making a modified version of TrajectoryList)
    CondensationSampleSet* activeCondens[GESTURE_MAX_SIMULTANEOUS_TRACKS];
    MotionSample lastSample[GESTURE_MAX_SIMULTANEOUS_TRACKS];
};
