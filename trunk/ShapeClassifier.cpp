#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "ShapeClassifier.h"

ShapeClassifier::ShapeClassifier() :
	Classifier() {
    templateStorage = cvCreateMemStorage(0);

    // set the default "friendly name" and type
    wcscpy(friendlyName, L"Shape Classifier");
    classifierType = IDC_RADIO_SHAPE;        
}

ShapeClassifier::~ShapeClassifier() {
    cvReleaseMemStorage(&templateStorage);
}

BOOL ShapeClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return (sampleSet->posSampleCount > 0);
}

void ShapeClassifier::StartTraining(TrainingSet *sampleSet) {
    cvClearMemStorage(templateStorage);
    templateContours = NULL;
    cvZero(filterImage);

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample

            IplImage *grayscale = cvCreateImage( cvSize(sample->fullImageCopy->width, sample->fullImageCopy->height), IPL_DEPTH_8U, 1);
            cvCvtColor(sample->fullImageCopy, grayscale, CV_BGR2GRAY);
            cvCanny(grayscale, grayscale, 50, 200, 5);

            CvMemStorage *storage = cvCreateMemStorage(0);
            CvSeq *sampleContours = NULL;

            cvFindContours(grayscale, storage, &sampleContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);

            for (CvSeq *contour = sampleContours; contour != NULL; contour = contour->h_next)
	        {
		        if (fabs(cvArcLength(contour)) > SHAPE_MIN_LENGTH) {
                    if (!templateContours) {
                        templateContours = cvCloneSeq(contour, templateStorage);
                    } else {
                        CvSeq *newContour = cvCloneSeq(contour, templateStorage);
                        newContour->h_next = templateContours->h_next;
                        templateContours->h_next = newContour;
                    }
                }
	        }
            cvReleaseMemStorage(&storage);
            cvReleaseImage(&grayscale);

		} else if (sample->iGroupId == GROUPID_NEGSAMPLES) { // negative sample
            // do nothing for now
            // TODO: we could compare guesses against these as well and remove them if they match
        }
    }

    int contourNum = 0;
    for (CvSeq *contour = templateContours; contour != NULL; contour = contour->h_next) {
        // TODO: draw contours scaled in different places in image so they don't overlap
        cvDrawContours(filterImage, contour, colorSwatch[contourNum], CV_RGB(0,0,0), 0, 1, 8, cvPoint(0,0));
        contourNum = (contourNum+1) % COLOR_SWATCH_SIZE;
    }
    IplToBitmap(filterImage, filterBitmap);

	// update member variables
	isTrained = true;
}

void ShapeClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

    IplImage *copy = cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
    IplImage *grayscale = cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);

    cvCvtColor(frame, grayscale, CV_BGR2GRAY);
    cvCanny(grayscale, grayscale, 50, 200, 5);

    cvCvtColor(grayscale, copy, CV_GRAY2BGR);

    CvSeq *frameContours;
    CvMemStorage *storage = cvCreateMemStorage(0);
    cvFindContours(grayscale, storage, &frameContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);

	objList->clear();
    for (CvSeq *contour = frameContours; contour != NULL; contour = contour->h_next) {
        if (fabs(cvArcLength(contour)) > SHAPE_MIN_LENGTH) {

            int contourNum = 0;
            for (CvSeq *matchContour = templateContours; matchContour != NULL; matchContour = matchContour->h_next) {
                double similarity = cvMatchShapes(contour, matchContour, CV_CONTOURS_MATCH_I1, 0);
                if (similarity < SHAPE_SIMILARITY_THRESHOLD) {
                    cvDrawContours(copy, contour, colorSwatch[contourNum], CV_RGB(0,0,0), 0, 3, 8, cvPoint(0,0));
		            Rect objRect;
		            CvRect rect = cvBoundingRect(contour, 1);
		            objRect.X = rect.x;
		            objRect.Y = rect.y;
		            objRect.Width = rect.width;
		            objRect.Height = rect.height;
		            objList->push_back(objRect);
                }
                contourNum = (contourNum+1) % COLOR_SWATCH_SIZE;
            }
        }
    }
    cvResize(copy, applyImage);
    IplToBitmap(applyImage, applyBitmap);

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&copy);
    cvReleaseImage(&grayscale);
}

void ShapeClassifier::Save() {
    isSaved = true;
}
