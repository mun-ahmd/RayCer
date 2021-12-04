#pragma once
#include "Color.h"
#include "Ray.h"
#include <vector>
#include <tuple>

struct ContributingRay
{
	ContributingRay(Ray ray, double contribution_factor) : ray(ray), contribution_factor(contribution_factor) {}
	Ray ray;
	double contribution_factor;
};

struct EvaluatedMat
{
	Color color;
	double color_fac;
	std::vector<ContributingRay> contributing_rays;
};

class Material
{
public:
	virtual void color(EvaluatedMat* eval,const Ray& hitting_ray,const RayIntersectionQuery& query_results) const = 0;
	virtual ~Material() {};
};

class MetalMaterial : public Material
{
public:
	void color(EvaluatedMat* eval, const Ray& hitting_ray, const RayIntersectionQuery& query_results) const override
	{
		eval->color = Color(0);
		eval->color_fac = 0;
		auto normalized_norm = query_results.surface_normal.normalized();	//todo maybe make sure normal is normalized?
		eval->contributing_rays.push_back(ContributingRay(Ray(query_results.intersection_pos, (hitting_ray.dir -  normalized_norm * (hitting_ray.dir*normalized_norm) * 2 )   ),1.0 ) );
	}
};

class BasicMaterial : public Material
{
public:
	void color(EvaluatedMat* eval, const Ray& hitting_ray, const RayIntersectionQuery& query_results) const override
	{
		//light is coming from camera
		eval->color = Color(1.0, 1.0, 1.0) * std::max((double)(hitting_ray.dir.normalized() * query_results.surface_normal.normalized()), 0.0) + vec3(0.1);
		eval->color_fac = 1.0;
	}
};