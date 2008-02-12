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
	int ival;
	float fval;
	Point pt;
	vector<Rect> *bboxes;
	string sval;

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
					ival = var.GetIntData();
					p << osc::BeginMessage(var.GetName().c_str()) << ival << osc::EndMessage;
					break;
				case CVAR_FLOAT:
					fval = var.GetFloatData();
					p << osc::BeginMessage(var.GetName().c_str()) << fval << osc::EndMessage;
					break;
				case CVAR_POINT:
					pt = var.GetPointData();
					p << osc::BeginMessage(var.GetName().c_str()) << ((int)pt.X) << ((int)pt.Y) << osc::EndMessage;
					break;
				case CVAR_STRING:
					sval = var.GetStringData();
					p << osc::BeginMessage(var.GetName().c_str()) << (sval.c_str()) << osc::EndMessage;
					break;
				case CVAR_SEQ:
					break;
				case CVAR_BBOXES:
					bboxes = var.GetBoundingBoxData();
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
