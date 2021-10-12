#pragma once
#include "Vec.h"

class Ray
{
private:
public:
	Point3 o;	//public cause why not
	vec3 dir;
	Ray() = default;
	Ray(Point3 o, vec3 dir) : o(o), dir(dir) {}
};

struct RayIntersectionQuery
{
	bool did_intersect;
	double ray_parameter;
	Point3 intersection_pos;
	vec3 surface_normal = vec3(1.0, 1.0, 0.0);
	//todo keep adding
};