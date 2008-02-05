#pragma once
#include "OutputSink.h"

// OSC Includes
#include "../OSCPack/osc/OscOutboundPacketStream.h"
#include "../OSCPack/ip/UdpSocket.h"

class OSCOutput : public OutputSink {
public:
    OSCOutput();
    ~OSCOutput();

	void ProcessInput(IplImage* image);
	void ProcessOutput(IplImage* image, IplImage* mask, CvSeq* contours, char* filterName);

private:
    UdpTransmitSocket *transmitSocket;
};
