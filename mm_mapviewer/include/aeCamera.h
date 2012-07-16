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
#ifndef _ae_Camera_FILE
#define _ae_Camera_FILE
#include "stdafx.h"
class aeCamera
{
private:
	vec3_t	_angles;
	vec3_t	_pos;
	vec3_t	_forward, _right, _up;
	float	min_pitch, max_pitch;
	float	_fov;
	plane_t	frustum[5];
	void	SetFrustum();
public:
	
	aeCamera( float fov = 90 );
	aeCamera::aeCamera(float minp, float maxp,  float fov = 90 );
	~aeCamera(){}
	const vec3_t& pos()		{return _pos;}
	const vec3_t& angles()	{return _angles;}
	const vec3_t& forward()	{return _forward;}
	const vec3_t& right()	{return _right;}
	const vec3_t& up()		{return _up;}

	float pos( int i )		{return _pos[i];}
	float fov( )		{return _fov;}
	float angles( int i )	{return _angles[i];}
	float forward( int i )	{return _forward[i];}
	float right( int i )	{return _right[i];}
	float up( int i )		{return _up[i];}

	const vec3_t& MoveTo( const vec3_t& to ) { return (_pos = to); }
	const vec3_t& Move( float forwardmove, float rightmove = 0, float upmove = 0);
	const vec3_t& Rotate( float yaw, float pitch , float roll = 0);
	const vec3_t& SetAngle( const vec3_t& a );

	bool	CullBox( const bbox_t& bbox );
	bool	CullSphere( const vec3_t& centre, float radius );
protected:
	
};
extern aeCamera Camera;

#endif //_ae_Camera_FILE