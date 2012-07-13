#include "stdafx.h"
class aeLog
{
public:
	typedef enum {LOG_DEBUG, LOG_NORMAL, LOG_NONE} msglevel_t;
	typedef enum {endl, normal, debug} logcontrol_t;
	aeLog( msglevel_t msglev = LOG_NORMAL );
	~aeLog();
	void SetMsgLevel( msglevel_t mlev )
	{
		msglev = mlev;
	}
	void Init( const char* fname);
	void Print( msglevel_t level, const char* text);
	void Print( msglevel_t level, const std::string& text );
	void Print( const char* text)
	{
		Print( LOG_NORMAL, text );
	}
	void Print( const std::string& text )
	{
		Print( LOG_NORMAL, text );
	}
	void Shutdown();
	template<class T>
	aeLog& operator<< ( const T& val )
	{
		sstr << val;
		return *this;
	}

	aeLog& operator<< ( int val )
	{
		sstr << val;
		return *this;
	}

	aeLog& operator<< ( float val )
	{
		sstr << val;
		return *this;
	}

	aeLog& operator<< ( const char* val )
	{
		sstr << val;
		return *this;
	}

	aeLog& operator<< ( const logcontrol_t val );
	
	private:
std::ofstream	logfile;
		std::ostringstream sstr;
		msglevel_t		msglev;
		msglevel_t		currentlev;
};
extern aeLog Log;
