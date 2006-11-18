#include "precomp.h"
#include "_cvhaartraining.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "HaarClassifier.h"


HaarClassifier::HaarClassifier() {
    isTrained = false;
    cascade = NULL;
    nStages = 4;
    storage = cvCreateMemStorage(0);
}

HaarClassifier::~HaarClassifier() {
    cvReleaseMemStorage(&storage);
    if (isTrained) cvReleaseHaarClassifierCascade(&cascade);
}

void HaarClassifier::Train(TrainingSet *sampleSet, HWND hwndProgress) {
    char tempPathname[MAX_PATH];
    char vecFilename[MAX_PATH];
    char negFilename[MAX_PATH];
    char imageFilename[MAX_PATH];
    char classifierPathname[MAX_PATH];
    char classifierName[MAX_PATH];

    GetTempPathA(MAX_PATH, tempPathname);
    sprintf_s(vecFilename, "%spossamples.vec", tempPathname);
    sprintf_s(negFilename, "%snegsamples.dat", tempPathname);
    int classifiernum = (int)time(0);
    sprintf_s(classifierPathname, "%sclassifier%d/", tempPathname, classifiernum);
    sprintf_s(classifierName, "%sclassifier%d", tempPathname, classifiernum);

    icvMkDir(vecFilename);
    icvMkDir(negFilename);
    FILE *vec = fopen(vecFilename, "wb");
    FILE *neglist = fopen(negFilename,"w");
    int imgNum=0;

    icvWriteVecHeader(vec, sampleSet->posSampleCount, SAMPLE_X, SAMPLE_Y);
   
    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) { // positive sample
            icvWriteVecSample(vec, sample->sampleImage);
        } else if (sample->iGroupId == 1) { // negative sample
            sprintf_s(imageFilename, "%sneg%d.jpg", tempPathname, imgNum);
            cvSaveImage(imageFilename, sample->fullImageCopy);
            fprintf(neglist, "neg%d.jpg\n", imgNum);
            imgNum++;
        }
    }
    fclose(vec);
    fclose(neglist);

    cvCreateCascadeClassifier(classifierPathname,  vecFilename, negFilename, 
        sampleSet->posSampleCount, sampleSet->negSampleCount, nStages, 0, 2, .99, .5, .95, 0, 1, 1, SAMPLE_X, SAMPLE_Y, 3, 0, hwndProgress);

    cascade = cvLoadHaarClassifierCascade(classifierName, cvSize(SAMPLE_X, SAMPLE_Y));
    isTrained = true;

}

int HaarClassifier::AddStage(TrainingSet *sampleSet, HWND hwndProgress) {
    if (!isTrained) return 0;
    nStages++;
    Train(sampleSet, hwndProgress);
    return nStages;
}

void HaarClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if (!cascade) return;

    // Clear the memory storage which was used before
    cvClearMemStorage( storage );

    // There can be more than one object in an image, so we create a growable sequence of objects
    // Detect the objects and store them in the sequence
    CvSeq* objects = cvHaarDetectObjects( frame, cascade, storage,
                                        1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
                                        cvSize(SAMPLE_X, SAMPLE_Y) );

    objList->clear();
    // Loop over the found objects
    for(int i = 0; i < (objects ? objects->total : 0); i++ )
    {
        Rect objRect;
        CvRect* r = (CvRect*)cvGetSeqElem(objects, i);

        objRect.X = r->x;
        objRect.Y = r->y;
        objRect.Width = r->width;
        objRect.Height = r->height;
        objList->push_back(objRect);
    }
}
