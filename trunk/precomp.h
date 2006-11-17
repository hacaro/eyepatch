#pragma once

#define _WIN32_WINNT 0x0600
#define WINVER 0x0600
#define _WIN32_IE 0x0700
#define UNICODE

#define SAMPLE_X 24
#define SAMPLE_Y 24
#define MAX_SAMPLES 100
#define WINDOW_X 1024
#define WINDOW_Y 565
#define VIDEO_X 640
#define VIDEO_Y 480
#define SLIDER_Y 35

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
