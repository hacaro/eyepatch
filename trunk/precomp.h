#pragma once

#define _WIN32_WINNT 0x0600
#define WINVER 0x0600
#define _WIN32_IE 0x0700
#define UNICODE

// Windows includes
#include <windows.h>
#include <shfolder.h>
#include <shlobj.h>

// standard includes
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <io.h>
#include <time.h>

#include <gdiplus.h>
using namespace Gdiplus;

// STL includes
#include <list>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

// ATL includes
#include <atlbase.h>
#include <atltypes.h>
#include <atlfile.h>
#include <atlwin.h>

// OpenCV Includes
#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

// Haar Training Includes
#include "CVHaar/_cvcommon.h"
#include "CVHaar/_cvhaartraining.h"
#include "CVHaar/cvhaartraining.h"
#include "CVHaar/cvclassifier.h"

// SIFT includes
#include "SIFT/sift.h"
#include "SIFT/imgfeatures.h"
#include "SIFT/kdtree.h"
#include "SIFT/utils.h"
#include "SIFT/xform.h"

// MotionSample and MotionTrack types for gestures
typedef struct _MotionSample {
    double x, y, vx, vy, sizex, sizey;
} MotionSample;
typedef vector<MotionSample> MotionTrack;

// Gesture Tracking includes
#include "Gesture/RandomUtils.h"
#include "Gesture/TrajectoryList.h"
#include "Gesture/BlobTracker.h"
#include "Gesture/TrajectoryModel.h"
#include "Gesture/Condensation.h"

// Utility functions
void IplToBitmap(IplImage *src, Bitmap *dst);
CvScalar hsv2rgb(float hue);
void DrawArrow(IplImage *img, CvPoint center, double angleDegrees, double magnitude, CvScalar color, int thickness=1);
void DrawTrack(IplImage *img, MotionTrack mt, CvScalar color, int thickness, int maxNumPoints=0);
void DrawTrack(IplImage *img, TrajectoryModel *mt, CvScalar color, int thickness, int maxNumPoints=0);
bool DeleteDirectory(LPCTSTR lpszDir, bool useRecycleBin);

// swatch of "nice" colors
#define COLOR_SWATCH_SIZE 16
extern CvScalar colorSwatch[];

