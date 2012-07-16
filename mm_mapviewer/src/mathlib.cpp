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

void AngleVectors ( const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up)
{
    float   angle, sr, sp, sy, cr, cp, cy, temp;

	if ( angles[YAW] )
	{
		angle = DEG2RAD( angles[YAW] );
		sy = sin( angle );
		cy = cos( angle );
	} else
	{
		sy = 0;
		cy = 1;
	}

	if ( angles[PITCH] )
	{
		angle = DEG2RAD( angles[PITCH] );
		sp = sin( angle );
		cp = cos( angle );
	} else
	{
		sp = 0;
		cp = 1;
	}

	if ( forward )
	{
		forward->vec_array[0] = cp * cy;
		forward->vec_array[1] = cp * sy;
		forward->vec_array[2] = -sp;
	}

	if ( right || up )
	{

		if ( angles[ROLL] )
		{
			angle = DEG2RAD( angles[ROLL] );
			sr = sin( angle );
			cr = cos( angle );

			if ( right )
			{
				temp = sr * sp;
				right->vec_array[0] = -1 * temp * cy + cr * sy;
				right->vec_array[1] = -1 * temp * sy - cr * cy;
				right->vec_array[2] = -1 * sr * cp;
			}

			if ( up )
			{
				temp = cr * sp;
				up->vec_array[0] = ( temp * cy + sr * sy );
				up->vec_array[1] = ( temp * sy - sr * cy );
				up->vec_array[2] = cr * cp;
			}
		} else
		{
			if ( right )
			{
				right->vec_array[0] = sy;
				right->vec_array[1] = -cy;
				right->vec_array[2] = 0;
			}

			if ( up )
			{
				up->vec_array[0] = sp * cy;
				up->vec_array[1] = sp * sy;
				up->vec_array[2] = cp;
			}
		}
	}

}

inline bool PointInFace( const vec3_t point, const bbox_t& bbox, int notcoord )
{
	for ( int i = 0; i < 3; i++)
	{
		if( i == notcoord )
			continue;
		if( (point[i] < bbox[0][i]) || (point[i] > bbox[1][i]))
			return false;
	}
	return true;
}
bool isLineCrossBBox( const vec3_t& point, const vec3_t& dir, const bbox_t& bbox, float* dist)
{
	vec3_t cp;
//	*dist = 0;

	for ( int i = 0 ; i< 3; i++)
	{
		float pdist = bbox[0][i] - point[i];
		float pdist1 = bbox[1][i] - point[i];
		if( dir[i] == 0 )
		{
			if( pdist * pdist1 >0 )
				return false;
		}else
		{
			
			float len = pdist / dir[i];
			float len1 = pdist1 / dir[i];

			if( len < 0 )
			{
				if( len1 < 0 )
					return false;
				else
					continue;
			}
			if( len1 < 0 )
				continue;

			if( len1 < len )
				len = len1;

			cp = point + ( dir * len );
			if( PointInFace( cp, bbox, i ))
			{
				*dist = len;
				return true;
			}
		}
	}
	return false;
}

/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
}

vec3_t RotatePointAroundVector( const vec3_t& point, const vec3_t& vf, const vec3_t& vr, const vec3_t& vup,float alpha)
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
//	PerpendicularVector( vr, dir );
//	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] = cos( DEG2RAD( alpha ) );
	zrot[0][1] = sin( DEG2RAD( alpha ) );
	zrot[1][0] = -sin( DEG2RAD( alpha ) );
	zrot[1][1] = cos( DEG2RAD( alpha ) );

	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	vec3_t dst;
	for ( int i = 0; i < 3; i++ )
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
	return dst;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide (const vec3_t &emins, const vec3_t &emaxs, const plane_t& p)
{
	float	dist1, dist2;
	// general case
	switch (p.signbits)
	{
	case 0:
		dist1 = p.normal[0]*emaxs[0] + p.normal[1]*emaxs[1] + p.normal[2]*emaxs[2];
		dist2 = p.normal[0]*emins[0] + p.normal[1]*emins[1] + p.normal[2]*emins[2];
		break;
	case 1:
		dist1 = p.normal[0]*emins[0] + p.normal[1]*emaxs[1] + p.normal[2]*emaxs[2];
		dist2 = p.normal[0]*emaxs[0] + p.normal[1]*emins[1] + p.normal[2]*emins[2];
		break;
	case 2:
		dist1 = p.normal[0]*emaxs[0] + p.normal[1]*emins[1] + p.normal[2]*emaxs[2];
		dist2 = p.normal[0]*emins[0] + p.normal[1]*emaxs[1] + p.normal[2]*emins[2];
		break;
	case 3:
		dist1 = p.normal[0]*emins[0] + p.normal[1]*emins[1] + p.normal[2]*emaxs[2];
		dist2 = p.normal[0]*emaxs[0] + p.normal[1]*emaxs[1] + p.normal[2]*emins[2];
		break;
	case 4:
		dist1 = p.normal[0]*emaxs[0] + p.normal[1]*emaxs[1] + p.normal[2]*emins[2];
		dist2 = p.normal[0]*emins[0] + p.normal[1]*emins[1] + p.normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p.normal[0]*emins[0] + p.normal[1]*emaxs[1] + p.normal[2]*emins[2];
		dist2 = p.normal[0]*emaxs[0] + p.normal[1]*emins[1] + p.normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p.normal[0]*emaxs[0] + p.normal[1]*emins[1] + p.normal[2]*emins[2];
		dist2 = p.normal[0]*emins[0] + p.normal[1]*emaxs[1] + p.normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p.normal[0]*emins[0] + p.normal[1]*emins[1] + p.normal[2]*emins[2];
		dist2 = p.normal[0]*emaxs[0] + p.normal[1]*emaxs[1] + p.normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		throw error("BoxOnPlaneSide:  Bad signbits");
		break;
	}
	int		sides = 0;
	if (dist1 >= p.dist)
		sides = 1;
	if (dist2 < p.dist)
		sides |= 2;
	return sides;
}