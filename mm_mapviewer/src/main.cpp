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
#include "mm_map.h"
#include "odmmap.h"

//float   fov = 90;
float   m_speed = 0.12;
float   speed = 1200;

int zFar = 8196*2;

void Game_Init()
{
	Timer.reset( new aeTimer() );
	Input.Init();
	LodManager.AddLod("data/bitmaps.lod");
	LodManager.AddLod("data/games.lod");
	LodManager.AddLod("data/icons.lod");
	LodManager.AddLod("data/events.lod");
	LodManager.AddLod("data/sprites.lod");
	LodManager.AddLod("data/EnglishD.lod");
	LodManager.AddLod("data/EnglishT.lod");
	GL_Init();
}

void GL_Shutdown();
void Game_Shutdown()
{
	Input.Shutdown();
	GL_Shutdown();
}
void LoadMap(  );
void SetFrustum();
void ApplyInput(  )
{
	int     key;


	while ( key = Input.GetKeyMessage(  ) )
	{
		if ( key == K_ESCAPE )
			Sys_Quit(  );
		if ( key == 'l')
			 LoadMap(  );
		if( pODMMap )
			pODMMap->ProcessInput( key );
		if( pMap )
			pMap->ProcessInput( key );
	}
	point_t pt = Input.GetMouseMove(  );
	Camera.Rotate( -m_speed * pt.x, m_speed * pt.y );

	float   forwardmove = 0, sidemove = 0;
	if ( Input.GetKeyStatus( 'w' ) )
		forwardmove += speed;
	if ( Input.GetKeyStatus( 's' ) )
		forwardmove -= speed;

	if ( Input.GetKeyStatus( 'a' ) )
		sidemove -= speed;
	if ( Input.GetKeyStatus( 'd' ) )
		sidemove += speed;
	if( Input.GetKeyStatus( K_SHIFT ) )
	{
		forwardmove*=3;
		sidemove*=3;
	}
	forwardmove *= Timer->FrameTime();
	sidemove    *= Timer->FrameTime();
	Camera.Move( forwardmove , sidemove );
	if ( pODMMap)
	{
		int posx = Camera.pos(0);
		int posy = Camera.pos(1);
		int posz = Camera.pos(2);
		posx = bound( -0x40*512, posx, 0x40*512 );
		posy = bound( -0x40*512, posy, 0x40*512 );
		posz = bound( -0x100*32, posz, 0x100*32);
		Camera.MoveTo(vec3_t(posx,posy,posz));
	}
}
void Game_Frame( )
{
	Input.ProcessInput(  );
	ApplyInput(  );
	display();
}
