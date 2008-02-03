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
	Recognizer rec;

    int nTemplates;
    int maxTemplateLength;
};
