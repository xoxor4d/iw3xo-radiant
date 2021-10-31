#include "std_include.hpp"
#include "vector.hpp"
#include "cmath"

namespace utils
{
	namespace vector // https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c
	{
		float q_rsqrt(float number)
		{
			long i;
			float x2, y;
			const float threehalfs = 1.5F;

			x2 = number * 0.5F;
			y = number;
			i = *(long *)&y;						// evil floating point bit level hacking
			i = 0x5f3759df - (i >> 1);              // what the fuck?
			y = *(float *)&i;
			y = y * (threehalfs - (x2 * y * y));    // 1st iteration
			return y;
		}

		int compare(const vec3_t v1, const vec3_t v2) 
		{
			if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) 
			{
				return 0;
			}

			return 1;
		}

		void clamp_vec3(const vec3_t a, const vec3_t b, vec3_t o1, vec3_t o2)
		{
			if (a[0] < o1[0]) o1[0] = a[0];
			if (a[1] < o1[1]) o1[1] = a[1];
			if (a[2] < o1[2]) o1[2] = a[2];
			
			if (b[0] > o2[0]) o2[0] = b[0];
			if (b[1] > o2[1]) o2[1] = b[1];
			if (b[2] > o2[2]) o2[2] = b[2];
		}

		vec_t length(const vec3_t v) 
		{
			return (vec_t)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		}

		vec_t length_squared(const vec3_t v) 
		{
			return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		}

		vec_t distance(const vec3_t p1, const vec3_t p2) 
		{
			vec3_t	v;

			VectorSubtract(p2, p1, v);
			return length(v);
		}

		vec_t distance_squared(const vec3_t p1, const vec3_t p2) 
		{
			vec3_t	v;

			VectorSubtract(p2, p1, v);
			return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		}

		// zero's input vector
		void zero(vec3_t v1)
		{
			v1[0] = 0.0f;
			v1[1] = 0.0f;
			v1[2] = 0.0f;
		}

		void set_vec3(vec3_t v, float val)
		{
			v[0] = val;
			v[1] = val;
			v[2] = val;
		}
		
		void set_vec3(vec3_t v, float x, float y, float z)
		{
			v[0] = x;
			v[1] = y;
			v[2] = z;
		}

		void set_vec4(vec4_t v, float val)
		{
			v[0] = val;
			v[1] = val;
			v[2] = val;
			v[3] = val;
		}
		
		void set_vec4(vec4_t v, float x, float y, float z, float w)
		{
			v[0] = x;
			v[1] = y;
			v[2] = z;
			v[3] = w;
		}

		// inverse input vector
		void inverse(vec3_t v)
		{
			v[0] = -v[0];
			v[1] = -v[1];
			v[2] = -v[2];
		}

		// cross product of v1 / v2, result stored in cross
		void cross_product(const vec3_t v1, const vec3_t v2, vec3_t cross)
		{
			cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
			cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
			cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
		}

		// fast vector normalize routine that does not check to make sure
		// that length != 0, nor does it return length, uses rsqrt approximation
		void normalize_fast(vec3_t v)
		{
			float ilength;

			ilength = q_rsqrt(DotProduct(v, v));

			v[0] *= ilength;
			v[1] *= ilength;
			v[2] *= ilength;
		}

		// (float) normalize input vector and returns "float length"
		vec_t normalize(vec3_t v)
		{
			float length, ilength;

			length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

			if (length) 
			{
				ilength = 1 / length;
				v[0] *= ilength;
				v[1] *= ilength;
				v[2] *= ilength;
			}

			return length;
		}

		// (float) normalize input vector, stores result in output if length != 0 and returns "float length"
		vec_t normalize2(const vec3_t v, vec3_t out)
		{
			float length, ilength;

			length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
			length = sqrt(length);

			if (length)
			{
				ilength = 1 / length;
				out[0] = v[0] * ilength;
				out[1] = v[1] * ilength;
				out[2] = v[2] * ilength;
			}
			else 
			{
				VectorClear(out);
			}

			return length;

		}

		void ma(const vec3_t veca, float scale, const vec3_t vecb, vec3_t out)
		{
			out[0] = veca[0] + scale * vecb[0];
			out[1] = veca[1] + scale * vecb[1];
			out[2] = veca[2] + scale * vecb[2];
		}

		vec_t dot(const vec3_t v1, const vec3_t v2)
		{
			return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
		}

		void subtract(const vec3_t veca, const vec3_t vecb, vec3_t out)
		{
			out[0] = veca[0] - vecb[0];
			out[1] = veca[1] - vecb[1];
			out[2] = veca[2] - vecb[2];
		}

		void add(const vec3_t veca, const vec3_t vecb, vec3_t out)
		{
			out[0] = veca[0] + vecb[0];
			out[1] = veca[1] + vecb[1];
			out[2] = veca[2] + vecb[2];
		}

		void copy(const vec3_t in, vec3_t out)
		{
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
		}

		void scale(const vec3_t in, float scale, vec3_t out)
		{
			out[0] = in[0] * scale;
			out[1] = in[1] * scale;
			out[2] = in[2] * scale;
		}

		void vec4_scale(const vec4_t in, vec_t scale, vec4_t out)
		{
			out[0] = in[0] * scale;
			out[1] = in[1] * scale;
			out[2] = in[2] * scale;
			out[3] = in[3] * scale;
		}

		void vec4_to_vec3(const vec4_t in, vec3_t out)
		{
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
		}
	}
}