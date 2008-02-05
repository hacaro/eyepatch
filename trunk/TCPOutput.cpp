#include "precomp.h"
#include "constants.h"

#include "OutputSink.h"
#include "TCPOutput.h"

TCPOutput::TCPOutput() :
    OutputSink() {

    SetName(L"XML over TCP");

    // Begin server listen thread
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerListenThread, (LPVOID)&server, 0, &threadID);
}

TCPOutput::~TCPOutput() {

    // End server listen thread
	TerminateThread(m_hThread, 0);
}

DWORD WINAPI TCPOutput::ServerListenThread(SocketServer* server) {
    while (true) {
        server->WaitForConnection();
        while (server->IsConnected()) {
            Sleep(100);
        }
    }
    return 1L;
}

void TCPOutput::ProcessInput(IplImage *image) {
}

void TCPOutput::ProcessOutput(IplImage *image, IplImage *mask, CvSeq* contours, char *filterName) {
    char buffer[TCP_OUTPUT_BUFFER_SIZE];
    char message[TCP_OUTPUT_BUFFER_SIZE];

    // If nobody is connected to the server, don't bother doing anything
    if (!server.IsConnected()) return;

    sprintf(buffer,"<FRAME ID=\"%s\">\r\n", filterName);
    if (contours != NULL) {
        for (CvSeq *contour = contours; contour != NULL; contour = contour->h_next) {
            CvRect r = cvBoundingRect(contour, 1);
            CvPoint center;
            int radius;
            center.x = cvRound((r.x + r.width*0.5));
            center.y = cvRound((r.y + r.height*0.5));
            radius = cvRound((r.width + r.height)*0.25);

			sprintf(message,"<REGION X=\"%d\" Y=\"%d\" size=\"%d\" />\r\n", center.x, center.y, radius);
            strcat(buffer, message);

        }
    }
    strcat(buffer,"</FRAME>\r\n\0");
    server.SendData(buffer);
}