#include "stdafx.h"

aeLog::aeLog( msglevel_t msglev ):msglev(msglev),currentlev( LOG_NORMAL )
{

}

aeLog::~aeLog( )
{
	Shutdown();
}

void aeLog::Init( const char* fname)
{
	logfile.open(fname);
	if( !logfile )
		throw error( (boost::format("aeLog:cannot open %s") % fname).str() );
	logfile << "Log started\n";
}

void aeLog::Shutdown()
{
	if( sstr.str().size() )
	{
		operator<<(endl);
	}
	logfile << "Log shutdown\n";
	logfile.close();
}
void aeLog::Print( msglevel_t level, const char* text )
{
	if( level < msglev )
		return;

	logfile << text << std::endl;
}

void aeLog::Print( msglevel_t level, const std::string& text )
{
	if( level < msglev )
		return;
	logfile << text << std::endl;
}

aeLog& aeLog::operator<< ( const logcontrol_t val )
{
	switch( val ) 
	{
	case endl:
		Print( currentlev, sstr.str() );
		currentlev = LOG_NORMAL;
		sstr.str("");
		break;
	case normal:
		currentlev = LOG_NORMAL;
		break;
	case debug:
		currentlev = LOG_DEBUG;
		break;
	default:
		break;
	}
	return *this;
}

aeLog Log;
