#pragma once
#include "Vec.h"


class Color : public vec3
{
private:

public:
	using vec3::__GeneralVector__;
	void operator<<(unsigned char(&rgb)[3]) const
	{
		//rgb[0] = static_cast<unsigned char>(255.999 * this->x());
		//rgb[1] = static_cast<unsigned char>(255.999 * y());
		//rgb[1] = static_cast<unsigned char>(255.999 * z());

		//TODO
		//I much prefer the above (commented) version
		//replace the below version later


		for (unsigned char i = 0; i < 3; ++i)
		{
			if (data[i] > 1.0)
				rgb[i] = 255;
			else if (data[i] < 0.0)
				rgb[i] = 0;
			else
				rgb[i] = static_cast<unsigned char>(255 * data[i]);
		}

	}
	void operator>>(const unsigned char(&rgb)[3])
	{
		x() = static_cast<double>(rgb[0]) / 255.0;
		y() = static_cast<double>(rgb[1]) / 255.0;
		z() = static_cast<double>(rgb[2]) / 255.0;
	}
	void operator=(vec3 v)
	{
		data[0] = v.x();
		data[1] = v.y();
		data[2] = v.z();
	}
};