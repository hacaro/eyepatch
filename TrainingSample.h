#pragma once

class TrainingSample
{
public:
    IplImage *fullImageCopy, *motionHistory;
    LVITEM lvi;
    HBITMAP hbmImage;
    UINT id;
    int iGroupId;
	Rect selectBounds;
    MotionTrack motionTrack;

    TrainingSample(IplImage*, IplImage*, HWND, HIMAGELIST, Rect, int);
    TrainingSample(char *filename, HWND lc, HIMAGELIST il, int groupId);
    TrainingSample(IplImage*, MotionTrack mt, HWND lc, HIMAGELIST il, int groupId);
    ~TrainingSample(void);
    void Draw(Graphics*, int x, int y);

private:
    IplImage *resizedImage;
    Bitmap *bmpImage;
    int width, height;
    HWND hwndListControl;
    HIMAGELIST hImageList;
};
