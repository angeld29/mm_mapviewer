/*

Copyright (C) 2009 Angel (angel.d.death@gmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*
 Window Class
*/
#pragma once

#ifndef _ae_Window_FILE
#define _ae_Window_FILE
#include "stdafx.h"

#define WM_MOUSEWHEEL                   0x020A

#ifndef WM_XBUTTONDOWN 
#  define WM_XBUTTONDOWN                0x020B 
#endif // WM_XBUTTONDOWN 

#ifndef WM_XBUTTONUP 
#  define WM_XBUTTONUP                  0x020C 
#endif // WM_XBUTTONUP 

#ifndef WM_XBUTTONDBLCLK 
#  define WM_XBUTTONDBLCLK              0x020D 
#endif // WM_XBUTTONUP 

#ifndef MK_XBUTTON1 
#  define MK_XBUTTON1                   0x0020 
#endif // MK_XBUTTON1 

#ifndef MK_XBUTTON2 
#  define MK_XBUTTON2                   0x0040 
#endif // MK_XBUTTON2 

#ifndef MK_XBUTTON3 
#  define MK_XBUTTON3                   0x0080 
#endif // MK_XBUTTON3 

#ifndef MK_XBUTTON4 
#  define MK_XBUTTON4                   0x0100 
#endif // MK_XBUTTON4 

#ifndef MK_XBUTTON5 
#  define MK_XBUTTON5                   0x0200 
#endif // MK_XBUTTON5 

typedef enum{MS_WINDOWED, MS_FULLSCREEN, MS_FULLDIB, MS_UNINIT} modestate_t;

class vmode_t
{
  public:
    vmode_t(int modenum_,modestate_t t,int w,int h,int bpp_=0,int freq_=0)
     {
      type=t;width=w;height=h;bpp=bpp_;freq=freq_;
      modenum=modenum_;
       switch(t)
       {
        case MS_FULLSCREEN:sprintf(desc,"Fullscreen %dx%d %d bpp %d Hz",w,h,bpp_,freq_);break;
        case MS_WINDOWED:  sprintf(desc,"Windowed %dx%d",w,h);break;
        default:sprintf(desc,"none %dx%d",w,h);break;
       }
     };
    ~vmode_t(){};
     modestate_t    type;
     int	    width;
     int	    height;
     int	    bpp;
     int            freq;
     int            modenum;
     char	    desc[50];
};
class aeGLWindow
{
  private:
   bool SetWindowPixelFormat();
   bool CreateGLWindow( void );
   HINSTANCE hAppInst;
   HWND hWnd;
   HGLRC hGLRC;
   HDC hDC;
   DEVMODE gdevmode;
   bool isFullscreen,v_isActive,GLWinCreated;
   int currentmode;
   std::vector<vmode_t> modelist;
   int			window_center_x, window_center_y, window_x, window_y, window_width, window_height;
   RECT			window_rect, gl_rect;
   GLfloat		aspect;
   void			MoveWindowRect( int x, int y);
   friend static LRESULT CALLBACK WindowFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
   void			CenterWindow();
  public:
    aeGLWindow():hAppInst(0),hWnd(0),hGLRC(0),hDC(0),currentmode(0),GLWinCreated(false){}
    ~aeGLWindow(){if(GLWinCreated)Shutdown();}
    bool Init(HINSTANCE hInstance);//enumerate modes
    
    void Shutdown();

    //int GetNumModes(){return modelist.size();}
    //const vmode_t& operator[](int mode){return modelist[mode];}

    HINSTANCE   GetInstance(){return hAppInst;}
    HWND        GetWnd(){return hWnd;}
    //HGLRC GetGLRC(){return hGLRC;}
    //HDC GetDC(){return hDC;}
    //int GetCurrentModeN(){return currentmode;}
    //const vmode_t& GetCurrentMode(){return modelist[currentmode];}
	int         Width() { return modelist[currentmode].width;}
	int         Height() { return modelist[currentmode].height;}
	int			CenterX() { return window_center_x;}
	int			CenterY() { return window_center_y;}
	float		CalcFov( float fov_x);
	const RECT& Rect() {return window_rect;}
    bool        isActive(){return v_isActive;}
    void        ActivateWindow(bool state);
    void        InitGL3D();
    void        InitGL2D();
    void        EndFrame();
};

extern aeGLWindow GL_Window;
#endif //_ae_Window_FILE
