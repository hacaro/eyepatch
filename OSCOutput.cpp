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

void OSCOutput::ProcessOutput(IplImage *image, IplImage *mask, ClassifierOutputData data, char *filterName) {
    char buffer[OSC_OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream p( buffer, OSC_OUTPUT_BUFFER_SIZE );

    p << osc::BeginBundleImmediate;

	int nVars = data.NumVariables();
	for (int i=0; i<nVars; i++) {
		ClassifierOutputVariable var = data.data[i];
		if (var.GetState() == true) {	// this variable is active
			switch(var.GetType()) {
				case CVAR_VOID:
				case CVAR_IMAGE:
					// can't do anything with these types
					break;
				case CVAR_INT:
					break;
				case CVAR_FLOAT:
					break;
				case CVAR_POINT:
					break;
				case CVAR_STRING:
					break;
				case CVAR_SEQ:
					break;
				case CVAR_BBOXES:
					vector<Rect> *bboxes = var.GetBoundingBoxData();
					for (vector<Rect>::iterator box = bboxes->begin(); box != bboxes->end(); box++) {
						Rect r = (*box);
						p << osc::BeginMessage(var.GetName().c_str())
							<< ((int)r.X) << ((int)r.Y) << ((int)r.Width) << ((int)r.Height) << osc::EndMessage;
					}
					break;
			}
		}
	}
    p << osc::EndBundle;  
    transmitSocket->Send( p.Data(), p.Size() );
}
