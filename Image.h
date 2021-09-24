#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <functional>

#include <assert.h>

#include "Vec.h"



template<typename PixelType>
//PixelType must overload << operator to output to ostream for writing as such
// void operator<<(unsigned char(&rgb)[3]) and write r,g,b to this array
//  PixelType must overload >> operator as such:
//   void operator>>(const unsigned char(&rgb)[3]) to initialize the pixel
//     no other requirement
class __GeneralImage__
{
private:
	unsigned int width;
	unsigned int height;
	std::vector<PixelType> data;

public:

	static __GeneralImage__<PixelType> load_ppm(std::string filedir)
	{
		std::ifstream file;
		file.open(filedir);
		if (!file.is_open())
		{
			//todo handle
			int i = 0;
		}
		bool metadata_loaded = false;
		std::string line_string;
		while (!metadata_loaded)
		{
			std::getline(file, line_string);
			if (line_string[0] == '#')
				continue;
			if (line_string[0] == 'P' && line_string[1] == '3')
				metadata_loaded = true;
		}
		unsigned int width, height;
		unsigned int max_val;
		file >> width >> height >> max_val;
		float max_val_f = (float)max_val;

		__GeneralImage__<PixelType> img(width, height);

		for (unsigned int y = 0; y < height; ++y)
			for (unsigned int x = 0; x < width; ++x)
			{
				unsigned int curr_vals[3];
				file >> curr_vals[0] >> curr_vals[1] >> curr_vals[2];
				unsigned char unsigned_char_curr_vals[3] = { curr_vals[0],curr_vals[1],curr_vals[2] };
				img.at(x, y) >> unsigned_char_curr_vals;
			}

		file.close();
		return img;
	}

	void store_ppm(std::string filedir) const
	{
		std::ofstream file;
		file.open(filedir);
		if (!file.is_open())
		{
			//todo handle
			int i = 0;
		}
		file << "P3" << std::endl << width << " " << height << std::endl << 255 << std::endl;

		for (unsigned int y = 0; y < height; ++y)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				unsigned char curr_rgb[3] = {0,0,0};
				at(x, y) << curr_rgb;
				file << std::to_string(curr_rgb[0]) << " " << std::to_string(curr_rgb[1]) << " " << std::to_string(curr_rgb[2]) << " ";
			}
			file << std::endl;
		}
		file.close();
	}

	__GeneralImage__(unsigned int width, unsigned int height) : width(width), height(height)
	{
		data = std::vector<PixelType>(width * height);
	}

	inline unsigned int get_width() { return width; }
	inline unsigned int get_height() { return height; }

	inline PixelType& at(unsigned int x, unsigned int y)
	{
		assert(x < width&& y < height);
		return data[y * width + x];
	}

	inline const PixelType& at(unsigned int x, unsigned int y) const
	{
		assert(x < width&& y < height);
		return data[y * width + x];
	}

	void for_each_perform(std::function<void(PixelType&)> action)
	{
		for (auto curr_ptr = data.begin(); curr_ptr < data.end(); ++curr_ptr)
			action(data);
	}

	void for_each_perform(std::function<void(const PixelType&)> action) const
	{
		for (auto curr_ptr = data.begin(); curr_ptr < data.end(); ++curr_ptr)
			action(data);
	}

};

typedef double basic_image_type;

class Color : public __Vector3__<double>
{
private:

public:
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
				rgb[i] = static_cast<unsigned char>(255.999 * data[i]);
		}

	}
	void operator>>(const unsigned char(&rgb)[3])
	{
		x() = static_cast<double>(rgb[0])/255.99;
		y() = static_cast<double>(rgb[1])/255.99;
		z() = static_cast<double>(rgb[2])/255.99;
	}
};

typedef __GeneralImage__<Color> Image;