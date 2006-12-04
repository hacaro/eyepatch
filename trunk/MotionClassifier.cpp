#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "MotionClassifier.h"

MotionClassifier::MotionClassifier() :
	Classifier() {
    motionAngles.clear();
}

MotionClassifier::~MotionClassifier() {
}

void MotionClassifier::StartTraining(TrainingSet *sampleSet) {

    // clear list of motion directions
    motionAngles.clear();

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) { // positive sample

            CvSize size = cvSize(sample->motionHistory->width,sample->motionHistory->height);
            IplImage *orient = cvCreateImage(size, IPL_DEPTH_32F, 1);
            IplImage *segmask = cvCreateImage(size, IPL_DEPTH_32F, 1);
            IplImage *mask = cvCreateImage(size, IPL_DEPTH_8U, 1);
            IplImage *dst = cvCreateImage(size, IPL_DEPTH_8U, 3);
            CvMemStorage *storage = cvCreateMemStorage(0);

            // convert MHI to blue 8U image
            cvCvtScale(sample->motionHistory, mask, 255./MOTION_MHI_DURATION,(MOTION_MHI_DURATION-MOTION_NUM_HISTORY_FRAMES)*255./MOTION_MHI_DURATION);
            cvZero( dst );
            cvCvtPlaneToPix( mask, 0, 0, 0, dst );

            // calculate motion gradient orientation and valid orientation mask
            cvCalcMotionGradient(sample->motionHistory, mask, orient, MOTION_MAX_TIME_DELTA, MOTION_MIN_TIME_DELTA, 3 );
            
            // segment motion: get sequence of motion components
            // Segmask is marked motion components map.  It is not used further.
            CvSeq *seq = cvSegmentMotion(sample->motionHistory, segmask, storage, MOTION_NUM_HISTORY_FRAMES, MOTION_MAX_TIME_DELTA );

            // get the global motion of the sample image
            CvScalar color = CV_RGB(255,255,255);
            double magnitude = min(size.width/3, size.height/3);

            // calculate orientation and add to list of motion directions
            double motionAngle = cvCalcGlobalOrientation( orient, mask, sample->motionHistory, 1.0, MOTION_MHI_DURATION);
            motionAngle = 360.0 - motionAngle;  // adjust for images with top-left origin
            motionAngles.push_back(motionAngle);

            // draw a clock with arrow indicating the direction
            CvPoint center = cvPoint(size.width/2,size.height/2);

            cvCircle( dst, center, cvRound(magnitude*1.2), color, 3, CV_AA, 0 );
            cvLine( dst, center, cvPoint( cvRound( center.x + magnitude*cos(motionAngle*CV_PI/180)),
                    cvRound( center.y - magnitude*sin(motionAngle*CV_PI/180))), color, 3, CV_AA, 0 );

            // copy the motion picture to the demo image
            cvResize(dst, filterImage);
            IplToBitmap(filterImage, filterBitmap);

            cvReleaseImage(&orient);
            cvReleaseImage(&segmask);
            cvReleaseImage(&mask);
            cvReleaseImage(&dst);
            cvReleaseMemStorage(&storage);

		} else if (sample->iGroupId == 1) { // negative sample
        }
    }

    // update member variables
	nPosSamples = sampleSet->posSampleCount;
	nNegSamples = sampleSet->negSampleCount;
	isTrained = true;
}

void MotionClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

    // clear the list of guesses
	objList->clear();

    // check to make sure that the frame passed in is a motion history image (not a normal frame image)
    if (frame->depth != IPL_DEPTH_32F) return;
    if (frame->nChannels != 1) return;

    // first find the motion components in this motion history image
    CvSize size = cvSize(frame->width,frame->height);
    IplImage *orient = cvCreateImage(size, IPL_DEPTH_32F, 1);
    IplImage *segmask = cvCreateImage(size, IPL_DEPTH_32F, 1);
    IplImage *mask = cvCreateImage(size, IPL_DEPTH_8U, 1);
    IplImage *dst = cvCreateImage(size, IPL_DEPTH_8U, 3);
    CvMemStorage *storage = cvCreateMemStorage(0);

    // convert MHI to blue 8U image
    cvCvtScale(frame, mask, 255./MOTION_MHI_DURATION,(MOTION_MHI_DURATION-MOTION_NUM_HISTORY_FRAMES)*255./MOTION_MHI_DURATION);
    cvZero( dst );
    cvCvtPlaneToPix( mask, 0, 0, 0, dst );

    // calculate motion gradient orientation and valid orientation mask
    cvCalcMotionGradient(frame, mask, orient, MOTION_MAX_TIME_DELTA, MOTION_MIN_TIME_DELTA, 3 );
    
    // segment motion: get sequence of motion components
    // Segmask is marked motion components map.  It is not used further.
    CvSeq *seq = cvSegmentMotion(frame, segmask, storage, MOTION_NUM_HISTORY_FRAMES, MOTION_MAX_TIME_DELTA );

    // iterate through the motion components
    for(int i = 0; i < seq->total; i++) {
        CvRect comp_rect = ((CvConnectedComp*)cvGetSeqElem(seq, i ))->rect;
        if(comp_rect.width * comp_rect.height < MOTION_MIN_COMPONENT_AREA ) // reject very small components
            continue;
        CvScalar color = CV_RGB(255,0,0);
        double magnitude = 30;

        // select component ROI
        cvSetImageROI(frame, comp_rect );
        cvSetImageROI(orient, comp_rect );
        cvSetImageROI(mask, comp_rect );

        // calculate orientation
        double motionAngle = cvCalcGlobalOrientation(orient, mask, frame, MOTION_NUM_HISTORY_FRAMES, MOTION_MHI_DURATION);
        motionAngle = 360.0 - motionAngle;  // adjust for images with top-left origin

        cvResetImageROI(frame);
        cvResetImageROI(orient);
        cvResetImageROI(mask);

        // draw a clock with arrow indicating the direction
        CvPoint center = cvPoint((comp_rect.x + comp_rect.width/2), (comp_rect.y + comp_rect.height/2));

        cvCircle(dst, center, cvRound(magnitude*1.2), colorSwatch[i%COLOR_SWATCH_SIZE], 3, CV_AA, 0 );
        cvLine(dst, center, cvPoint( cvRound( center.x + magnitude*cos(motionAngle*CV_PI/180)),
                cvRound( center.y - magnitude*sin(motionAngle*CV_PI/180))), colorSwatch[i%COLOR_SWATCH_SIZE], 3, CV_AA, 0 );

        // copy motion picture to demo image
        cvResize(dst, applyImage);
        IplToBitmap(applyImage, applyBitmap);

        // now check if the direction of this component matches the direction in one of the samples
        for (list<double>::iterator i = motionAngles.begin(); i!=motionAngles.end(); i++) {
            double angleDiff = fabs((*i)-motionAngle);
            // for angles close to 360
            angleDiff = min(angleDiff, (((int)(*i+motionAngle)) % 360));
            if (angleDiff < MOTION_ANGLE_DIFF_THRESHOLD) { // add to list of guesses
                Rect objRect;
                objRect.X = comp_rect.x;
                objRect.Y = comp_rect.y;
                objRect.Width = comp_rect.width;
                objRect.Height = comp_rect.height;
                objList->push_back(objRect);
            }

        }


    }

    cvReleaseImage(&orient);
    cvReleaseImage(&segmask);
    cvReleaseImage(&mask);
    cvReleaseImage(&dst);
    cvReleaseMemStorage(&storage);
}
