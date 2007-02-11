#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "GestureClassifier.h"

GestureClassifier::GestureClassifier() :
	Classifier() {
    nModels = 0;
    maxModelLength = 0;
    models = NULL;

    // set the default "friendly name" and type
    wcscpy(friendlyName, L"Gesture Filter");
    classifierType = IDC_RADIO_GESTURE;

    // append identifier to directory name
    wcscat(directoryName, FILE_GESTURE_SUFFIX);
}

GestureClassifier::GestureClassifier(LPCWSTR pathname) :
	Classifier() {

	USES_CONVERSION;

    nModels = 0;
    maxModelLength = 0;
    models = NULL;

    // save the directory name for later
    wcscpy(directoryName, pathname);

    WCHAR filename[MAX_PATH];
    wcscpy(filename, pathname);
    wcscat(filename, FILE_DATA_NAME);

    // load the trajectories from the data file
    FILE *datafile = fopen(W2A(filename), "rb");
	fread(&nModels, sizeof(int), 1, datafile); 

    models = new TrajectoryModel*[nModels];
    int modelNum = 0;
	for(int i = 0; i < nModels; i++) {
		models[i] = new TrajectoryModel(datafile);
		maxModelLength = max(maxModelLength, models[i]->GetLength());
    }
    fclose(datafile);

    // load the "friendly name" and set the type
    wcscpy(filename, pathname);
    wcscat(filename, FILE_FRIENDLY_NAME);
    FILE *namefile = fopen(W2A(filename), "r");
    fgetws(friendlyName, MAX_PATH, namefile);
    fclose(namefile);
    classifierType = IDC_RADIO_GESTURE;

    isTrained = true;
    isOnDisk = true;

	UpdateTrajectoryImage();
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

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_RANGESAMPLES) { // gesture (range) sample
            models[modelNum] = new TrajectoryModel(sample->motionTrack);
            maxModelLength = max(maxModelLength, sample->motionTrack.size());
            modelNum++;
		}
    }
    if (isOnDisk) { // this classifier has been saved so we'll update the files
        Save();        
    }

    // update member variables
	isTrained = true;

	// update demo image
	UpdateTrajectoryImage();
}

BOOL GestureClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return (sampleSet->rangeSampleCount > 0);
}

void GestureClassifier::ClassifyFrame(IplImage *frame, IplImage* guessMask) {
    // not implemented: this class uses ClassifyTrack instead
    assert(false);
}    

void GestureClassifier::ClassifyTrack(MotionTrack mt, IplImage* guessMask) {
    IplImage *newMask = cvCloneImage(guessMask);
    cvZero(newMask);
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

            // draw a rectangle in the new mask image
            CvPoint topLeft = cvPoint(ms.x - ms.sizex/2, ms.y - ms.sizey/2);
            CvPoint bottomRight = cvPoint(ms.x + ms.sizex/2, ms.y + ms.sizey/2);
            cvRectangle(newMask, topLeft, bottomRight, cvScalar(0xFF), CV_FILLED, 8); 
        }
    }
    cvLine(applyImage,cvPoint(0,startY),cvPoint(applyImage->width,startY),CV_RGB(255,255,255),1);
 
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX_SMALL, 0.55,0.6,0,1, CV_AA);
	cvPutText (applyImage,"Gesture Completion Probabilities",cvPoint(7,20), &font, cvScalar(255,255,255));

    // Combine old and new mask
    // TODO: support OR operation as well
    cvAnd(guessMask, newMask, guessMask);

    IplToBitmap(applyImage, applyBitmap);
	cvReleaseImage(&newMask);
}

void GestureClassifier::UpdateTrajectoryImage() {
    cvZero(filterImage);
	if (nModels < 1) return;

	for (int i=0; i<nModels; i++) {
        DrawTrack(filterImage, models[i], colorSwatch[i % COLOR_SWATCH_SIZE], 3);
	}

    // update demo image
    IplToBitmap(filterImage, filterBitmap);
}

void GestureClassifier::Save() {
    USES_CONVERSION;
    WCHAR filename[MAX_PATH];

    SHCreateDirectory(NULL, directoryName);
    // save the trajectory data
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_DATA_NAME);
    FILE *datafile = fopen(W2A(filename), "wb");

	// write out the number of models
	fwrite(&nModels, sizeof(int), 1, datafile); 

	// write out all the trajectory models
	for(int i = 0; i < nModels; i++) {
		models[i]->WriteToFile(datafile);
    }
    fclose(datafile);

    // save the "friendly name"
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_FRIENDLY_NAME);
    FILE *namefile = fopen(W2A(filename), "w");
    fputws(friendlyName, namefile);
    fclose(namefile);

    isOnDisk = true;
}
