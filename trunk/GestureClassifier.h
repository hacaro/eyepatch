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

    // run classifier on an entire motion track and return TRUE if it matches a template
    bool ClassifyTrack(MotionTrack mt, IplImage*);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
	void UpdateTrajectoryImage();
	Recognizer rec;

    int nTemplates;
    int maxTemplateLength;
};
