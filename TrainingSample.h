#pragma once

class TrainingSample
{
public:
    IplImage *sampleImage, *fullImageCopy;
    LVITEM lvi;
    HBITMAP hbmImage;
    UINT id;
    int iGroupId;

    TrainingSample(IplImage*, HWND, HIMAGELIST, Rect, int);
    ~TrainingSample(void);
    void Draw(Graphics*, int x, int y);

private:
    IplImage *resizedImage;
    Bitmap *bmpImage;
    int width, height;
    HWND hwndListControl;
    HIMAGELIST hImageList;
};
