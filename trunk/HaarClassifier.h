#pragma once

class HaarClassifier
{
public:
    HaarClassifier();
    ~HaarClassifier(void);
    void Train(TrainingSet*);
    int AddStage(TrainingSet*);
    void ClassifyFrame(IplImage*, list<Rect>*);
    bool isTrained;

private:
    CvHaarClassifierCascade* cascade;
    CvMemStorage* storage;
    int nStages;
};
