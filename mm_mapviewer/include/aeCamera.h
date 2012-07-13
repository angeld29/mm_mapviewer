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