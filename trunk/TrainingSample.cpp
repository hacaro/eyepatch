#include "precomp.h"
#include "TrainingSample.h"

TrainingSample::TrainingSample(IplImage* srcImage, IplImage* motionHist, HWND lc, HIMAGELIST il, Rect bounds, int groupId) {
    hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;

    fullImageCopy = cvCreateImage(cvSize(bounds.Width,bounds.Height),IPL_DEPTH_8U, 3); 
    motionHistory = cvCreateImage(cvSize(bounds.Width,bounds.Height),IPL_DEPTH_32F, 1);

    resizedImage = cvCreateImage(cvSize(SAMPLE_X,SAMPLE_Y),IPL_DEPTH_8U, 3); 
    sampleImage = cvCreateImage(cvSize(SAMPLE_X,SAMPLE_Y),IPL_DEPTH_8U, 1);
    bmpImage = new Bitmap(SAMPLE_X, SAMPLE_Y, PixelFormat24bppRGB);

    cvSetImageROI(srcImage, cvRect( bounds.X, bounds.Y, bounds.Width, bounds.Height));
    cvCopyImage(srcImage, fullImageCopy);
    cvSetImageROI(motionHist, cvRect( bounds.X, bounds.Y, bounds.Width, bounds.Height));
    cvCopyImage(motionHist, motionHistory);
    if (srcImage->width >= SAMPLE_X && srcImage->height >= SAMPLE_Y) {
        cvResize(srcImage, resizedImage, CV_INTER_AREA);
    } else { 
        cvResize(srcImage, resizedImage, CV_INTER_LINEAR);
    }
    cvResetImageROI(srcImage);
    cvResetImageROI(motionHist);

    cvCvtColor(resizedImage,sampleImage,CV_BGR2GRAY);

    IplToBitmap(resizedImage, bmpImage);
    bmpImage->GetHBITMAP(NULL, &hbmImage);

    // Add image to imagelist
    int imgIndex = ImageList_Add(hImageList, hbmImage, NULL);

    // Add item to list view
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_GROUPID;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iGroupId = groupId;
    lvi.iItem = imgIndex;
    lvi.iImage = imgIndex;
    lvi.iSubItem = 0;
    ListView_InsertItem(hwndListControl, &lvi);

    id = imgIndex;
}

TrainingSample::~TrainingSample(void) {
    cvReleaseImage(&sampleImage);
    cvReleaseImage(&fullImageCopy);
    cvReleaseImage(&resizedImage);
    cvReleaseImage(&motionHistory);
    delete bmpImage;
}

void TrainingSample::Draw(Graphics *g, int x, int y) {
    g->DrawImage(bmpImage, x, y);
}
