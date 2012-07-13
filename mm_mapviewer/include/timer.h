/*
Timer Class
*/
#ifndef _ae_Timer_FILE
#define _ae_Timer_FILE

class aeTimer
{
private:
	DWORD  starttime;
	double frametime, lastfps_time, lastframe;
	DWORD fps, fps_count, maxfps;
public:
	aeTimer();
	~aeTimer();
	void Init();
	//for Call each frame
	bool   StartFrame();
	double FrameTime(){ return frametime;};
	double StartFrameTime(){ return lastframe;};
	double GetTime();
	DWORD  FPS(){return fps;}
};
//extern aeTimer Timer;
extern boost::scoped_ptr<aeTimer> Timer;
#endif //_ae_Timer_FILE