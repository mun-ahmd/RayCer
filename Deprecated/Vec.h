#pragma once

#include <cmath>
#include <fstream>
#include <initializer_list>


template<typename Targ1, typename... Targs>
struct are_arithmetic
{
	static const bool value = std::is_arithmetic<Targ1>::value || are_arithmetic<Targs...>::value;
};

template<typename Targ1_>
struct are_arithmetic<Targ1_>
{
	static const bool value = std::is_arithmetic<Targ1_>::value;
};

template<typename numeric_type, unsigned char num_elements>
class __GeneralVector__
{
	static_assert(std::is_arithmetic<numeric_type>::value, "Only Numeric Templates accepted by __GeneralVector__");
protected:
	numeric_type data[num_elements];

public:
	__GeneralVector__() = default;

	template<unsigned char index, typename T,typename... Ts>
	void _init_set_(T arg,Ts... args)
	{
		static_assert(std::is_arithmetic<T>::value, "Only Numeric Templates accepted by __GeneralVector__");
		data[index] = arg;
		_init_set_<index + 1>(args...);
	}

	template<unsigned char index, typename T>
	void _init_set_(T arg)
	{
		data[index] = arg;
		for (unsigned char i = index + 1; i < num_elements; ++i)
		{
			data[i] = data[0];
		}
	}

	template<typename... Ts>
	__GeneralVector__(numeric_type arg, Ts... args)
	{
		//static_assert(are_arithmetic<numeric_type, T, Ts...>::value);
		//static_assert(sizeof...(Ts) == num_elements - 1);
		_init_set_<0>(arg, args...);
	}


	inline __GeneralVector__<numeric_type, num_elements> operator+(
		__GeneralVector__<numeric_type, num_elements> const& v) const
	{
		__GeneralVector__<numeric_type, num_elements> new_vec;
		for (unsigned char i = 0; i < num_elements; ++i)
			new_vec.data[i] = data[i] + v.data[i];
		return new_vec;
	}

	inline void operator +=(__GeneralVector__<numeric_type, num_elements> const& other)
	{
		for (unsigned char i = 0; i < num_elements; ++i)
			data[i] += other.data[i];
	}

	inline __GeneralVector__<numeric_type, num_elements> operator -(
		__GeneralVector__<numeric_type, num_elements> const& v) const
	{
		__GeneralVector__<numeric_type, num_elements> new_vec;
		for (unsigned char i = 0; i < num_elements; ++i)
			new_vec.data[i] = data[i] - v.data[i];
		return new_vec;
	}

	inline void operator -=(__GeneralVector__<numeric_type, num_elements> const& other)
	{
		for (unsigned char i = 0; i < num_elements; ++i)
			data[i] -= other.data[i];
	}

	inline numeric_type operator *(__GeneralVector__<numeric_type, num_elements> const& v) const
	{
		numeric_type dot_product = 0;
		for (unsigned char i = 0; i < num_elements; ++i)
			dot_product += data[i] * v.data[i];
		return dot_product;
	}

	inline __GeneralVector__<numeric_type,num_elements> operator*(numeric_type t) const
	{
		__GeneralVector__<numeric_type, num_elements> prod = 0;
		for (unsigned char i = 0; i < num_elements; ++i)
			prod.data[i] = data[i] * t;
		return prod;
	}

	inline __GeneralVector__<numeric_type, num_elements> operator/(numeric_type t) const
	{
		return (1 / t) * this;
	}

	inline bool operator ==(__GeneralVector__<numeric_type, num_elements> const& other)
	{
		for (unsigned char i = 0; i < num_elements; ++i)
		{
			if (data[i] != other[i])
				return false;
		}
		return true;
	}

	inline bool operator !=(__GeneralVector__<numeric_type, num_elements> const& other)
	{
		for (unsigned char i = 0; i < num_elements; ++i)
		{
			if (data[i] != other[i])
				return true;
		}
		return false;
	}



	static double fast_inv_sqrt(double n) {

		const double threehalfs = 1.5F;
		double y = n;

		long long i = *(long long*)&y;

		i = 0x5fe6eb50c7b537a9 - (i >> 1);
		y = *(double*)&i;

		y = y * (threehalfs - ((n * 0.5F) * y * y));

		return y;
	}

	__GeneralVector__<numeric_type, num_elements> normalized() const
	{
		double inv_sqrt = __GeneralVector__::fast_inv_sqrt(magnitude2());
		//todo not great way to initalize r_vec
		__GeneralVector__<numeric_type, num_elements> r_vec;
		for (unsigned char i = 0; i < num_elements;++i)
			r_vec.data[i] = data[i] * inv_sqrt;
		return r_vec;
	}

	static inline numeric_type dot(const __GeneralVector__<numeric_type, num_elements>& u, const __GeneralVector__<numeric_type, num_elements>& v)
	{
		numeric_type dot_product = 0;
		for (unsigned char i = 0; i < num_elements; ++i)
			dot_product += u.data[i] * v.data[i];
		return dot_product;
	}

	__GeneralVector__<numeric_type,3> cross(__GeneralVector__<numeric_type, 3> other) const
	{
		static_assert(num_elements >= 3);
		return __GeneralVector__<numeric_type, 3>({
		(this->y() * other.z() - this->z() * other.y()),
		-(this->x() * other.z() - this->z() * other.x()),
		(this->x() * other.y() - this->y() * other.x())
			});
	}

	numeric_type magnitude2() const
	{
		numeric_type mag = 0;
		for (unsigned char i = 0; i < num_elements; ++i)
			mag += data[i] * data[i];
		return mag;
	}

	numeric_type magnitude() const
	{
		return std::sqrt(magnitude2());
	}

	numeric_type& operator[](unsigned char index)
	{
		assert(num_elements > index);
		return data[index];
	}

	const numeric_type& operator[](unsigned char index) const
	{
		assert(num_elements > index);
		return data[index];
	}

	numeric_type& x()
	{
		static_assert(num_elements > 0);
		return data[0];
	}
	numeric_type& y()
	{
		static_assert(num_elements > 1);
		return data[1];
	}
	numeric_type& z()
	{
		static_assert(num_elements > 2);
		return data[2];
	}
	numeric_type& w()
	{
		static_assert(num_elements > 3);
		return data[3];
	}

	const numeric_type& x() const
	{
		static_assert(num_elements > 0);
		return data[0];
	}
	const numeric_type& y() const
	{
		static_assert(num_elements > 1);
		return data[1];
	}
	const numeric_type& z() const
	{
		static_assert(num_elements > 2);
		return data[2];
	}
	const numeric_type& w() const
	{
		static_assert(num_elements > 3);
		return data[3];
	}

};


//template <typename T>
//class __Vector3__ : public __GeneralVector__<T, 3>
//{
//public:
//	using __GeneralVector__<T, 3>::__GeneralVector__;
//	using __GeneralVector__<T, 3>::operator+;
//	__Vector3__<T> cross(__Vector3__<T> other)
//	{
//		__Vector3__<T> new_vec;
//		new_vec.x() = (this->y() * other.z() - this->z() * other.y());
//		new_vec.y() = -(this->x() * other.z() - this->z() * other.x());
//		new_vec.z() = (this->x() * other.y() - this->y() * other.x());
//		return new_vec;
//	}
//};

typedef float default_vector_type;
typedef __GeneralVector__<default_vector_type, 2> vec2;
typedef __GeneralVector__<default_vector_type,3> vec3;
typedef __GeneralVector__<default_vector_type, 4> vec4;

typedef __GeneralVector__<int, 2> ivec2;
typedef __GeneralVector__<int,3> ivec3;
typedef __GeneralVector__<int, 4> ivec4;

typedef __GeneralVector__<unsigned int, 2> uivec2;
typedef __GeneralVector__<unsigned int,3> uivec3;
typedef __GeneralVector__<unsigned int, 4> uivec4;

typedef __GeneralVector__<default_vector_type,3> Point3;