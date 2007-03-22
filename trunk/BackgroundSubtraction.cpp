#include "precomp.h"
#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "BackgroundSubtraction.h"

BackgroundSubtraction::BackgroundSubtraction() :
	Classifier() {

    // set the "friendly name" and type
    wcscpy(friendlyName, L"Subtract Background");
    classifierType = IDC_STANDARD_CLASSIFIER;

    isTrained = false;
    isOnDisk = false;

    frameNum = 0;
    bgmodel = NULL;
}

BackgroundSubtraction::BackgroundSubtraction(LPCWSTR pathname) :
	Classifier() {

    // We will never load standard filters from disk, so this should never be called
    assert(FALSE);
    BackgroundSubtraction();
}

BackgroundSubtraction::~BackgroundSubtraction() {
}

BOOL BackgroundSubtraction::ContainsSufficientSamples(TrainingSet *sampleSet) {

    // Standard filters don't need training samples
    assert(FALSE);
    return true;
}

void BackgroundSubtraction::StartTraining(TrainingSet *sampleSet) {

    // Standard filters don't use training (this should never be called)
    assert(FALSE);
}

void BackgroundSubtraction::ClassifyFrame(IplImage *frame, IplImage* guessMask) {
    if(!frame) return;

    if (!isTrained) {
        // we need some more background frames before we have a decent background model
        if (frameNum < BACKGROUND_SUBTRACTION_DISCARD_FRAMES) {
            // throw away the first few frames (usually junk)
        } else if (frameNum == BACKGROUND_SUBTRACTION_DISCARD_FRAMES) {
            // initialize the background model on this frame
            bgmodel = cvCreateFGDStatModel(frame);
        } else {
            // update background model
            cvUpdateBGStatModel(frame, bgmodel);
        }
        frameNum++;

        // see if we're done training
        if (frameNum >= BACKGROUND_SUBTRACTION_DISCARD_FRAMES + BACKGROUND_SUBTRACTION_MIN_FRAMES) {
            isTrained = true;
        }
    } else {

        // update background model
        cvUpdateBGStatModel(frame, bgmodel);

        // Combine foreground mask with passed-in mask
        cvAnd(guessMask, bgmodel->foreground, guessMask);

        // close the foreground mask?
//        IplConvKernel *circElem = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);        
//        cvMorphologyEx(backproject, backproject, 0, circElem, CV_MOP_CLOSE, 1);  
    }
}


void BackgroundSubtraction::Save() {
    
    // We can't save built-in classifiers, so this should never be called
    assert(FALSE);
}
