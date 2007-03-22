#pragma once
#include "OutputSink.h"

// OSC Includes
#include "../OSCPack/osc/OscOutboundPacketStream.h"
#include "../OSCPack/ip/UdpSocket.h"

class OSCOutput : public OutputSink {
public:
    OSCOutput();
    ~OSCOutput();

    void OutputData(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName);
private:
    UdpTransmitSocket *transmitSocket;
};
