#include "precomp.h"
#include "TrainingSample.h"

TrainingSample::TrainingSample(IplImage* srcImage, IplImage* motionHist, HWND lc, HIMAGELIST il, Rect bounds, int groupId) {
    hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;

    fullImageCopy = cvCreateImage(cvSize(bounds.Width,bounds.Height),IPL_DEPTH_8U, 3); 
    motionHistory = cvCreateImage(cvSize(bounds.Width,bounds.Height),IPL_DEPTH_32F, 1);

    resizedImage = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3); 
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    cvSetImageROI(srcImage, cvRect( bounds.X, bounds.Y, bounds.Width, bounds.Height));
    cvCopyImage(srcImage, fullImageCopy);
    cvSetImageROI(motionHist, cvRect( bounds.X, bounds.Y, bounds.Width, bounds.Height));
    cvCopyImage(motionHist, motionHistory);

    if (srcImage->width >= LISTVIEW_SAMPLE_X && srcImage->height >= LISTVIEW_SAMPLE_Y) {
        cvResize(srcImage, resizedImage, CV_INTER_AREA);
    } else { 
        cvResize(srcImage, resizedImage, CV_INTER_LINEAR);
    }
    cvResetImageROI(srcImage);
    cvResetImageROI(motionHist);

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
    int newListItemPos = ListView_InsertItem(hwndListControl, &lvi);

    id = ListView_MapIndexToID(hwndListControl, newListItemPos);
}

TrainingSample::TrainingSample(char *filename, HWND lc, HIMAGELIST il, int groupId) {
    hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;

    fullImageCopy = cvLoadImage(filename, 1);
    if (fullImageCopy == NULL) {
        fullImageCopy = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3);
        cvZero(fullImageCopy);
    }
    motionHistory = NULL;

    resizedImage = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3); 
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    if (fullImageCopy->width >= LISTVIEW_SAMPLE_X && fullImageCopy->height >= LISTVIEW_SAMPLE_Y) {
        cvResize(fullImageCopy, resizedImage, CV_INTER_AREA);
    } else { 
        cvResize(fullImageCopy, resizedImage, CV_INTER_LINEAR);
    }

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
    int newListItemPos = ListView_InsertItem(hwndListControl, &lvi);

    id = ListView_MapIndexToID(hwndListControl, newListItemPos);
}

TrainingSample::~TrainingSample(void) {
    cvReleaseImage(&fullImageCopy);
    cvReleaseImage(&resizedImage);
    if (motionHistory != NULL) cvReleaseImage(&motionHistory);
    delete bmpImage;
}

void TrainingSample::Draw(Graphics *g, int x, int y) {
    g->DrawImage(bmpImage, x, y);
}
