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