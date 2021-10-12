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
	vec3 up = vec3(0, 1, 0);
	vec3 right = vec3(1, 0, 0);
	vec3 position;
	double len_dist;
	uivec2 viewport_dimensions;
public:
	SimpleCam(vec3 position, double len_dist, unsigned int width,unsigned int height) : position(position), len_dist(len_dist), viewport_dimensions(uivec2(width,height)) { front = vec3(0.0,0.0,1.0); }

	Ray get_cam_ray(const vec2& uv)
	{
		double aspect_ratio = ((double)viewport_dimensions.x() / (double)viewport_dimensions.y());
		double vert = 2.0;
		double hori = 2.0 * aspect_ratio;
		vec3 lower_left = position - vec3(hori / 2, vert / 2, 0);
		auto rt = Ray(position - vec3(0,0,len_dist), lower_left + vec3(uv.x() * hori, uv.y() * vert) - position);
		return rt;
		//vec2 ranged_uv(  (2.0*uv.x() - 1.0) , (2.0*uv.y()-1.0));
		//return Ray(position, vec3((ranged_uv.x())*(double)viewport_dimensions.x() ,(ranged_uv.y())*(double)viewport_dimensions.y() +1.0,len_dist) );
	}

	const uivec2& get_viewport_dimensions()
	{
		return viewport_dimensions;
	}
};