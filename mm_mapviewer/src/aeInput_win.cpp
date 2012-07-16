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
#include "stdafx.h"

static unsigned char scantokey[128] = {
//  0       1        2       3       4       5       6       7
//  8       9        A       B       C       D       E       F
	0, K_ESCAPE, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', K_BACKSPACE, 9,	// 0
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', K_ENTER, K_CTRL, 'a', 's',	// 1
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', K_SHIFT, '\\', 'z', 'x', 'c', 'v',	// 2
	'b', 'n', 'm', ',', '.', '/', K_SHIFT, KP_STAR,
	K_ALT, ' ', K_CAPSLOCK, K_F1, K_F2, K_F3, K_F4, K_F5,	// 3
	K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE, K_SCRLCK, K_HOME,
	K_UPARROW, K_PGUP, KP_MINUS, K_LEFTARROW, KP_5, K_RIGHTARROW, KP_PLUS, K_END,	// 4
	K_DOWNARROW, K_PGDN, K_INS, K_DEL, 0, 0, 0, K_F11,
	K_F12, 0, 0, 0, 0, 0, 0, 0,	// 5
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	// 6
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0	// 7
};

/*
=======
IN_MapKey

Map from windows to quake keynums
=======
*/
int     cl_keypad = 0;
int IN_MapKey( int key )
{
	int     extended;

	extended = ( key >> 24 ) & 1;

	key = ( key >> 16 ) & 255;

	if ( key > 127 )
		return 0;

	key = scantokey[key];

	if ( cl_keypad )
	{
		if ( extended )
		{
			switch ( key )
			{
			case K_ENTER:
				return KP_ENTER;
			case '/':
				return KP_SLASH;
			case K_PAUSE:
				return KP_NUMLOCK;
			}
		} else
		{
			switch ( key )
			{
			case K_HOME:
				return KP_HOME;
			case K_UPARROW:
				return KP_UPARROW;
			case K_PGUP:
				return KP_PGUP;
			case K_LEFTARROW:
				return KP_LEFTARROW;
			case K_RIGHTARROW:
				return KP_RIGHTARROW;
			case K_END:
				return KP_END;
			case K_DOWNARROW:
				return KP_DOWNARROW;
			case K_PGDN:
				return KP_PGDN;
			case K_INS:
				return KP_INS;
			case K_DEL:
				return KP_DEL;
			}
		}
	} else
	{
		// cl_keypad 0, compatibility mode
		switch ( key )
		{
		case KP_STAR:
			return '*';
		case KP_MINUS:
			return '-';
		case KP_5:
			return '5';
		case KP_PLUS:
			return '+';
		}
	}

	return key;
}

// <-- Tonik

/*===================
aeInput::MouseInput
Called by the system between frames
Should NOT be called during an interrupt!
складывает смещения
===================*/
void aeInput::IN_MouseInput(  )
{
	if ( !active )
		return;
	POINT current_pos;
	int window_center_x = GL_Window.CenterX();
	int window_center_y = GL_Window.CenterY();

	GetCursorPos( &current_pos );
	mousemove.x += current_pos.x - window_center_x;
	mousemove.y += current_pos.y - window_center_y;

	SetCursorPos( window_center_x, window_center_y );
}

/*===================
aeInput::MouseActivate
===================*/
void aeInput::MouseActivate( bool state )
{
	mouse_oldbuttonstate = 0;
	if ( state )
	{
		int window_center_x = GL_Window.CenterX();
		int window_center_y = GL_Window.CenterY();

		SetCursorPos( window_center_x, window_center_y );
		ShowCursor( FALSE );
		SetCapture( GL_Window.GetWnd() );
		ClipCursor (&GL_Window.Rect());
	} else
	{
		ShowCursor( TRUE );
		ReleaseCapture();
		ClipCursor (NULL);
	}
}

/*===================
aeInput::IN_MouseEvent
===================*/
void aeInput::IN_MouseEvent( int mstate )
{
	int i;

	if ( !active )
		return;
	// perform button actions
	for ( i = 0; i < mouse_buttons; i++ )
	{
		if ( ( mstate & ( 1 << i ) ) && !( mouse_oldbuttonstate & ( 1 << i ) ) )
		{
			Key_Event( K_MOUSE1 + i, true );
		}

		if ( !( mstate & ( 1 << i ) ) && ( mouse_oldbuttonstate & ( 1 << i ) ) )
		{
			Key_Event( K_MOUSE1 + i, false );
		}
	}

	mouse_oldbuttonstate = mstate;
}
