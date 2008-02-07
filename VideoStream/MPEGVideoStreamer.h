#pragma once

class MPEGVideoStreamer
{
public:
	MPEGVideoStreamer(void);
	~MPEGVideoStreamer(void);

	void StartStreaming();
	void UpdateFrame(IplImage *newFrame);
	void StopStreaming();
private:
	static DWORD WINAPI ThreadCallback(LPVOID);

	IplImage *frame;
	DWORD threadID;
	HANDLE m_hThread;
	bool streaming;
};
