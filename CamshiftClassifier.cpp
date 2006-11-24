#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "CamshiftClassifier.h"

CamshiftClassifier::CamshiftClassifier() :
	Classifier() {
	image = NULL;
	hsv = NULL;
	hue = NULL;
	mask = NULL;
	backproject = NULL;
	histimg = NULL;
	hist = NULL;

	backproject_mode = 0;
	select_object = 0;
	track_object = 0;
	show_hist = 1;
	hdims = 16;
	hranges_arr[0] = 0;	hranges_arr[1] = 180;
	hranges = hranges_arr;
	vmin = 10;
	vmax = 256;
	smin = 30;
}

CamshiftClassifier::~CamshiftClassifier() {
}

CvScalar CamshiftClassifier::hsv2rgb( float hue )
{
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

void CamshiftClassifier::PrepareData(TrainingSet *sampleSet) {
	readyForTraining = false;

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) { // positive sample
			// for now just take from first pos sample
			// this should really create a color histogram across all positive samples
			sampleimage = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 3 );
			sampleimage->origin = sample->fullImageCopy->origin;
			cvCopy( sample->fullImageCopy, sampleimage, 0 );
			cvXorS( sampleimage, cvScalarAll(255), sampleimage, 0 );
			break;
		} else if (sample->iGroupId == 1) { // negative sample
        }
    }

	nPosSamples = sampleSet->posSampleCount;
	nNegSamples = sampleSet->negSampleCount;
	readyForTraining = true;
}

void CamshiftClassifier::StartTraining() {
	if (!readyForTraining) return;
	isTrained = true;
}

void CamshiftClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

	int i, bin_w;

    cvNamedWindow( "Histogram", 1 );
    cvNamedWindow( "CamShiftDemo", 1 );

    if( !image )
    {
        /* allocate all the buffers */
        image = cvCreateImage( cvGetSize(frame), 8, 3 );
        image->origin = frame->origin;

        hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
        hue = cvCreateImage( cvGetSize(frame), 8, 1 );
        mask = cvCreateImage( cvGetSize(frame), 8, 1 );

        samplehsv = cvCreateImage( cvGetSize(sampleimage), 8, 3 );
        samplehue = cvCreateImage( cvGetSize(sampleimage), 8, 1 );
        samplemask = cvCreateImage( cvGetSize(sampleimage), 8, 1 );

		backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
        hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
        histimg = cvCreateImage( cvSize(320,200), 8, 3 );
        cvZero( histimg );
    }

    cvCopy( frame, image, 0 );
//    cvCvtColor( image, hsv, CV_BGR2HSV );
    cvCvtColor(sampleimage, samplehsv, CV_BGR2HSV );
    cvCvtColor(image, hsv, CV_BGR2HSV );

	// track the object
    int _vmin = vmin, _vmax = vmax;
    cvInRangeS( samplehsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                cvScalar(180,256,MAX(_vmin,_vmax),0), samplemask );
    cvSplit( samplehsv, samplehue, 0, 0, 0 );

    cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
    cvSplit( hsv, hue, 0, 0, 0 );


//	if( track_object < 0 ) 
    {
        float max_val = 0.f;
        cvCalcHist( &samplehue, hist, 0, samplemask );
        cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
        cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
        track_window.x = 0;	track_window.y = 0;
		track_window.width = frame->width; track_window.height = frame->height;
        track_object = 1;

        cvZero( histimg );
        bin_w = histimg->width / hdims;
        for( i = 0; i < hdims; i++ )
        {
            int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
            CvScalar color = hsv2rgb(i*180.f/hdims);
            cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
                         cvPoint((i+1)*bin_w,histimg->height - val),
                         color, -1, 8, 0 );
        }
    }

    cvCalcBackProject( &hue, backproject, hist );
    cvAnd( backproject, mask, backproject, 0 );
    cvCamShift(backproject, track_window,
                cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                &track_comp, &track_box );
    track_window = track_comp.rect;
        
    if( backproject_mode )
        cvCvtColor( backproject, image, CV_GRAY2BGR );
    if( image->origin )
        track_box.angle = -track_box.angle;
    cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );

   cvShowImage( "CamShiftDemo", image );
   cvShowImage( "Histogram", histimg );

    //objList->clear();
    //// Loop over the found objects
    //Rect objRect;
    //objList->push_back(objRect);
}
