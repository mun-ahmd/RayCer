#pragma once

#include <cmath>
#include <fstream>
#include <initializer_list>

template<typename numeric_type, unsigned char num_elements>
class __GeneralVector__
{
	static_assert(std::is_arithmetic<numeric_type>::value, "Only Numeric Templates accepted by __GeneralVector__");
protected:
	numeric_type data[num_elements];

public:
	__GeneralVector__() = default;

	__GeneralVector__(numeric_type val)
	{
		for (unsigned char i = 0; i < num_elements; ++i)
		{
			data[i] = val;
		}
	}
	__GeneralVector__(const numeric_type(&init)[num_elements])
	{
		for (unsigned char i = 0; i < num_elements; ++i)
		{
			data[i] = num_elements;
		}
	}

	inline __GeneralVector__<numeric_type, num_elements> operator+(
		__GeneralVector__<numeric_type, num_elements> const& v)
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
		__GeneralVector__<numeric_type, num_elements> const& v)
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

	inline numeric_type operator *(__GeneralVector__<numeric_type, num_elements> const& v)
	{
		numeric_type dot_product;
		for (unsigned char i = 0; i < num_elements; ++i)
			dot_product += data[i] * v.data[i];
		return dot_product;
	}

	inline __GeneralVector__<numeric_type,num_elements> operator*(numeric_type t)
	{
		__GeneralVector__<numeric_type, num_elements> v_new;
		for (unsigned char i = 0; i < num_elements; ++i)
			v_new.data[i] = data[i] * t;
		return v_new;
	}

	inline numeric_type operator/(numeric_type t)
	{
		return (1 / t) * this;
	}

	static inline numeric_type dot(const __GeneralVector__<numeric_type, num_elements>& u, const __GeneralVector__<numeric_type, num_elements>& v)
	{
		numeric_type dot_product;
		for (unsigned char i = 0; i < num_elements; ++i)
			dot_product += u.data[i] * v.data[i];
		return dot_product;
	}


	numeric_type magnitude2()
	{
		numeric_type mag;
		for (unsigned char i = 0; i < num_elements; ++i)
			mag += data[i] * data[i];
		return mag;
	}

	numeric_type magnitude()
	{
		return std::sqrt(magnitude2());
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




template <typename T>
class __Vector3__ : public __GeneralVector__<T, 3>
{
public:
	using __GeneralVector__<T, 3>::__GeneralVector__;
	__Vector3__<T> cross(__Vector3__<T> other)
	{
		__Vector3__<T> new_vec;
		new_vec.x() = (this->y() * other.z() - this->z() * other.y());
		new_vec.y() = -(this->x() * other.z() - this->z() * other.x());
		new_vec.z() = (this->x() * other.y() - this->y() * other.x());
		return new_vec;
	}
};

typedef float default_vector_type;

typedef __GeneralVector__<default_vector_type, 2> vec2;
typedef __Vector3__<default_vector_type> vec3;
typedef __GeneralVector__<default_vector_type, 4> vec4;