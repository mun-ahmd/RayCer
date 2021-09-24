
#include <iostream>

#include "Image.h"
#include "Vec.h"

struct xzc
{
	int i;
	
};


int main()
{

	Image i = Image::load_ppm("C:\\Users\\munee\\Desktop\\Media Encoding Projects\\ppm\\ppmTest.ppm");
	vec3 v({2.f,3.0,0});

	

	i.store_ppm("test.ppm");
	return 0;
}