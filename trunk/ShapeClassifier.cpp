#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "ShapeClassifier.h"

ShapeClassifier::ShapeClassifier() :
	Classifier() {
    templateStorage = cvCreateMemStorage(0);

    cvNamedWindow("ShowGrayscale");
    cvNamedWindow("ShowContours");
    cvNamedWindow("ShowResults");

}

ShapeClassifier::~ShapeClassifier() {
    cvReleaseMemStorage(&templateStorage);
}


void ShapeClassifier::StartTraining(TrainingSet *sampleSet) {
	// TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) { // positive sample

            // for now just use first positive sample
            // TODO: get contours for all samples and match against each one

            IplImage *copy = cvCreateImage( cvSize(sample->fullImageCopy->width, sample->fullImageCopy->height), IPL_DEPTH_8U, 3);
            IplImage *grayscale = cvCreateImage( cvSize(sample->fullImageCopy->width, sample->fullImageCopy->height), IPL_DEPTH_8U, 1);
            cvCopy(sample->fullImageCopy, copy);
            cvCvtColor(sample->fullImageCopy, grayscale, CV_BGR2GRAY);
            cvCanny(grayscale, grayscale, 50, 200, 5);

            cvClearMemStorage(templateStorage);
            cvFindContours(grayscale, templateStorage, &templateContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);

	        for (CvSeq *contour = templateContours; contour != NULL; contour = contour->h_next)
	        {
		        if (fabs(cvArcLength(contour)) > SHAPE_MIN_LENGTH) {
                    cvDrawContours(copy, contour, CV_RGB(0,255,255), CV_RGB(255,0,0), 0, 1, 8, cvPoint(0,0));
                }
	        }

            cvShowImage("ShowGrayscale",grayscale);
            cvShowImage("ShowContours",copy);

            cvReleaseImage(&copy);
            cvReleaseImage(&grayscale);
            break;
		} else if (sample->iGroupId == 1) { // negative sample

        }
    }

	// update member variables
	nPosSamples = sampleSet->posSampleCount;
	nNegSamples = sampleSet->negSampleCount;
	isTrained = true;
}

void ShapeClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

    
    IplImage *copy = cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
    IplImage *grayscale = cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);
    cvCopy(frame, copy);
    cvCvtColor(frame, grayscale, CV_BGR2GRAY);
    cvCanny(grayscale, grayscale, 50, 200, 5);

    CvSeq *frameContours;
    CvMemStorage *storage = cvCreateMemStorage(0);
    cvFindContours(grayscale, storage, &frameContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);

    for (CvSeq *contour = frameContours; contour != NULL; contour = contour->h_next) {
        if (fabs(cvArcLength(contour)) > SHAPE_MIN_LENGTH) {
            for (CvSeq *matchContour = templateContours; matchContour != NULL; matchContour = matchContour->h_next) {
                if (fabs(cvArcLength(matchContour)) > SHAPE_MIN_LENGTH) {
                    double similarity = cvMatchShapes(contour, matchContour, CV_CONTOURS_MATCH_I1, 0);
                    if (similarity < SHAPE_SIMILARITY_THRESHOLD) {
                        cvDrawContours(copy, contour, CV_RGB(0,255,255), CV_RGB(255,0,0), 0, 1, 8, cvPoint(0,0));
                    }
                }
            }
        }
    }
    
    cvShowImage("ShowResults", copy);

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&copy);
    cvReleaseImage(&grayscale);
/*

	// find contours in backprojection image
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;
    cvFindContours( backproject, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// Loop over the found contours
	objList->clear();
	for (; contours != NULL; contours = contours->h_next)
	{
		if (fabs(cvContourArea(contours)) > COLOR_MIN_AREA) {
			Rect objRect;
			CvRect rect = cvBoundingRect(contours);
			objRect.X = rect.x;
			objRect.Y = rect.y;
			objRect.Width = rect.width;
			objRect.Height = rect.height;
			objList->push_back(objRect);
    	}	
	}
	cvReleaseMemStorage(&storage);
*/
}
