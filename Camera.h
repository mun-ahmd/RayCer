#pragma once

#include "Vec.h"
#include "Ray.h"

class Camera
{
public:
	virtual Ray get_cam_ray(const vec2& uv) = 0;
	virtual ~Camera() {};
};

class SimpleCam : public Camera
{
private:
	vec3 front;
	vec3 position;
	double len_dist;
	double aspect_ratio;	//width over height;
public:
	SimpleCam(vec3 position, double len_dist, double aspect_ratio) : position(position), len_dist(len_dist), aspect_ratio(aspect_ratio) { front = vec3(0.0,0.0,1.0); }

	Ray get_cam_ray(const vec2& uv)
	{
		vec2 ndc_uv(  (2.0*uv.x() - 1.0) , (2.0*uv.y()-1.0)/aspect_ratio );
		auto sda = vec3(ndc_uv.x(), ndc_uv.y(), 0.0) + front * len_dist;
		return Ray(position, vec3(ndc_uv.x(), ndc_uv.y(),0.0 ) + front * len_dist);
	}
};