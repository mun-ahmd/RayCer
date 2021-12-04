
#include <iostream>

#include "Image.h"
#include "Vec.h"
#include "Scene.h"
#include "Camera.h"

#include <atomic>

struct xzc
{
	int i;
};


int main()
{
	Scene scn;
	Image img(512,512);
	SimpleCam cam(vec3(0.0), 1.0, img.get_width(), img.get_height());

	vec3 popo(10, 2, 6);

	std::shared_ptr<Material> material = std::make_shared<BasicMaterial>();
	std::shared_ptr<Material> material_metal = std::make_shared<MetalMaterial>();


	SphereSurface sphre(vec3(0.0, 0.0 , 1.981 ), 0.5, material_metal);
	PlaneSurface pln(vec3(0, +0.2, 0), vec3(0, -1, 0) , material);

	SphereSurface sphere_metal(vec3(1.2,0.0, 1.981), 0.5,material);

	scn.add_object(&sphre);
	scn.add_object(&sphere_metal);
	scn.add_object(&pln);

	scn.simple_trace_scene(&img,&cam);
	img.store_bmp("Resources_Outputs\\traced.bmp");
	return 0;
}