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
	double aspect_ratio;
public:
	SimpleCam(vec3 position, double len_dist, unsigned int width,unsigned int height) :
		position(position), len_dist(len_dist),
		viewport_dimensions(uivec2(width,height)),
		aspect_ratio((double)width/height)
	{ front = vec3(0.0,0.0,1.0); }

	Ray get_cam_ray(const vec2& uv)
	{
		double vert = 2.0;
		double hori = 2.0 * aspect_ratio;
		auto rt = Ray(position, (front * len_dist - vec3(hori / 2, vert / 2, 0) + vec3(uv.x() * hori, uv.y() * vert,0.0) )  );
		return rt;
	}

	const uivec2& get_viewport_dimensions()
	{
		return viewport_dimensions;
	}
};