
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
	Image img(1920,1080);
	SimpleCam cam(vec3(0.0), 0.1,(double)img.get_width()/img.get_height());

	vec3 popo(10, 2, 6);

	SphereObj sphre(vec3(0, 0, 0.75), 0.5);
	scn.add_object(&sphre);

	scn.simple_trace_scene(&img,&cam);
	img.store_bmp("Resources_Outputs\\traced.bmp");
	return 0;
}