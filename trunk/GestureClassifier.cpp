#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "GestureClassifier.h"

GestureClassifier::GestureClassifier() :
	Classifier() {
    nModels = 0;
    maxModelLength = 0;
    models = NULL;
}

GestureClassifier::~GestureClassifier() {
    if (isTrained) { // delete the models
        for (int i=0; i<nModels; i++) {
            delete models[i];
        }
        delete[] models;
    }
}

void GestureClassifier::StartTraining(TrainingSet *sampleSet) {
    if (isTrained) { // delete the old models
        for (int i=0; i<nModels; i++) {
            delete models[i];
        }
        delete[] models;
    }
    maxModelLength = 0;

    nModels = sampleSet->rangeSampleCount;
    models = new TrajectoryModel*[nModels];
    int modelNum = 0;
    cvZero(filterImage);
    IplImage *resizedGestureImage = cvCloneImage(filterImage);

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_RANGESAMPLES) { // gesture (range) sample
            IplImage *gestureImage = cvCreateImage(cvSize(sample->fullImageCopy->width, sample->fullImageCopy->height), IPL_DEPTH_8U, 3);
            cvZero(gestureImage);
            DrawTrack(gestureImage, sample->motionTrack, colorSwatch[modelNum % COLOR_SWATCH_SIZE], 3);
            cvResize(gestureImage, resizedGestureImage);
            cvAdd(filterImage, resizedGestureImage, filterImage);
            models[modelNum] = new TrajectoryModel(sample->motionTrack);
            maxModelLength = max(maxModelLength, sample->motionTrack.size());
            modelNum++;
		}
    }

    // update demo image
    IplToBitmap(filterImage, filterBitmap);
    cvReleaseImage(&resizedGestureImage);

    // update member variables
	isTrained = true;
}

BOOL GestureClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return (sampleSet->rangeSampleCount > 0);
}

void GestureClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;
    objList->clear();
}    

void GestureClassifier::ClassifyTrack(MotionTrack mt, list<Rect>* objList) {

    objList->clear();
    CondensationSampleSet condensSampleSet(GESTURE_NUM_CONDENSATION_SAMPLES, models, nModels);

    // don't start all the way at the beginning of the track if it's really long
    int startFrame = max(0, mt.size()-RHO_MAX*((double)maxModelLength));

    MotionSample ms;
    for (int i = startFrame; i<mt.size(); i++) {
        // update probabilities based on this sample point
        ms = mt[i];
        condensSampleSet.Update(ms.vx, ms.vy, ms.sizex, ms.sizey);
    }

    cvZero(applyImage);
    int barWidth = applyImage->width/(nModels*2+1);
    double maxY = (double)applyImage->height - 40.0;
    int startY = applyImage->height-10;

    for (int modelNum=0; modelNum<nModels; modelNum++) {
        double probability = condensSampleSet.GetModelProbability(modelNum);
        double completionProb = condensSampleSet.GetModelCompletionProbability(modelNum);

        CvPoint tl = cvPoint(barWidth+(modelNum*2)*barWidth, (int)(startY-5-completionProb*maxY));
        CvPoint br = cvPoint(barWidth+(modelNum*2+1)*barWidth, startY);
        cvRectangle(applyImage, tl, br, colorSwatch[modelNum % COLOR_SWATCH_SIZE], -1, CV_AA);

        if (completionProb>0.1) {
            Rect objRect;
            objRect.X = ms.x - ms.sizex/2;
            objRect.Y = ms.y - ms.sizey/2;
            objRect.Width = ms.sizex;
            objRect.Height = ms.sizey;
            objList->push_back(objRect);
        }
    }
    cvLine(applyImage,cvPoint(0,startY),cvPoint(applyImage->width,startY),CV_RGB(255,255,255),1);
 
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX_SMALL, 0.55,0.6,0,1, CV_AA);
	cvPutText (applyImage,"Gesture Completion Probabilities",cvPoint(7,20), &font, cvScalar(255,255,255));

    IplToBitmap(applyImage, applyBitmap);
}