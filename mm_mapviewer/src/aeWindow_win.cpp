#include "stdafx.h"

aeGLWindow GL_Window;

//extern LRESULT CALLBACK WindowFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void    AppActivate( bool state );

////////////////////////////////////////////////
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define MINWIDTH		640
#define MINHEIGHT		480
#define MINFREQ 		60
////////////////////////////////////////////////
bool aeGLWindow::Init( HINSTANCE hInstance )
{
	hAppInst = hInstance;
	int req_width = GetPrivateProfileInt( "Video", "Width", 800, ".\\config.ini" );
	int req_height = GetPrivateProfileInt( "Video", "Height", req_width * 3 / 4, ".\\config.ini" );
	int isWin = GetPrivateProfileInt( "Video", "Window", 1, ".\\config.ini" );
	int req_freq = GetPrivateProfileInt( "Video", "Freq", MINFREQ, ".\\config.ini" );
	int req_bpp = GetPrivateProfileInt( "Video", "Bpp", 32, ".\\config.ini" );
	int i = 0;
	DEVMODE devmode;
	modelist.push_back( vmode_t( 0, MS_WINDOWED, 640, 480) );
	modelist.push_back( vmode_t( 1, MS_WINDOWED, 800, 600) );
	modelist.push_back( vmode_t( 2, MS_WINDOWED, 1024, 768) );
//	modelist.push_back( vmode_t( 3, MS_WINDOWED, 1280, 960) );
	if(isWin)
	{
	        modelist.push_back( vmode_t( 3, MS_WINDOWED, req_width, req_height) );
	}

	while ( 0 != EnumDisplaySettings( NULL, i++, &devmode ) )
	{
		if ( ( devmode.dmBitsPerPel >= 15 ) &&
		     ( devmode.dmPelsWidth <= MAXWIDTH ) &&
		     ( devmode.dmPelsHeight <= MAXHEIGHT ) &&
		     ( devmode.dmPelsHeight >= MINHEIGHT ) &&
		     ( devmode.dmPelsWidth >= MINWIDTH ) && ( devmode.dmDisplayFrequency >= MINFREQ ) )
		{
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
			if ( ChangeDisplaySettings( &devmode, CDS_TEST | CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
			{
				modelist.push_back( vmode_t( modelist.size(  ),
							     MS_FULLSCREEN,
							     devmode.dmPelsWidth,
							     devmode.dmPelsHeight,
							     devmode.dmBitsPerPel, devmode.dmDisplayFrequency ) );
			}
		}
	}

	Log << boost::format( "Найдено видеорежимов %d") % modelist.size(  ) << aeLog::endl;
	for ( i = 0; i < modelist.size(  ); i++ )
	{

	        Log << boost::format( "%3d: %4dx%4d %s %3dhz") % 
	        i % modelist[i].width % modelist[i].height % 
	        ((modelist[i].type==MS_WINDOWED)?"win ":"full" )% modelist[i].freq << aeLog::endl;
		if ( isWin )
		{
			if ( modelist[i].type != MS_WINDOWED )
				continue;
			if ( modelist[i].width != req_width )
				continue;
			if ( modelist[i].height != req_height )
				continue;
			currentmode = i;
			break;
		}
		if ( modelist[i].type != MS_FULLSCREEN )
			continue;
		if ( modelist[i].width != req_width )
			continue;
		if ( modelist[i].height != req_height )
			continue;
		if ( modelist[i].bpp != req_bpp )
			continue;
		if ( modelist[i].freq != req_freq )
			continue;
		currentmode = i;
		break;

	}
	if ( !isWin && !currentmode )
		throw error( "Подходящий видеорежим не найден\n" );

	Log << boost::format( "Выбран видеорежим # %d: %s") % currentmode % modelist[currentmode].desc << aeLog::endl;
	if ( !CreateGLWindow(  ) )
		throw error( "Can't Create GLWindow" );
	if(0 == GetPrivateProfileInt( "Video", "vsync", 1, ".\\config.ini" ))
	       if(wglSwapIntervalEXT)     wglSwapIntervalEXT(0);
	return true;
}

////////////////////////////////////////////////
extern int IN_MapKey( int key );
static LRESULT CALLBACK WindowFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
//PAINTSTRUCT ps;
	int temp;

	switch ( msg )
	{

	case WM_CREATE:
		break;

	case WM_ACTIVATE:
		AppActivate( ( LOWORD( wParam ) != WA_INACTIVE ) );
		break;
		/*case WM_KILLFOCUS:
		   GL_Window.ActivateWindow(false);
		   //ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
		   break; */

	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	case WM_CLOSE:
		if ( MessageBox
		     ( GL_Window.GetWnd(  ), "Are you sure you want to quit?",
		       "Confirm Exit", MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION ) == IDYES )
		{
			Sys_Quit(  );
		}

		break;

/*	case WM_SIZE:
		break;  */
	case WM_MOVE:
		GL_Window.MoveWindowRect( (int) LOWORD(lParam), (int) HIWORD(lParam));
		break;
	case WM_SYSCOMMAND:
		if ( wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER )
			return 0;
		// fall through


/*	case WM_PAINT:
                  BeginPaint(hWnd, &ps);
                  display();
                  EndPaint(hWnd, &ps);
                break;*/

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		Input.Key_Event( IN_MapKey( lParam ), true );
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		Input.Key_Event( IN_MapKey( lParam ), false );
		break;

	case WM_SYSCHAR:
		// keep Alt-Space from happening
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		temp = 0;
		if ( wParam & MK_LBUTTON )
			temp |= 1;
		if ( wParam & MK_RBUTTON )
			temp |= 2;
		if ( wParam & MK_MBUTTON )
			temp |= 4;
		if ( wParam & MK_XBUTTON1 )
			temp |= 8;
		if ( wParam & MK_XBUTTON2 )
			temp |= 16;
		if ( wParam & MK_XBUTTON3 )
			temp |= 32;
		if ( wParam & MK_XBUTTON4 )
			temp |= 64;
		if ( wParam & MK_XBUTTON5 )
			temp |= 128;
		Input.IN_MouseEvent( temp );
		break;
	case WM_MOUSEWHEEL:
		if ( ( short ) HIWORD( wParam ) > 0 )
		{
			Input.Key_Event( K_MWHEELUP, true );
			Input.Key_Event( K_MWHEELUP, false );
		} else
		{
			Input.Key_Event( K_MWHEELDOWN, true );
			Input.Key_Event( K_MWHEELDOWN, false );
		}
		break;

	default:
		return DefWindowProc( hWnd, msg, wParam, lParam );
	}
	return 0;
}

////////////////////////////////////////////////
bool aeGLWindow::CreateGLWindow(  )
{
	WNDCLASS wcl;
	long ret;
	RECT rect;

	vmode_t & mode = modelist[currentmode];
	Log.Print( "Register window class.................." );
	wcl.hInstance = hAppInst;
	wcl.lpszClassName = "AngelOpenGLWinClass";
	wcl.lpfnWndProc = WindowFunc;
	wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcl.lpszMenuName = NULL;

	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;

	wcl.hbrBackground = ( HBRUSH ) GetStockObject( BLACK_BRUSH ); //WHITE_BRUSH );
	RegisterClass( &wcl );


	if ( mode.type == MS_FULLSCREEN )
	{
		Log.Print( "ChangeDisplaySettings.................." );
		gdevmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		gdevmode.dmBitsPerPel = mode.bpp;
		gdevmode.dmPelsWidth = mode.width;
		gdevmode.dmPelsHeight = mode.height;
		gdevmode.dmSize = sizeof( gdevmode );
		if ( mode.freq )
		{
			gdevmode.dmDisplayFrequency = mode.freq;
			gdevmode.dmFields |= DM_DISPLAYFREQUENCY;
		}
		ret = ChangeDisplaySettings( &gdevmode, CDS_FULLSCREEN );
		if ( ret != DISP_CHANGE_SUCCESSFUL )
			return false;
	}
	rect.top = rect.left = 0;
	rect.right = mode.width + rect.left;
	rect.bottom = mode.height + rect.top;
	window_rect = rect;
	gl_rect = rect;
	aspect = (float)mode.width  / (float)mode.height;

	DWORD WindowStyle;

	if ( mode.type == MS_FULLSCREEN )
	{
		isFullscreen = true;
		WindowStyle = WS_POPUP;
	} else
	{
		isFullscreen = false;
		WindowStyle = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}
	AdjustWindowRectEx( &rect, WindowStyle , FALSE, 0 );

	window_width = rect.right - rect.left;
	window_height = rect.bottom - rect.top;
	MoveWindowRect( rect.left, rect.top );

	Log.Print( "Create Window..........................");
	hWnd = CreateWindowEx( 0, "AngelOpenGLWinClass", "Might&Magic6-8 map viewer by Angel",
			       WindowStyle, CW_USEDEFAULT/*rect.left*/, rect.top, window_width , window_height, NULL, NULL, hAppInst, NULL );

	if ( !hWnd )
		return false;
	
	Log.Print("Get device context.....................");
	hDC =::GetDC( hWnd );
	if ( !hDC )
	{
		return false;
	}
	if ( mode.type != MS_FULLSCREEN )
		CenterWindow();

	Log.Print("Set pixel format.......................");

	if ( !SetWindowPixelFormat(  ) )
		return false;

	Log.Print( "Create GL render context...............");
	hGLRC = wglCreateContext( hDC );
	if ( !hGLRC )
		return false;
	Log.Print( "Activate GL render context...............");
	if(!wglMakeCurrent( hDC, hGLRC ))
	{
		throw error("wglMakeCurrent failed");
		return false;
	}
	GLWinCreated = true;
	ShowWindow( hWnd, SW_SHOWDEFAULT );
	UpdateWindow( hWnd );
/*	HDC hdc = GetDC(hWnd);
	PatBlt(hdc,0,0,window_width,window_height,BLACKNESS);
	ReleaseDC(hWnd, hdc);*/

	return true;
}

void aeGLWindow::MoveWindowRect( int x , int y )
{
	window_x = x;
	window_y = y;
	window_rect.left = window_x;
	window_rect.top = window_y;
	window_rect.right = window_x + window_width;
	window_rect.bottom = window_y + window_height;
	window_center_x = (window_rect.left + window_rect.right) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;
}
void aeGLWindow::CenterWindow()
{
	HMONITOR hMon = MonitorFromWindow( hWnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO minfo = {sizeof(MONITORINFO)};

	if( !GetMonitorInfo(hMon, &minfo))
	{
		Log.Print("Fails get monitor info");
		return;
	}

	int 	CenterX, CenterY;
	CenterX = (minfo.rcMonitor.left + minfo.rcMonitor.right - window_width) / 2;
	CenterY = (minfo.rcMonitor.top + minfo.rcMonitor.bottom - window_height) / 2;
	SetWindowPos (hWnd, NULL, CenterX, CenterY, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);


/*	int 	CenterX, CenterY;
	CenterX = (GetSystemMetrics(SM_CXSCREEN) - window_width) / 2;
	CenterY = (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2;
	if (CenterX > CenterY*2)
		CenterX >>= 1;	// dual screens
	CenterX = (CenterX < 0) ? 0: CenterX;
	CenterY = (CenterY < 0) ? 0: CenterY;
	SetWindowPos (hWnd, NULL, CenterX, CenterY, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);

		*/
}
////////////////////////////////////////////////
void aeGLWindow::Shutdown(  )
{
	wglMakeCurrent( NULL, NULL );

	if ( hGLRC )
	{
		Log.Print( "Release GL render context.............." );
		wglDeleteContext( hGLRC );
		hGLRC = 0;
	}

	if ( hDC && hWnd )
	{
		Log.Print( "Release device context................." );
		ReleaseDC( hWnd, hDC );
		Log.Print( "Destroy window........................." );
		DestroyWindow( hWnd );
		hDC = 0;
		hWnd = 0;
	}
	if ( isFullscreen )
	{
		Log.Print( "Restore display settings..............." );
		ChangeDisplaySettings( NULL, 0 );
	}
	GLWinCreated = false;
}

////////////////////////////////////////////////
bool aeGLWindow::SetWindowPixelFormat(  )
{
	int m_GLPixelIndex;
	PIXELFORMATDESCRIPTOR pfd =											// pfd указывает Windows как устанавливать OpenGL
	{
		sizeof (PIXELFORMATDESCRIPTOR),	// Размер этого Pixel Format Descriptor'а
		1,				// Номер версии
		PFD_DRAW_TO_WINDOW |		// Формат должен поддерживать отрисовку в окно
		PFD_SUPPORT_OPENGL |		// Формат должен поддерживать OpenGL
		PFD_DOUBLEBUFFER,		// Формат должен поддерживать Double Buffering
		PFD_TYPE_RGBA,			// Формат должен поддерживать RGBA режим
		modelist[currentmode].bpp,	// Выбираем глубину цвета
		0, 0, 0, 0, 0, 0,		// Игнорируем биты цвета
		0,				// Нет альфа буфера (пока нету)
		0,				// Игнорируем бит сдвига
		0,				// Нету Accumulation Buffer'а
		0, 0, 0, 0,			// Accumulation биты игнорируем
		24,				// Глубина Z-Buffer'а (Depth Buffer)  
		0,				// Нету Stencil Buffer'а (пока нету)
		0,				// Нету Auxiliary Buffer'а
		PFD_MAIN_PLANE,			// Главный слой для отрисовки
		0,				// Зарезирвировано
		0, 0, 0				// Игнорируем маску слоёв
	};
	pfd.cDepthBits = GetPrivateProfileInt( "Video", "zbuf", 24, ".\\config.ini" );


	m_GLPixelIndex = ChoosePixelFormat( hDC, &pfd );
	if ( m_GLPixelIndex == 0 )	
	{
		/*m_GLPixelIndex = 1;
		if ( DescribePixelFormat( hDC, m_GLPixelIndex, sizeof( PIXELFORMATDESCRIPTOR ), &pfd ) == 0 )
		*/
		throw error("ChoosePixelFormat failed");
		return false;
	}


	if ( SetPixelFormat( hDC, m_GLPixelIndex, &pfd ) == FALSE )
	{
		throw error("SetPixelFormat failed");
		return false;
	}
	DescribePixelFormat(hDC, m_GLPixelIndex, sizeof(pfd), &pfd);
	Log << "PixelFormat: bpp = "<<  (int)pfd.cColorBits << " zbuf = " <<  (int)pfd.cDepthBits << 
		//" StencilBits = " << (int)pfd.cStencilBits << 
		aeLog::endl;
	return true;
}

////////////////////////////////////////////////

float aeGLWindow::CalcFov( float fov_x )
{
	float a;
	float x;

	float width  = gl_rect.right;
	float height = gl_rect.bottom;
	fov_x = bound( 5.0 , fov_x, 130.0 );

	x = width / tan( fov_x / 360 * M_PI );

	a = atan( height / x );

	a = a * 360 / M_PI;

	return a;
}

void MYgluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan( fovy * M_PI / 360.0 );
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

void aeGLWindow::InitGL3D(  )
{
	float fov = Camera.fov();

	glViewport( 0, 0, ( GLsizei ) ( gl_rect.right ), ( GLsizei ) ( gl_rect.bottom  ) );	// Переустанавливаем ViewPort (область видимости)

	glMatrixMode( GL_PROJECTION );	// Выбираем матрицу проекции
	glLoadIdentity(  );	// Сбрасываем её на единичную

	//GLfloat aspect = ( GLfloat ) ( modelist[currentmode].width ) / ( GLfloat ) ( modelist[currentmode].height );

	// Calculate The Aspect Ratio Of The Window
	float fov_y = CalcFov( fov/*, gl_rect.right, gl_rect.bottom*/  );

	//gluPerspective( fov_y * aspect, aspect , 0.1, 4096 );
	extern int zFar;
	MYgluPerspective( fov_y, aspect, 4, zFar*5/4 );
	glMatrixMode( GL_MODELVIEW );	// Выбираем видовую матрицу
	glLoadIdentity(  );	// Сбрасываем её на единичную

	glRotatef( -90, 1, 0, 0 );	// put Z going up
	glRotatef( 90, 0, 0, 1 );	// put Z going up
	glRotatef( -Camera.angles(2), 1, 0, 0 );
	glRotatef( -Camera.angles(0), 0, 1, 0 );
	glRotatef( -Camera.angles(1), 0, 0, 1 );
	glTranslatef( -Camera.pos(0), -Camera.pos(1), -Camera.pos(2) );


	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
}

////////////////////////////////////////////////
void aeGLWindow::InitGL2D(  )
{
	// Переустанавливаем ViewPort (область видимости)
	glViewport( 0, 0, ( GLsizei ) ( gl_rect.right ), ( GLsizei ) ( gl_rect.bottom) );	
	//glViewport( gl_rect.left, gl_rect.top, ( GLsizei ) ( gl_rect.left+gl_rect.right ), ( GLsizei ) ( gl_rect.top+gl_rect.bottom) );	
	// Сохраняем матрицу проекции
	glMatrixMode( GL_PROJECTION );
	// Делаем её единичной
	glLoadIdentity(  );
	// Устанвливаем 2D режим, в зависимости от заданных размеров
	//glOrtho( 0, 1024, 768, 0, -99999, 99999 );
	glOrtho( 0, ( GLsizei ) ( gl_rect.right   ), ( GLsizei ) ( gl_rect.bottom ), 0, -99999, 99999 );
		
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity(  );

	// Отключаем тест глубины
	glDisable( GL_DEPTH_TEST );
	//Запрещаем запись в  буффер глубины
	glDepthMask( FALSE );

	// Включаем прозрачность
	
	// Операция прозрачности
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//	glBlendFunc( GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	//glBlendFunc(GL_ONE,GL_ONE);

//	glEnable( GL_ALPHA_TEST );
//  glAlphaFunc(GL_GREATER,0);	

	glDisable( GL_CULL_FACE );
//  glFrontFace( GL_CCW );


        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	 glColor3f( 1, 1, 1);
}

////////////////////////////////////////////////
//bool skip_update;
void aeGLWindow::EndFrame(  )
{
	glFlush(  );
	//if(!skip_update)
	{
		if(!SwapBuffers( hDC ))
		{
			Log.Print(aeLog::LOG_DEBUG,"failed SwapBuffers");
		}
	}
}

////////////////////////////////////////////////
void aeGLWindow::ActivateWindow( bool state )
{
	v_isActive = state;			
	if ( state )
	{
		if ( isFullscreen )
			ChangeDisplaySettings( &gdevmode, CDS_FULLSCREEN );
		ShowWindow( hWnd, SW_SHOWNORMAL );
	} else
	{
		if ( isFullscreen )
			ChangeDisplaySettings( NULL, 0 );
		ShowWindow( hWnd, SW_SHOWMINNOACTIVE );
	}
}
