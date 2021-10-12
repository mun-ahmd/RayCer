#pragma once
#include "Ray.h"
#include <memory>
#include "Material.h"

class Surface
{
protected:
	std::shared_ptr<Material>  material;
public:
	//todo replace bool with a more detailed return
	//todo create another function for most robust test that returns the most info
	//todo create an aabb intersection or a low cost intersection test func
	Surface(std::shared_ptr<Material> material) : material(material) {}
	virtual bool test_intersection(Ray ray) = 0;	// less info intersection
	virtual bool test_intersection(Ray ray, RayIntersectionQuery* query) = 0;	//detailed intersection
	virtual bool test_intersection_low_cost(Ray ray) = 0;
	std::shared_ptr<const Material> get_material()
	{
		return material;
	}
	// low cost intersection test it can say if it is definitely not intersecting but not if it definitely is
	// for eg: aabb intersection tests

	virtual ~Surface() {};
	// no leaks yay
};


class SphereSurface : public Surface
{
private:
	Point3 center;
	double radius;
	
public:
	SphereSurface(Point3 center,double radius,std::shared_ptr<Material> material_):center(center),radius(radius) , Surface(material_) {}
	bool test_intersection(Ray ray) override
	{
		double b = 2 * (ray.dir * (ray.o - center));
		double a = ray.dir.magnitude2();
		double c = ((ray.o - center).magnitude2() - radius * radius);
		double discriminant =  ((b * b - 4 * a * c));
		return discriminant >= -0.0001;
	}
	bool test_intersection(Ray ray, RayIntersectionQuery* query) override	//detailed intersection
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
		query->surface_normal = query->intersection_pos - center;
		return true; 
	}
	bool test_intersection_low_cost(Ray ray) override { return false; }		//todo complete
};

class PlaneSurface : public Surface
{
private:
	Point3 point_on_plane;
	vec3 normal;
	double d;
public:

	PlaneSurface(Point3 point_on_plane, vec3 normal,std::shared_ptr<Material> material_) :
		point_on_plane(point_on_plane),
		normal(normal),
		d(normal*point_on_plane),
		Surface(material_)
	{}

	bool test_intersection(Ray ray) override
	{
		return (vec3::dot(ray.dir, normal) != 0);
	}
	bool test_intersection(Ray ray, RayIntersectionQuery* query) override	//detailed intersection
	{
		query->did_intersect = test_intersection(ray);
		if (!query->did_intersect)
			return false;
		query->ray_parameter = (d - normal * ray.o)/(ray.dir*normal);
		if (query->ray_parameter < 0)
		{
			query->did_intersect = false;
			return false;
		}
		query->surface_normal = normal;
		query->intersection_pos = ray.o + ray.dir * query->ray_parameter;
		return true;
	}
	bool test_intersection_low_cost(Ray ray) override { return false; }		//todo complete
};