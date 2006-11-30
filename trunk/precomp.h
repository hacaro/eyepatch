#pragma once

#define _WIN32_WINNT 0x0600
#define WINVER 0x0600
#define _WIN32_IE 0x0700
#define UNICODE

// Haar Training parameters
#define SAMPLE_X 24
#define SAMPLE_Y 24
#define MAX_SAMPLES 100
#define MIN_HAAR_STAGES 5
#define START_HAAR_STAGES 10

// color matching parameters
#define COLOR_MIN_AREA 100
#define COLOR_MAX_AREA 120000
#define COLOR_VMIN 15
#define COLOR_VMAX 230
#define COLOR_SMIN 30

// shape matching parameters
// todo: add canny thresholds
#define SHAPE_MIN_LENGTH 100
#define SHAPE_SIMILARITY_THRESHOLD 0.2

// control placement
#define WINDOW_X 1024
#define WINDOW_Y 768
#define VIDEO_X 640
#define VIDEO_Y 480
#define SLIDER_Y 35
#define FILTERIMAGE_X 0
#define FILTERIMAGE_Y 520
#define FILTERIMAGE_WIDTH 240
#define FILTERIMAGE_HEIGHT 180

#define APP_CLASS L"Eyepatch"
#define FILTER_CREATE_CLASS L"VideoMarkup"

#include <windows.h>
#include "resource.h"

#include <math.h>
#include <float.h>
#include <io.h>
#include <time.h>

#include <gdiplus.h>
using namespace Gdiplus;

#include <list>
#include <map>
#include <algorithm>
using namespace std;

#include <atlbase.h>
#include <atltypes.h>
#include <atlfile.h>
#include <atlwin.h>
//#include <atlmisc.h>
//#include <atlcrack.h>
//#include <atltheme.h>   // XP/Vista theme support
//#include <dwmapi.h>     // DWM APIs

// OpenCV Includes
#include "cv.h"
#include "highgui.h"
#include "cvhaartraining.h"


// Utility functions
void IplToBitmap(IplImage *src, Bitmap *dst);
CvScalar hsv2rgb(float hue);

// swatch of "nice" colors
#define COLOR_SWATCH_SIZE 16
extern CvScalar colorSwatch[];

