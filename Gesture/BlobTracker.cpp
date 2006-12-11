#include "../precomp.h"
#include "TrajectoryList.h"
#include "BlobTracker.h"

BlobTracker::BlobTracker() {

    isTrained = false;

    ZeroMemory(&param, sizeof(CvBlobTrackerAutoParam1));

    // Set number of foreground training frames
    param.FGTrainFrames = GESTURE_NUM_FGTRAINING_FRAMES;

    // Create FG Detection module
    param.pFG = cvCreateFGDetectorBase(CV_BG_MODEL_FGD, NULL);

    // Create Blob Entrance Detection module
//    param.pBD = cvCreateBlobDetectorSimple();
    param.pBD = cvCreateBlobDetectorCC();
    param.pBD->SetParam("MinDistToBorder",0);
    param.pBD->SetParam("Latency",5);

    // Create blob tracker module
    param.pBT = cvCreateBlobTrackerCCMSPF();

    // create blob trajectory generation module (currently not needed)
    param.pBTGen = (CvBlobTrackGen*) &trajectories;
    param.pBTGen->SetFileName("C:\\users\\monzy\\Desktop\\trajectorylog.txt");

    // create blob trajectory post processing module
    param.pBTPP = cvCreateModuleBlobTrackPostProcKalman();

    // create blob trajectory analysis module (currently not needed)
    param.UsePPData = 0;
    param.pBTA = NULL;

    // create a pipeline using these components
    pTracker = cvCreateBlobTrackerAuto1(&param);
}

BlobTracker::~BlobTracker() {

    // Release all the modules
    if(param.pBT)cvReleaseBlobTracker(&param.pBT);
    if(param.pBD)cvReleaseBlobDetector(&param.pBD);
    if(param.pBTGen)cvReleaseBlobTrackGen(&param.pBTGen);
    if(param.pBTA)cvReleaseBlobTrackAnalysis(&param.pBTA);
    if(param.pFG)cvReleaseFGDetector(&param.pFG);
    if(pTracker)cvReleaseBlobTrackerAuto(&pTracker);
}

void BlobTracker::LearnTrajectories(CvCapture* videoCapture) {

    cvNamedWindow("Tracking"); // display tracking output

    int nFrames = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_COUNT);
    cvSetCaptureProperty(videoCapture, CV_CAP_PROP_POS_FRAMES, 0);

    IplImage *currentFrame = cvQueryFrame(videoCapture);
    if(currentFrame == NULL) return;

    IplImage* copyFrame = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,3);
    IplImage* maskedCopyFrame = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,3);
    IplImage* fgImageCopy = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,3);

    for(int FrameNum=0; FrameNum < nFrames; FrameNum++) {

        // make sure the image isn't upside down
        if (currentFrame->origin  == IPL_ORIGIN_TL) {
            cvCopy(currentFrame,copyFrame);
        } else {
            cvFlip(currentFrame,copyFrame);
        }

        // Process the new frame with the blob tracker
        // the second parameter is an optional mask
        pTracker->Process(copyFrame, NULL);

        // make a color copy of the grayscale foreground mask
        IplImage* fgImage = pTracker->GetFGMask();
        cvCvtColor(fgImage, fgImageCopy, CV_GRAY2BGR );

        // mask the current frame with the foreground mask
        cvAnd(copyFrame, fgImageCopy, maskedCopyFrame);

        // overlay masked image on original image
        cvAddWeighted(copyFrame, 0.3, maskedCopyFrame, 0.7, 0.0, maskedCopyFrame);

        int nBlobs = pTracker->GetBlobNum();
        if (nBlobs>0) { // some blobs were detected in the current frame
            for(int i=0; i<nBlobs; i++) {
                CvSize  TextSize;
                CvBlob* pB = pTracker->GetBlob(i-1);
                CvPoint center = cvPoint(cvRound(pB->x*256),cvRound(pB->y*256));
                CvSize  size = cvSize(MAX(1,cvRound(CV_BLOB_RX(pB)*256)), MAX(1,cvRound(CV_BLOB_RY(pB)*256)));

                cvEllipse(maskedCopyFrame, center, size, 0, 0, 360, CV_RGB(0,255,0), 1, CV_AA, 8);

                // TODO: should we use the blob id (int) or state description (char*) ?
                //  CV_BLOB_ID(pB)
                //  pTracker->GetStateDesc(CV_BLOB_ID(pB));
            }
        }

        // display color foreground image in window
        cvShowImage("Tracking",maskedCopyFrame);

        cvWaitKey(1);

        // grab the next frame of the video
        currentFrame = cvQueryFrame(videoCapture);
    }

    cvReleaseImage(&copyFrame);
    cvReleaseImage(&fgImageCopy);

}

void BlobTracker::GetTrajectories(vector<MotionTrack> *trackList) {
    trajectories.GetTracks(trackList);
}
