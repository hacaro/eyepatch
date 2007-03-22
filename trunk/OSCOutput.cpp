#include "precomp.h"
#include "constants.h"

#include "OutputSink.h"
#include "OSCOutput.h"

OSCOutput::OSCOutput() :
    OutputSink() {
    transmitSocket = new UdpTransmitSocket(IpEndpointName(OSC_ADDRESS, OSC_PORT));
    contourStorage = cvCreateMemStorage(0);

    SetName(L"OSC over UDP");
}

OSCOutput::~OSCOutput() {
    cvReleaseMemStorage(&contourStorage);
    delete transmitSocket;
}

void OSCOutput::OutputData(IplImage *image, IplImage *mask, char *filterName) {
    char buffer[OSC_OUTPUT_BUFFER_SIZE];
    char messageType[OSC_OUTPUT_BUFFER_SIZE];
    sprintf(messageType, "%s/", filterName);
    osc::OutboundPacketStream p( buffer, OSC_OUTPUT_BUFFER_SIZE );

    p << osc::BeginBundleImmediate;

    CvSeq* contours = NULL;
    cvFindContours(mask, contourStorage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
    if (contours != NULL) {
        contours = cvApproxPoly(contours, sizeof(CvContour), contourStorage, CV_POLY_APPROX_DP, 3, 1 );
        for (CvSeq *contour = contours; contour != NULL; contour = contour->h_next) {
            CvRect r = cvBoundingRect(contour, 1);
            CvPoint center;
            int radius;
            center.x = cvRound((r.x + r.width*0.5));
            center.y = cvRound((r.y + r.height*0.5));
            radius = cvRound((r.width + r.height)*0.25);

            p << osc::BeginMessage(messageType)
                << center.x << center.y << radius << osc::EndMessage;
        }
    }
    cvClearMemStorage(contourStorage);
    p << osc::EndBundle;
   
    transmitSocket->Send( p.Data(), p.Size() );
}