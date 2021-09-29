#pragma once
#include "Ray.h"

struct RayIntersectionQuery
{
	bool did_intersect;
	double ray_parameter;
	Point3 intersection_pos;
	vec3 intersection_norm = vec3(1.0,1.0,0.0);
	//todo keep adding
};

class SceneObj
{
public:
	//todo replace bool with a more detailed return
	//todo create another function for most robust test that returns the most info
	//todo create an aabb intersection or a low cost intersection test func
	virtual bool test_intersection(Ray ray) = 0;	// less info intersection
	virtual bool test_intersection(Ray ray, RayIntersectionQuery* query) = 0;	//detailed intersection
	virtual bool test_intersection_low_cost(Ray ray) = 0;
	// low cost intersection test it can say if it is definitely not intersecting but not if it definitely is
	// for eg: aabb intersection tests

	virtual ~SceneObj() {};
	// no leaks yay
};


class SphereObj : public SceneObj
{
private:
	Point3 center;
	double radius;
	
public:
	SphereObj(Point3 center,double radius):center(center),radius(radius) {}
	bool test_intersection(Ray ray)
	{
		double b = 2 * (ray.dir * (ray.o - center));
		double a = ray.dir.magnitude2();
		double c = ((ray.o - center).magnitude2() - radius * radius);
		double discriminant =  ((b * b - 4 * a * c));
		return discriminant >= -0.0001;
	}
	bool test_intersection(Ray ray, RayIntersectionQuery* query)	//detailed intersection
	{
		double b = 2 * (ray.dir * (ray.o - center));
		double a = ray.dir.magnitude2();
		double c = ((ray.o - center).magnitude2() - radius * radius);
		double discriminant = ((b * b - 4 * a * c));
		query->did_intersect = (discriminant >= 0);
		if (query->did_intersect == false)
			return false;
		double disc_sqrt = std::sqrt(discriminant);
		double t1 = (-b + disc_sqrt) / (2 * a);
		double t2 = (-b - disc_sqrt) / (2 * a);
		query->ray_parameter = std::max(t1, t2);
		if (query->ray_parameter < 0)
			return false;
		query->intersection_pos = ray.o + ray.dir * query->ray_parameter;
		query->intersection_norm = query->intersection_pos - center;
		return true; 
	}
	bool test_intersection_low_cost(Ray ray) { return false; }		//todo complete
};