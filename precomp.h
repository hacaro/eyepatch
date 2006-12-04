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

// Color matching parameters
#define COLOR_MIN_AREA 100
#define COLOR_MAX_AREA 120000
#define COLOR_VMIN 15
#define COLOR_VMAX 230
#define COLOR_SMIN 30

// shape matching parameters
// todo: add canny thresholds
#define SHAPE_MIN_LENGTH 100
#define SHAPE_SIMILARITY_THRESHOLD 0.2

// SIFT matching parameters
/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 200
/* threshold on squared ratio of distances between NN and 2nd NN */
#define NN_SQ_DIST_RATIO_THR 0.49
#define SIFT_MIN_RANSAC_FEATURES 4

// Motion parameters
#define MOTION_MIN_COMPONENT_AREA 100
/* history image and deltas are in frames, not seconds */
#define MOTION_MHI_DURATION 15.0
#define MOTION_MAX_TIME_DELTA 7.0
#define MOTION_MIN_TIME_DELTA 1.0
/* amount of pixel difference considered motion */
#define MOTION_DIFF_THRESHOLD 30
/* amount of angle difference (in degrees) that we still consider the same direction */
#define MOTION_ANGLE_DIFF_THRESHOLD 30
/* number of images used to compute silhouette */
#define MOTION_NUM_IMAGES 4
#define MOTION_NUM_HISTORY_FRAMES 15

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

// SIFT includes
extern "C" {
    #include "SIFT/sift.h"
    #include "SIFT/imgfeatures.h"
    #include "SIFT/kdtree.h"
    #include "SIFT/utils.h"
    #include "SIFT/xform.h"
}

// Utility functions
void IplToBitmap(IplImage *src, Bitmap *dst);
CvScalar hsv2rgb(float hue);
void DrawArrow(IplImage *img, CvPoint center, double angleDegrees, double magnitude, CvScalar color, int thickness=1);

// swatch of "nice" colors
#define COLOR_SWATCH_SIZE 16
extern CvScalar colorSwatch[];

