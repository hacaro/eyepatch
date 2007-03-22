#pragma once
#include "Classifier.h"

class BackgroundSubtraction : public Classifier {
public:
    BackgroundSubtraction();
    BackgroundSubtraction(LPCWSTR pathname);
    ~BackgroundSubtraction();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	void ClassifyFrame(IplImage*, IplImage*);
    void Save();

private:
    long frameNum;
    CvBGStatModel* bgmodel;
};
