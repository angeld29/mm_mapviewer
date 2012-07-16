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
aeCamera Camera;

aeCamera::aeCamera( float fov ):
	_angles(0, 90, 0 ), _pos(0,0,0), min_pitch(-70.0), max_pitch( 80.0 ), _fov(fov)
{
	AngleVectors ( _angles, &_forward, &_right, &_up );
	SetFrustum();
}

aeCamera::aeCamera(float minp, float maxp, float fov ):
	_angles(0, 90, 0 ), _pos(0,0,0), min_pitch(minp), max_pitch( maxp ), _fov(fov)
{
	AngleVectors ( _angles, &_forward, &_right, &_up );
	SetFrustum();
}

const vec3_t& aeCamera::Move( float forwardmove, float rightmove, float upmove )
{
	_pos += ( _forward * forwardmove + _right * rightmove + _up * upmove) ;
	SetFrustum();
	return _pos;
}
const vec3_t& aeCamera::Rotate( float yaw, float pitch , float roll)
{
	_angles[PITCH] = bound( min_pitch, _angles[PITCH] + pitch, max_pitch);
	_angles[YAW]   = anglemod ( _angles[YAW] + yaw );
	_angles[ROLL]  = bound( -50.0, _angles[ROLL] + roll, 50.0 );
	AngleVectors ( _angles, &_forward, &_right, &_up );
	SetFrustum();
	return _angles;
}
const vec3_t& aeCamera::SetAngle( const vec3_t& a )
{
	_angles[PITCH] = bound( min_pitch, a[PITCH], max_pitch);
	_angles[YAW]   = anglemod ( a[YAW]);
	_angles[ROLL]  = bound( -50.0 , a[ROLL] , 50.0);
	AngleVectors ( _angles, &_forward, &_right, &_up );
	SetFrustum();
	return _angles;
}


inline int SignbitsForPlane (const vec3_t& normal)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}

void	aeCamera::SetFrustum()
{
	float fov_y = GL_Window.CalcFov( _fov );
	frustum[0].normal = RotatePointAroundVector( Camera.forward(), Camera.up(),Camera.right(),Camera.forward(), -(90 - _fov/2));
	frustum[1].normal = RotatePointAroundVector( Camera.forward(), Camera.up(),Camera.right(),Camera.forward(), (90 - _fov/2));
	frustum[2].normal = RotatePointAroundVector( Camera.forward(), Camera.right(),Camera.up(),Camera.forward(), (90 - fov_y/2));
	frustum[3].normal = RotatePointAroundVector( Camera.forward(), Camera.right(),Camera.up(),Camera.forward(), -(90 - fov_y/2));
	frustum[4].normal = -Camera.forward();

	for ( int i = 0; i<4;i++)
	{
		frustum[i].dist = DotProduct (Camera.pos(), frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane ( frustum[i].normal );
	}
	frustum[4].dist = DotProduct (Camera.pos()+Camera.forward()*zFar, frustum[4].normal);
	frustum[4].signbits = SignbitsForPlane ( frustum[4].normal );
}

/*
=================
CullBox

Returns true if the box is completely outside the frustum
=================
*/
bool	aeCamera::CullBox( const bbox_t& bbox )
{
	for (int i=0 ; i<5 ; i++)
		if (BoxOnPlaneSide (bbox[0], bbox[1], frustum[i]) == 2)
			return true;
	return false;
}

/*
=================
CullSphere

Returns true if the sphere is completely outside the frustum
=================
*/
bool	aeCamera::CullSphere( const vec3_t& centre, float radius )
{
	int		i;
	plane_t *p;

	for (i=0,p=frustum ; i<5; i++,p++)
	{
		if ( DotProduct (centre, p->normal) - p->dist <= -radius )
			return true;
	}
	return false;
}

