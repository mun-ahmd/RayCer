#pragma once
#include <vector>
#include "Surface.h"
#include "Image.h"
#include "Camera.h"

class Scene
{
private:
	std::vector<Surface*> objects;
public:
	static const unsigned short MAX_RECURSION_DEPTH = 4;

	template<unsigned short recursion_depth = 0>
	void ray_eval(const Ray& ray,Color& color_r,const Ray* origin_cam_ray = nullptr)
	{
		color_r = Color(0);
		RayIntersectionQuery intersection_query;
		for (size_t i = 0;i < objects.size(); ++i)
		{
			if (objects[i]->test_intersection(ray, &intersection_query))
			{
				//double sdadas = vec3(100, -20, 45).normalized() * int_query.surface_normal.normalized();
				//pix_color = (vec3(1.0) * std::max(sdadas ,0.0)) + vec3(0.1) ;
				//pix_color = int_query.surface_normal*0.5 + vec3(0.5);
				EvaluatedMat eval;
				objects[i]->get_material()->color(&eval, ray, intersection_query);
				color_r = eval.color * eval.color_fac;

				//todo test below for loop, with a material that gives contributing rays
				for (auto cr_itr = eval.contributing_rays.begin(); cr_itr < eval.contributing_rays.end(); ++cr_itr)
				{
					Color color_to_add = Color(0);
					if (origin_cam_ray == nullptr)
						origin_cam_ray = &ray;
					ray_eval<recursion_depth + 1>(cr_itr->ray, color_to_add,origin_cam_ray);
					color_r += color_to_add * cr_itr->contribution_factor;
				}
				return;
			}
		}
		color_r = Color(0.521, 0.807, 0.982) * (std::clamp((double)ray.dir.y(),-1.0,1.0) * 0.5 + 0.5 + 0.08) ;
	}

	template<>
	void ray_eval<MAX_RECURSION_DEPTH>(const Ray& ray, Color& color_r,const Ray* origin_cam_ray)
	{
		vec3 news = Color(0.521, 0.807, 0.982) * (std::clamp((double)origin_cam_ray->dir.y(), -1.0, 1.0) * 0.5 + 0.5 + 0.08);
		color_r = Color(news.x(), news.y(), news.z());
	}

	void simple_trace_scene(Image* img,Camera* cam)
	{
		int hits = 0;
		//todo make the for each perform function to take variadic templated args, no need to capture
		img->for_each_perform([this,cam,&hits](Color& pix_color, vec2 uv)
			{
				auto cam_ray = cam->get_cam_ray(uv);
				ray_eval(cam_ray, pix_color);
			}, 1);
		std::cout << "\n num hits: " << hits << "\n";
	}
	void add_object(Surface* obj)
	{
		objects.push_back(obj);
	}
};