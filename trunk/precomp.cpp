#include "precomp.h"

void IplToBitmap(IplImage *src, Bitmap *dst) {
    BitmapData bmData;
    bmData.Width = src->width;
    bmData.Height = src->height;
    bmData.PixelFormat = PixelFormat24bppRGB;
    bmData.Stride = src->widthStep;
    bmData.Scan0 = src->imageData;
    Rect sampleRect(0, 0, src->width, src->height);
    dst->LockBits(&sampleRect, ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &bmData);
    dst->UnlockBits(&bmData);
}

CvScalar hsv2rgb( float hue ) {
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}