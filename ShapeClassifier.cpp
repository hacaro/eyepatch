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
    wcscpy(friendlyName, L"Shape Filter");
    classifierType = SHAPE_FILTER;        

    // append identifier to directory name
    wcscat(directoryName, FILE_SHAPE_SUFFIX);
}

ShapeClassifier::ShapeClassifier(LPCWSTR pathname) :
	Classifier(pathname) {

	USES_CONVERSION;
    templateStorage = cvCreateMemStorage(0);

    WCHAR filename[MAX_PATH];
    wcscpy(filename, pathname);
    wcscat(filename, FILE_CONTOUR_NAME);

    // load the contours from the data file
    templateContours = (CvSeq*)cvLoad(W2A(filename), templateStorage, 0, 0);

	// set the type
	classifierType = SHAPE_FILTER;

    UpdateContourImage();
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

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample

            IplImage *grayscale = cvCreateImage( cvSize(sample->fullImageCopy->width, sample->fullImageCopy->height), IPL_DEPTH_8U, 1);
            cvCvtColor(sample->fullImageCopy, grayscale, CV_BGR2GRAY);
            cvCanny(grayscale, grayscale, SHAPE_CANNY_EDGE_LINK, SHAPE_CANNY_EDGE_FIND, SHAPE_CANNY_APERTURE);

            CvMemStorage *storage = cvCreateMemStorage(0);
            CvSeq *sampleContours = NULL;

            cvFindContours(grayscale, storage, &sampleContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

            for (CvSeq *contour = sampleContours; contour != NULL; contour = contour->h_next)
	        {
		        if ( (fabs(cvArcLength(contour)) > SHAPE_MIN_LENGTH) &&
                     (fabs(cvContourArea(contour)) > SHAPE_MIN_AREA) ) {
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

    UpdateContourImage();

    if (isOnDisk) { // this classifier has been saved so we'll update the files
        Save();        
    }

    // update member variables
	isTrained = true;
}

void ShapeClassifier::ClassifyFrame(IplImage *frame, IplImage* guessMask) {
    if (!isTrained) return;
    if(!frame) return;

    IplImage *copy = cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
    IplImage *grayscale = cvCreateImage( cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);
    IplImage *newMask = cvCloneImage(guessMask);
    cvZero(newMask);

    cvCvtColor(frame, grayscale, CV_BGR2GRAY);
    cvCanny(grayscale, grayscale, SHAPE_CANNY_EDGE_LINK, SHAPE_CANNY_EDGE_FIND, SHAPE_CANNY_APERTURE);

    cvCvtColor(grayscale, copy, CV_GRAY2BGR);

    CvSeq *frameContours;
    CvMemStorage *storage = cvCreateMemStorage(0);
    cvFindContours(grayscale, storage, &frameContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    for (CvSeq *contour = frameContours; contour != NULL; contour = contour->h_next) {
        if ( (fabs(cvArcLength(contour)) > SHAPE_MIN_LENGTH) &&
             (fabs(cvContourArea(contour)) > SHAPE_MIN_AREA) ) {

            int contourNum = 0;
            for (CvSeq *matchContour = templateContours; matchContour != NULL; matchContour = matchContour->h_next) {
                double similarity = cvMatchShapes(contour, matchContour, CV_CONTOURS_MATCH_I1, 0);
                if (similarity < SHAPE_SIMILARITY_THRESHOLD) {
                    cvDrawContours(copy, contour, colorSwatch[contourNum], CV_RGB(0,0,0), 0, 3, 8, cvPoint(0,0));
		            CvRect rect = cvBoundingRect(contour, 1);

                    // draw rectangle in mask image
                    cvRectangle(newMask, cvPoint(rect.x, rect.y), cvPoint(rect.x+rect.width, rect.y+rect.height), cvScalar(0xFF), CV_FILLED, 8);
                }
                contourNum = (contourNum+1) % COLOR_SWATCH_SIZE;
            }
        }
    }
    cvResize(copy, applyImage);
    IplToBitmap(applyImage, applyBitmap);

    // Combine old and new mask
    // TODO: support OR operation as well
    cvAnd(guessMask, newMask, guessMask);

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&copy);
    cvReleaseImage(&grayscale);
	cvReleaseImage(&newMask);
}

void ShapeClassifier::UpdateContourImage() {
    cvZero(filterImage);
    int contourNum = 0;
    for (CvSeq *contour = templateContours; contour != NULL; contour = contour->h_next) {
        // TODO: draw contours scaled in different places in image so they don't overlap
        cvDrawContours(filterImage, contour, colorSwatch[contourNum], CV_RGB(0,0,0), 0, 1, 8, cvPoint(0,0));
        contourNum = (contourNum+1) % COLOR_SWATCH_SIZE;
    }
    IplToBitmap(filterImage, filterBitmap);
}

void ShapeClassifier::Save() {
    if (!isTrained) return;

	Classifier::Save();

	USES_CONVERSION;
    WCHAR filename[MAX_PATH];

	// save the contour data
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_CONTOUR_NAME);

    const char* contour_attrs[] = {
        "recursive", "1",
        0
    };
    cvSave(W2A(filename), templateContours, 0, 0, cvAttrList(contour_attrs,0));
}
