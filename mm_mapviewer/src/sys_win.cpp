// agame.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "agame.h"
#include <odmmap.h>
#include <mm_map.h>


static HANDLE tevent,aeSemaphore;
const char* SemaphoreName = "AngelGame";

#define PAUSE_SLEEP		50	// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP	20	// sleep time when not focus

void Sys_Init()
{
	aeSemaphore = CreateMutex( NULL,	/* Security attributes */
				   0,	/* owner                 */
				   SemaphoreName );	/* Semaphore name               */
	if ( !aeSemaphore )
	        throw error("AngelGame is already running on this system");

	CloseHandle( aeSemaphore );

	aeSemaphore = CreateSemaphore( NULL,	/* Security attributes */
				       0,	/* Initial count                 */
				       1,	/* Maximum count                 */
				       SemaphoreName );	/* Semaphore name               */

	tevent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( !tevent )
	        throw error("Couldn't create event");
}
void Sys_Shutdown( void )
{
	if ( tevent )
		CloseHandle( tevent );
	if ( aeSemaphore )
		CloseHandle( aeSemaphore );
}

/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/
void SleepUntilInput( int time )
{

	MsgWaitForMultipleObjects( 1, &tevent, FALSE, time, QS_ALLINPUT );
}

void AppActivate( bool state )
{
	GL_Window.ActivateWindow( state );
	Input.Activate( state );
}

static bool quit = false;
void Sys_Quit(  )
{
        quit = true;
}

//extern bool skip_update;
void Sys_SendKeyEvents( void )
{
	MSG     msg;

	while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		// we always update if there are any event, even if we're paused
//		skip_update = false;
		if ( !GetMessage( &msg, NULL, 0, 0 ) )
			Sys_Quit(  );
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}

/*
==================
WinMain
==================
*/
/*void term_func()
{
	Log.Print( "terminate" );
	Game_Shutdown();
	GL_Window.Shutdown(  );
	Log.Shutdown();
	Sys_Shutdown();
	exit(-1);
}

*/
/////////////////////////////////////
HINSTANCE  hinst;
int WINAPI WinMain( HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR str, int nWinMode )
{
	hinst = hThisInst;
//	set_terminate( term_func );
	try
	{
			Sys_Init();
			Log.Init("console.log");
			if(GetPrivateProfileInt( "Game", "debug", 0, ".\\config.ini" ))
				Log.SetMsgLevel(aeLog::LOG_DEBUG);
			GL_Window.Init( hThisInst );
         	Game_Init(  );
			Log.Print( "Запуск игрового цикла.................." );
         	
         	while ( !quit )
         	{
         		if ( !GL_Window.isActive(  ) )
         		{
         			SleepUntilInput( NOT_FOCUS_SLEEP );
//					skip_update = true;
         		}
				if( Timer->StartFrame() )
         			  Game_Frame( );
         	}
	}
	catch( std::exception& e )
	{
		MessageBox( NULL, e.what(),"Game Error", MB_OK);
		Log << "Error: " << e.what() << aeLog::endl;
	}
#ifndef _DEBUG	
	catch(...)
	{  
			MessageBox( NULL,  "Unknown error","Game Error",MB_OK);
			Log.Print( "Error: Unknown error" );
	}
#endif
	Game_Shutdown();
	GL_Window.Shutdown(  );
	Log.Shutdown();
	Sys_Shutdown();
	return 0;
}
std::vector <std::string> fl;
BOOL CALLBACK SelectMap( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	int     i;

	switch ( message )
	{
	case WM_INITDIALOG:
		{
			char str[0x100];

			SendDlgItemMessage( hwndDlg, IDC_MAPLIST, LB_RESETCONTENT, 0, 0 );
			int selected = 0;
			for ( int i = 0; i< fl.size(); ++i)
			{
				strncpy(str,fl[i].c_str(),sizeof(str));
				if( pMap && fl[i] == pMap->MapName() )
					selected = i;
				if( pODMMap && fl[i] == pODMMap->MapName() )
					selected = i;
				SendDlgItemMessage( hwndDlg, IDC_MAPLIST, LB_ADDSTRING, 0,( LPARAM ) &str );
				
			}
			SendDlgItemMessage( hwndDlg, IDC_MAPLIST, LB_SETCURSEL, selected, 0 );

			int 	CenterX, CenterY;
			WINDOWINFO wi;
			GetWindowInfo(hwndDlg, &wi);
			CenterX = (GetSystemMetrics(SM_CXSCREEN) - (wi.rcWindow.left - wi.rcWindow.right )) / 2;
			CenterY = (GetSystemMetrics(SM_CYSCREEN) - (wi.rcWindow.bottom - wi.rcWindow.top)) / 2;
			
			if (CenterX > CenterY*2)
				CenterX >>= 1;	// dual screens
			CenterX = (CenterX < 0) ? 0: CenterX;
			CenterY = (CenterY < 0) ? 0: CenterY;
			SetWindowPos (hwndDlg, NULL, CenterX, CenterY, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
		}
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDOK:
			EndDialog( hwndDlg, SendDlgItemMessage( hwndDlg, IDC_MAPLIST, LB_GETCURSEL, 0, 0 ) );
			// Fall through. 
			break;
		case IDCANCEL:
			EndDialog( hwndDlg, -1 );
			break;
		case IDC_MAPLIST:
			switch(HIWORD(wParam)) 
			{
			case LBN_DBLCLK:
				EndDialog( hwndDlg, SendDlgItemMessage( hwndDlg, IDC_MAPLIST, LB_GETCURSEL, 0, 0 ) );
				break;
			default:
				break;
			}
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
////////////////////

void LoadMap()
{
	static const boost::regex re("maps/.+\\.(blv|odm)");
	fl.clear();
	LodManager.GetFileList( re, &fl);
	if( !fl.size() )
		throw error("map files not found");

	int    ret = DialogBox( hinst, MAKEINTRESOURCE( IDD_SELECTMAP ), NULL,( DLGPROC ) SelectMap);
	if( ret == -1 )
	{	if( !pODMMap && !pMap )
			throw error("cannot load map");
		else
			return;
	}
	int size;

	BYTE   *data  = LodManager.LoadFile( fl[ret], &size);
	if ( !data )
	{
		throw error("cannot load map");
	}
	Camera.MoveTo(vec3_t(0,0,100));
	if( fl[ret].substr(fl[ret].size()-4,4) == ".odm" )
	{
		pODMMap.reset( new ODMmap( data, size, fl[ret].c_str()));
		pMap.reset();
	}
	else
	{
		pMap.reset( new MMmap( data, size, fl[ret].c_str() ));
		pODMMap.reset();
	}
	delete[]data;
}
