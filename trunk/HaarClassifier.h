#pragma once

class HaarClassifier
{
public:
    HaarClassifier();
    ~HaarClassifier(void);
    void Train(TrainingSet*, HWND);
    int AddStage(TrainingSet*, HWND);
    void ClassifyFrame(IplImage*, list<Rect>*);
    bool isTrained;

private:
    CvHaarClassifierCascade* cascade;
    CvMemStorage* storage;
    int nStages;
};
