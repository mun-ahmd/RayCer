#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <thread>

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
	
	void perform_action(std::function<void(PixelType&, vec2)> func, unsigned int start_x, unsigned int start_y, unsigned int x_count, unsigned int y_count)
	{
		for (unsigned int y = start_y;y < start_y + y_count;++y)
			for(unsigned int x = start_x; x < start_x + x_count; ++x)
				func(data[y*width + x], vec2{x,y});	//todo major fix this
	}

	std::vector<uivec4> divide_image(int num_parts)
	{
		std::vector<uivec4> parts(num_parts);
		for (unsigned int i = 0; i < num_parts; ++i)
		{

		}
	}

	static void perform_action_const(std::function<void(const PixelType&)> func, const PixelType* start, const PixelType* end)
	{
		for (const PixelType* ptr = start; ptr <= end;++ptr)
		{
			func(*ptr);
		}
	}

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

		metadata_loaded = false;
		unsigned int width, height;
		while (!metadata_loaded)
		{
			std::getline(file, line_string);
			if (line_string[0] == '#')
				continue;
			else
			{
				std::stringstream w_and_h(line_string);
				w_and_h >> width >> height;
				metadata_loaded = true;
			}
		}

		metadata_loaded = false;
		unsigned int max_val;
		while (!metadata_loaded)
		{
			std::getline(file, line_string);
			if (line_string[0] == '#')
				continue;
			else
			{
				std::stringstream max_val_ss(line_string);
				max_val_ss >> max_val;
				metadata_loaded = true;
			}
		}


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

	void store_bmp(std::string filedir) const
	{
		std::ofstream file;
		file.open(filedir);
		assert(file.is_open(), ("unable to open file: " + filedir).c_str());		
		struct
		{
			//header
			char signature1 = 'B';
			char signature2 = 'M';
			unsigned int file_size;
			unsigned int unused_val = 0;
			unsigned int data_offset = 54;

			//info header
			unsigned int info_header_size = 40;
			int width;
			int height;
			unsigned short planes = 1;
			unsigned short bits_per_pixel = 24;
			unsigned int compression = 0;
			unsigned int image_data_size;
			int X_pixels_per_meter = 1000;
			int Y_pixels_per_meter = 1000;
			unsigned int num_colors_used_actual = 256*256*256;
			unsigned int imp_colors = 0;

			void write_header(std::ofstream& file, std::function<void(void*,unsigned int)> lew)
			{
				file.write(&signature1, 1);
				file.write(&signature2, 1);
				lew(&file_size,sizeof(int));
				lew(&unused_val,sizeof(int));
				lew(&data_offset,sizeof(int));
				lew(&info_header_size,sizeof(int));
				lew(&width,sizeof(int));
				lew(&height, sizeof(int));
				lew(&planes,sizeof(short));
				lew(&bits_per_pixel,sizeof(short));
				lew(&compression,sizeof(int));
				lew(&image_data_size,sizeof(int));
				lew(&X_pixels_per_meter,sizeof(int));
				lew(&Y_pixels_per_meter,sizeof(int));
				lew(&num_colors_used_actual,sizeof(int));
				lew(&imp_colors,sizeof(int));
			}
		} bmp_h;

		bmp_h.width = width;
		bmp_h.height = height;
		unsigned int padding_size = (width * 3) % 4;
		bmp_h.image_data_size = width * height * (bmp_h.bits_per_pixel / 8) + height*padding_size;
		
		bmp_h.file_size = bmp_h.data_offset + bmp_h.image_data_size;

		auto dano = bmp_h.image_data_size;
		bmp_h.image_data_size = 0;
		bmp_h.write_header(file, [&file](void* data, unsigned int data_size)
			{
				uint32_t endiannes_check_int = 0x1;
				if (((char*)&endiannes_check_int)[0] == 1)
				{
					//system is little endian
					file.write((char*)data, data_size);
				}
				else
				{
					//system is big endian
					char* bytes = (char*)&data;
					std::reverse(bytes, bytes + data_size);
					file.write(bytes, data_size);
				}
			});
		bmp_h.image_data_size = dano;
		
		unsigned char* _image_data = new unsigned char[bmp_h.image_data_size];
		unsigned int i_data_i = 0;
		for (int y = (this->height - 1); y > -1; y -= 1)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				unsigned char rgb[3] = { 0,0,0 };
				at(x, y) << rgb;
				//bitmap stores as bgr
				_image_data[i_data_i] = rgb[2];
				_image_data[i_data_i + 1] = rgb[1];
				_image_data[i_data_i + 2] = rgb[0];
				i_data_i += 3;
			}

			for (unsigned int i = 0; i < padding_size;++i)
			{
				_image_data[i_data_i] = 0;	//to make divisible by 4
				i_data_i++;
			}
		}

		file.write((char*)_image_data, bmp_h.image_data_size);	//todo check if you need static cast
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

	inline PixelType& at(uivec2 coord)
	{
		assert(coord.x() < width && coord.y() < height);
		return data[coord.y() * width + coord.x()];
	}
	inline const PixelType& at(uivec2 coord) const
	{
		assert(coord.x() < width && coord.y() < height);
		return data[coord.y() * width + coord.x()];
	}

	inline PixelType& at(ivec2 coord)
	{
		assert(coord.x() < width && coord.y() < height);
		return data[coord.y() * width + coord.x()];
	}
	inline const PixelType& at(ivec2 coord) const
	{
		assert(coord.x() < width && coord.y() < height);
		return data[coord.y() * width + coord.x()];
	}

	void for_each_perform(std::function<void(PixelType&,vec2)> action, int num_threads = 1)
	{

		//todo complete implementation
		assert(num_threads == 1, "Multithreading is not ready yet");
		assert(num_threads > 0, "Image::for_each_perform called with num_threads < 1");
		std::vector<std::thread> threads;
		threads.reserve(num_threads - 1);
		for (int i = 0; i < num_threads - 1; ++i)
		{
//			threads.push_back(std::thread(perform_action,action,
//				data.data() + static_cast<int>(data.size() * (static_cast<double>(i) / num_threads)),
//				data.data() + static_cast<int>(data.size() * (static_cast<double>((i + 1)) / num_threads))));
		}
		auto w_reciprocal = 1.0 / (double)width;
		auto h_reciprocal = 1.0 / (double)height;
		for (unsigned int y = 0; y < height; ++y)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				action(at(x, y), vec2(x*w_reciprocal,y*h_reciprocal));
			}
		}
		for (int i = 0; i < threads.size(); ++i)
			threads[i].join();
	}

	void for_each_perform(std::function<void(const PixelType&)> action, int num_threads = 1) const
	{
		assert(num_threads == 1, "Multithreading is not ready yet");
		assert(num_threads > 0, "Image::for_each_perform called with num_threads < 1");
		std::vector<std::thread> threads;
		threads.reserve(num_threads - 1);
		for (int i = 0; i < num_threads - 1; ++i)
		{
			threads.push_back(std::thread(perform_action_const, action,
				data.data() + static_cast<int>(data.size() * (static_cast<double>(i) / num_threads)),
				data.data() + static_cast<int>(data.size() * (static_cast<double>((i + 1)) / num_threads))));
		}
		for (auto i = static_cast<size_t>(data.size() * static_cast<double>((num_threads - 1)) / num_threads); i < data.size()-1; ++i)
			action( ((const PixelType*)data.data())[i] );
		for (int i = 0; i < threads.size(); ++i)
			threads[i].join();
	}

};

typedef double basic_image_type;

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
		x() = static_cast<double>(rgb[0])/255.0;
		y() = static_cast<double>(rgb[1])/255.0;
		z() = static_cast<double>(rgb[2])/255.0;
	}
};

typedef __GeneralImage__<Color> Image;