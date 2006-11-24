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
    else negSampleCount++;
    sampleMap[sample->id] = sample;
}

void  TrainingSet::SetSampleGroup(UINT sampleId, int groupId) {
    sampleMap[sampleId]->iGroupId = groupId;
}
