#include "precomp.h"
#include "constants.h"

#include "OutputSink.h"
#include "OSCOutput.h"

OSCOutput::OSCOutput() :
    OutputSink() {
    transmitSocket = new UdpTransmitSocket(IpEndpointName(OSC_ADDRESS, OSC_PORT));
 
    SetName(L"OSC over UDP on Port 7000");
}

OSCOutput::~OSCOutput() {
    delete transmitSocket;
}

void OSCOutput::ProcessInput(IplImage *image) {
}

void OSCOutput::ProcessOutput(IplImage *image, IplImage *mask, CvSeq* contours, char *filterName) {
    char buffer[OSC_OUTPUT_BUFFER_SIZE];
    char messageType[OSC_OUTPUT_BUFFER_SIZE];
    sprintf(messageType, "%s/", filterName);
    osc::OutboundPacketStream p( buffer, OSC_OUTPUT_BUFFER_SIZE );

    p << osc::BeginBundleImmediate;

    if (contours != NULL) {
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
    p << osc::EndBundle;
   
    transmitSocket->Send( p.Data(), p.Size() );
}