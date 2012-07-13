#include "stdafx.h"
#include <limits>

boost::scoped_ptr<aeTimer> Timer;
aeTimer::aeTimer():	maxfps( 72 ), frametime(0), fps_count(0), fps(0),
lastfps_time(0),lastframe(0), starttime (0)
{
	maxfps = GetPrivateProfileInt( "Game", "maxfps", 72, ".\\config.ini" );
	if( maxfps && maxfps < 30 )
		maxfps = 30;
	//timeBeginPeriod( 1 );
	starttime = timeGetTime();
}
aeTimer::~aeTimer()
{
	//timeEndPeriod( 1 );
}

bool aeTimer::StartFrame()
{
	double time = GetTime();
    double delta = time - lastframe;
	
	if( maxfps && (  delta < 1.0 / maxfps ) )
		return false;

	lastframe = time;
	frametime = delta;
	
	if( time - lastfps_time >= 1.0 )
	{
		fps = fps_count;
		fps_count = 0;
		lastfps_time = time;
	}
	fps_count++;

	return true;
}

double  aeTimer::GetTime()
{
	DWORD now = timeGetTime();

	if ( now < starttime ) // wrapped?
		return (now / 1000.0) + ( (LONG_MAX - starttime) / 1000.0);


	if( now - starttime == 0 )
		return 0.0;

	return (now - starttime) / 1000.0;
}
