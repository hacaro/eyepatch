#pragma once

class TrainingSample
{
public:
    IplImage *fullImageCopy, *motionHistory;
    LVITEM lvi;
    HBITMAP hbmImage;
    UINT id;
    int iGroupId;

    TrainingSample(IplImage*, IplImage*, HWND, HIMAGELIST, Rect, int);
    ~TrainingSample(void);
    void Draw(Graphics*, int x, int y);

private:
    IplImage *resizedImage;
    Bitmap *bmpImage;
    int width, height;
    HWND hwndListControl;
    HIMAGELIST hImageList;
};
