#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"

TrainingSet::TrainingSet(void) {
    posSampleCount = 0;
    negSampleCount = 0;
}

TrainingSet::~TrainingSet(void) {
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        delete sample;
    }
    sampleMap.clear();
}

HIMAGELIST TrainingSet::GetImageList() {
    // TODO: store and maintain imagelist here instead of in videomarkup?
    return NULL;
}

void TrainingSet::AddSample(TrainingSample *sample) {
    if (sample->iGroupId == 0) posSampleCount++;
    else if (sample->iGroupId == 1) negSampleCount++;
    sampleMap[sample->id] = sample;
}

void  TrainingSet::SetSampleGroup(UINT sampleId, int groupId) {
    map<UINT, TrainingSample*>::iterator i = sampleMap.find(sampleId);
    if (i != sampleMap.end()) {
        TrainingSample *sample = i->second;

        // reduce count of old group id
        int oldGroupId = sample->iGroupId;
        if (oldGroupId == 0) posSampleCount--;
        else if (oldGroupId == 1) negSampleCount--;

        // increase count of new group id
        if (groupId == 0) posSampleCount++;
        else if (groupId == 1) negSampleCount++;

        // set new group id
        sample->iGroupId = groupId;
    }
}

void TrainingSet::RemoveSample(UINT sampleId) {
    map<UINT, TrainingSample*>::iterator i = sampleMap.find(sampleId);
    if (i != sampleMap.end()) {
        TrainingSample *sample = i->second;
        delete sample;
        sampleMap.erase(i);
    }
}

void TrainingSet::ShowSamples() {
    cvNamedWindow("Samples");

    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 0.7, 0.7, 0, 1, 8);

    // draw the positive samples
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) {
            IplImage *copy = cvCloneImage(sample->fullImageCopy);
            cvPutText(copy, "Positive Sample", cvPoint(10,10), &font, CV_RGB(0,255,0));
            cvShowImage("Samples", copy);
            cvReleaseImage(&copy);
            cvWaitKey(0);
        }
    }

    // draw the negative samples
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 1) {
            IplImage *copy = cvCloneImage(sample->fullImageCopy);
            cvPutText(copy, "Negative Sample", cvPoint(10,10), &font, CV_RGB(255,0,0));
            cvShowImage("Samples", copy);
            cvReleaseImage(&copy);
            cvWaitKey(0);
        }
    }

    // draw the trash
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 2) {
            IplImage *copy = cvCloneImage(sample->fullImageCopy);
            cvPutText(copy, "Trash", cvPoint(10,10), &font, CV_RGB(150,150,150));
            cvShowImage("Samples", copy);
            cvReleaseImage(&copy);
            cvWaitKey(0);
        }
    }

    cvDestroyWindow("Samples");
}