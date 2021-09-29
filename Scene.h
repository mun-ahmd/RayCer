#pragma once
#include <vector>
#include "SceneObject.h"
#include "Image.h"
#include "Camera.h"

class Scene
{
private:
	std::vector<SceneObj*> objects;
public:
	void simple_trace_scene(Image* img,Camera* cam)
	{
		int hits = 0;
		img->for_each_perform([this,cam,&hits](Color& pix_color, vec2 uv)
			{
				auto cam_ray = cam->get_cam_ray(uv);
				RayIntersectionQuery int_query;
				for (size_t i = 0;i < objects.size(); ++i)
				{
					if (objects[i]->test_intersection(cam_ray,&int_query))
					{
						double sdadas = vec3(100, -20, 45).normalized() * int_query.intersection_norm.normalized();
						pix_color = (vec3(1.0) * std::max(sdadas ,0.0)) + vec3(0.1) ;
						hits++;
						return;
					}
				}
				pix_color = Color(0.0, 0.0, 0.0);
			}, 1);
		std::cout << "\n num hits: " << hits << "\n";
	}
	void add_object(SceneObj* obj)
	{
		objects.push_back(obj);
	}
};