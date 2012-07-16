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
#ifndef _ae_MATHLIB_FILE
#define _ae_MATHLIB_FILE
#include <math.h>


inline int bound( int a, int b, int c)
{
	return ((a) >= (c) ? (a) : 	(b) < (a) ? (a) : (b) > (c) ? (c) : (b));
}

inline float bound( float a, float b, float c)
{
	return ((a) >= (c) ? (a) : 	(b) < (a) ? (a) : (b) > (c) ? (c) : (b));
}

// up / down
#define	PITCH	0
// left / right
#define	YAW		1
// fall over
#define	ROLL	2

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

struct vec3_t
{
public:
	vec3_t(){}
	vec3_t( const float* xyz):x(xyz[0]),y(xyz[1]),z(xyz[2]){}
	vec3_t( const vec3_t& v):x(v.x),y(v.y),z(v.z){}
	vec3_t( float x, float y, float z):x(x),y(y),z(z){}
	vec3_t( float x, float y ):x(x),y(y),z(1.0){}
	~vec3_t(){}

	bool operator==(const vec3_t & u) const
	{
		return (u.x == x && u.y == y && u.z == z) ? true : false;
	}
	bool operator!=( const vec3_t& rhs ) const
	{
		return !(*this == rhs );
	}
	vec3_t & operator*=( float lambda )
	{
		x*= lambda;
		y*= lambda;
		z*= lambda;
		return *this;
	}

	vec3_t operator - () const
	{
		return vec3_t(-x, -y, -z);
	}
	vec3_t & operator-=(const vec3_t & u)
	{
		x-= u.x;
		y-= u.y;
		z-= u.z;
		return *this;
	}
	vec3_t & operator+=(const vec3_t & u)
	{
		x+= u.x;
		y+= u.y;
		z+= u.z;
		return *this;
	}
	float vlen() const
	{
		return sqrt(x*x + y*y + z*z);
	}
	float normalize()
	{
		float	length, ilength;

		length = vlen();

		if (length)
		{
			ilength = 1/length;
			x *= ilength;
			y *= ilength;
			z *= ilength;
		}
		return length;
	}

	float& operator[](int i)
	{
		return vec_array[i];
	}

	const float operator[](int i) const
	{
		return vec_array[i];
	}
	union {
		struct {
			float x,y,z;        // standard names for components
		};
		struct {
			float s,t,r;        // standard names for components
		};
		float vec_array[3];     // array access
	};
};

struct bbox_t
{
public:
	vec3_t mins,maxs;
	bbox_t(){}
	bbox_t( vec3_t _min, vec3_t _max):mins(_min),maxs(_max)
	{
		FixBBox();
	}
	bbox_t( const bbox_t& b):mins(b.mins),maxs(b.maxs)
	{
		FixBBox();
	}
	bbox_t( float minx,float miny,float minz,float maxx,float maxy,float maxz):
		mins(minx,miny,minz),maxs(maxx,maxy,maxz)
	{
		FixBBox();
	}

	bool operator==(const bbox_t& u) const
	{
		return ( u.mins == mins && u.maxs == maxs) ? true : false;
	}
	bool operator!=( const bbox_t& rhs ) const
	{
		return !(*this == rhs );
	}
	const vec3_t& operator[]( int i ) const
	{
		return (i)? maxs:mins;
	}

	void FixBBox()
	{
		if ( mins.x > maxs.x )
			std::swap( mins.x , maxs.x );
		if ( mins.y > maxs.y )
			std::swap( mins.y , maxs.y );
		if ( mins.z > maxs.z )
			std::swap( mins.z , maxs.z );
	}
};
inline const vec3_t operator+(const vec3_t& u, const vec3_t& v)
{
	return vec3_t( u.x + v.x, u.y + v.y, u.z + v.z);
}


inline const vec3_t operator-(const vec3_t& u, const vec3_t& v)
{
	return vec3_t(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline const vec3_t operator*(const float s, const vec3_t& u)
{
	return vec3_t(s * u.x, s * u.y, s * u.z);
}

inline const vec3_t operator*( const vec3_t& u, const float s)
{
	return vec3_t(s * u.x, s * u.y, s * u.z);
}

inline const vec3_t operator/(const vec3_t& u, const float s)
{
	return vec3_t(u.x / s, u.y / s, u.z / s);
}


/*inline const vec3_t operator^(const vec3_t& u, const vec3_t& v) 
{
	return vec3_t(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}*/

inline const vec3_t CrossProduct(const vec3_t& u, const vec3_t& v) 
{
	return vec3_t(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}


inline const float DotProduct(const vec3_t& u, const vec3_t& v) 
{
	return ( u.x*v.x+u.y*v.y+u.z*v.z );
}


typedef struct  
{
	vec3_t	normal;
	float	dist;
	int		signbits;
}plane_t;

inline float anglemod(float a)
{
	a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}

void AngleVectors ( const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up);
bool isLineCrossBBox( const vec3_t& point, const vec3_t& dir, const bbox_t& bbox, float* dist);
vec3_t RotatePointAroundVector( const vec3_t& point, const vec3_t& vf, const vec3_t& vr, const vec3_t& vup,float alpha);
int BoxOnPlaneSide (const vec3_t &emins, const vec3_t &emaxs, const plane_t& p);
#endif // _ae_MATHLIB_FILE