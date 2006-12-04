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
    TrainingSample *sample = sampleMap[sampleId];

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
