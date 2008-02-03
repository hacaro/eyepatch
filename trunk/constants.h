// Listview Icon size
#define LISTVIEW_SAMPLE_X 64
#define LISTVIEW_SAMPLE_Y 64

// Haar Training parameters
#define HAAR_SAMPLE_X 24
#define HAAR_SAMPLE_Y 24
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
#define SHAPE_MIN_LENGTH 100
#define SHAPE_MIN_AREA 10
#define SHAPE_CANNY_EDGE_FIND 250
#define SHAPE_CANNY_EDGE_LINK 100
#define SHAPE_CANNY_APERTURE 5
#define SHAPE_SIMILARITY_THRESHOLD 0.5

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

// constants for gesture recognition
// number of frames for which we collect background information before detecting blobs
#define GESTURE_NUM_FGTRAINING_FRAMES 10
// minimum number of frames required for a valid gesture
#define GESTURE_MIN_TRAJECTORY_LENGTH 20
// number of samples away from the end we need to be to consider the gesture finished
#define GESTURE_PHASE_CUTOFF 10
#define GESTURE_NUM_CONDENSATION_SAMPLES 1000
// maximum number of tracks we can look at simultaneously
#define GESTURE_MAX_SIMULTANEOUS_TRACKS 1

// minimum number of background subtraction frames to "spin up" background model
#define BACKGROUND_SUBTRACTION_MIN_FRAMES 10
// number of frames to discard at beginning before starting to build background model
#define BACKGROUND_SUBTRACTION_DISCARD_FRAMES 5

// OSC parameters
#define OSC_OUTPUT_BUFFER_SIZE 1024
#define OSC_ADDRESS "127.0.0.1"
#define OSC_PORT 7000

// TCP parameters
#define TCP_LISTEN_PORT 8000
#define TCP_OUTPUT_BUFFER_SIZE 8192

// control placement
#define WINDOW_X 1024
#define WINDOW_Y 768
#define VIDEO_X 640
#define VIDEO_Y 480
#define SLIDER_Y 60
#define EXAMPLEWINDOW_X 0
#define EXAMPLEWINDOW_Y (VIDEO_Y+SLIDER_Y)
#define EXAMPLEWINDOW_WIDTH VIDEO_X
#define EXAMPLEWINDOW_HEIGHT 185
#define FILTERIMAGE_WIDTH 240
#define FILTERIMAGE_HEIGHT 180
#define FILTERLIBRARY_WIDTH 670

// save recognizer: folder prefixes and file names
#define FILE_FRIENDLY_NAME L"\\name.dat"
#define FILE_DATA_NAME L"\\data.dat"
#define FILE_THRESHOLD_NAME L"\\threshold.dat"
#define FILE_CONTOUR_NAME L"\\data.xml"
#define FILE_CASCADE_NAME L"\\classifier.xml"
#define FILE_IMAGE_NAME L"\\image.jpg"
#define FILE_CLASSIFIER_PREFIX L"epc"

// save recognizer: folder suffixes
#define FILE_BRIGHTNESS_SUFFIX L"_BRI"
#define FILE_COLOR_SUFFIX L"_COL"
#define FILE_GESTURE_SUFFIX L"_GES"
#define FILE_HAAR_SUFFIX L"_APP"
#define FILE_MOTION_SUFFIX L"_MOT"
#define FILE_SHAPE_SUFFIX L"_SHP"
#define FILE_SIFT_SUFFIX L"_SIF"

// filter IDs (also used as combo box indices)
#define COLOR_FILTER		0
#define SHAPE_FILTER		1
#define BRIGHTNESS_FILTER	2
#define SIFT_FILTER			3
#define ADABOOST_FILTER		4
#define MOTION_FILTER		5
#define GESTURE_FILTER		6

#define APP_CLASS L"Eyepatch"
#define FILTER_CREATE_CLASS L"VideoMarkup"
#define FILTER_COMPOSE_CLASS L"FilterComposer"

// Custom window messages
#define WM_ADD_CUSTOM_FILTER   (WM_APP+1)
#define WM_ADD_STD_FILTER      (WM_APP+2)
#define WM_ADD_DATA_REDUCER    (WM_APP+3)
#define WM_ADD_OUTPUT_SINK     (WM_APP+4)
#define WM_LOAD_FILTER         (WM_APP+5)
#define WM_SET_THRESHOLD       (WM_APP+6)

// Classifier type for standard built-in classifiers
#define IDC_STANDARD_CLASSIFIER 10001

// Number of classifier types in the system
#define NUM_FILTERS 7

// listview group IDs
typedef enum {
    GROUPID_POSSAMPLES = 0,
    GROUPID_NEGSAMPLES,
    GROUPID_RANGESAMPLES,
    GROUPID_TRASH
} ListviewGroupId;

// eyepatch modes
typedef enum {
    EYEPATCHMODE_CREATEFILTERS = 0,
    EYEPATCHMODE_RUNFILTERS
} EyepatchMode;
