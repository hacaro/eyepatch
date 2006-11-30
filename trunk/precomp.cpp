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

CvScalar colorSwatch[COLOR_SWATCH_SIZE] = {
    CV_RGB(0x45,0x8A,0x8A),
    CV_RGB(0xE6,0xA1,0x73),
    CV_RGB(0xE6,0xC3,0x73),
    CV_RGB(0x5C,0x5C,0xA1),
    CV_RGB(0x40,0x80,0x80),
    CV_RGB(0x80,0x59,0x40),
    CV_RGB(0x80,0x6C,0x40),
    CV_RGB(0x49,0x49,0x80),
    CV_RGB(0xCF,0xE6,0xE6),
    CV_RGB(0xE6,0xD8,0xCF),
    CV_RGB(0xE6,0xDF,0xCF),
    CV_RGB(0xD2,0xD2,0xE6),
    CV_RGB(0x30,0xBF,0xBF),
    CV_RGB(0xBF,0x69,0x30),
    CV_RGB(0xBF,0x94,0x30),
    CV_RGB(0x44,0x44,0xBF)
};

