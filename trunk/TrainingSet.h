#pragma once

class TrainingSet
{
public:
    HIMAGELIST m_imageList;
    map<UINT, TrainingSample*> sampleMap;
    int posSampleCount, negSampleCount;

    TrainingSet(void);
    ~TrainingSet(void);
    HIMAGELIST GetImageList();
    void AddSample(TrainingSample *sample);
    void SetSampleGroup(UINT sampleId, int groupId);
    void RemoveSample(UINT sampleId);

    // debugging function that draws the samples to a window
    void ShowSamples();

private:

};
