#include "std_include.hpp"
#include "vector.hpp"
#include "cmath"

#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

namespace utils
{
	namespace vector // https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c
	{
		float rad_to_deg(const float radians)
		{
			return radians * (180.0f / M_PI);
		}

		float deg_to_rad(const float degrees)
		{
			return degrees * M_PI / 180.0f;
		}

		void to_euler_angles(const vec4_t* matrix, vec3_t out)
		{
			const float a = asinf(-matrix[0][2]);
			const float ca = cos(a);

			if (fabsf(ca) > 0.005f)
			{
				out[0] = atan2f(matrix[1][2] / ca, matrix[2][2] / ca);
				out[1] = a;
				out[2] = atan2f(matrix[0][1] / ca, matrix[0][0] / ca);
			}
			else
			{
				out[0] = atan2f(-matrix[2][1] / ca, matrix[1][1] / ca);
				out[1] = a;
				out[2] = 0.0f;
			}
		}

		void to_euler_angles_deg(const vec4_t* matrix, vec3_t out)
		{
			vec3_t euler_rad;
			to_euler_angles(matrix, euler_rad);

			out[0] = rad_to_deg(euler_rad[0]);
			out[1] = rad_to_deg(euler_rad[1]);
			out[2] = rad_to_deg(euler_rad[2]);
		}

		float cos_of_sum_of_arc_cos(float cos0, float cos1)
		{
			return (cos0 * cos1) - sqrt((1.0f - cos0 * cos0) * (1.0f - cos1 * cos1));
		}

		void rotate_point(float* point, const float *mat)
		{
			const game::vec3_t tvec = { point[0], point[1], point[2] };

			point[0] = mat[0] * tvec[0] + mat[1] * tvec[1] + mat[2] * tvec[2];
			point[1] = mat[3] * tvec[0] + mat[4] * tvec[1] + mat[5] * tvec[2];
			point[2] = mat[6] * tvec[0] + mat[7] * tvec[1] + mat[8] * tvec[2];
		}

		void rotate_point(const float* v, const float* q, float* out)
		{
			const float t2 =   q[3] * q[0];
			const float t3 =   q[3] * q[1];
			const float t4 =   q[3] * q[2];
			const float t5 =  -q[0] * q[0];
			const float t6 =   q[0] * q[1];
			const float t7 =   q[0] * q[2];
			const float t8 =  -q[1] * q[1];
			const float t9 =   q[1] * q[2];
			const float t10 = -q[2] * q[2];

			out[0] = ((t8 + t10) * v[0] + (t6 - t4)  * v[1] + (t3 + t7) * v[2]) * 2.0f + v[0];
			out[1] = ((t4 + t6)  * v[0] + (t5 + t10) * v[1] + (t9 - t2) * v[2]) * 2.0f + v[1];
			out[2] = ((t7 - t3)  * v[0] + (t2 + t9)  * v[1] + (t5 + t8) * v[2]) * 2.0f + v[2];
		}

		void vec3_rotate_transpose(const float* in, const float* matrix, float* out)
		{
			if (in == out)
			{
				AssertS("in != out");
			}

			out[0] = matrix[0] * in[0] + matrix[3] * in[1] + matrix[6] * in[2];
			out[1] = matrix[1] * in[0] + matrix[4] * in[1] + matrix[7] * in[2];
			out[2] = matrix[2] * in[0] + matrix[5] * in[1] + matrix[8] * in[2];
		}

		void angle_vectors(const game::vec3_t angles, game::vec3_t forward, game::vec3_t right, game::vec3_t up)
		{
			float angle;
			static float sr, sp, sy, cr, cp, cy;

			angle = angles[YAW] * (M_PI * 2 / 360);
			sy = sin(angle);
			cy = cos(angle);
			angle = angles[PITCH] * (M_PI * 2 / 360);
			sp = sin(angle);
			cp = cos(angle);
			angle = angles[ROLL] * (M_PI * 2 / 360);
			sr = sin(angle);
			cr = cos(angle);

			if (forward)
			{
				forward[0] = cp * cy;
				forward[1] = cp * sy;
				forward[2] = -sp;
			}

			if (right)
			{
				right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
				right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
				right[2] = -1 * sr * cp;
			}

			if (up)
			{
				up[0] = (cr * sp * cy + -sr * -sy);
				up[1] = (cr * sp * sy + -sr * cy);
				up[2] = cr * cp;
			}
		}

		void vectoangles(const vec3_t value1, vec3_t angles)
		{
			float forward;
			float yaw, pitch;

			if (value1[1] == 0 && value1[0] == 0) 
			{
				yaw = 0;
				if (value1[2] > 0) 
				{
					pitch = 90.0f;
				}
				else 
				{
					pitch = 270.0f;
				}
			}
			else 
			{
				if (value1[0]) 
				{
					yaw = (atan2(value1[1], value1[0]) * 180.0f / M_PI);
				}
				else if (value1[1] > 0.0f) 
				{
					yaw = 90.0f;
				}
				else 
				{
					yaw = 270.0f;
				}
				if (yaw < 0.0f) 
				{
					yaw += 360.0f;
				}

				forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
				pitch = (atan2(value1[2], forward) * 180.0f / M_PI);

				if (pitch < 0.0f)
				{
					pitch += 360.0f;
				}
			}

			angles[PITCH] = -pitch;
			angles[YAW] = yaw;
			angles[ROLL] = 0;
		}

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

		void copy(const float* in, float* out, const int size)
		{
			for (auto i = 0; i < size; i++)
			{
				out[i] = in[i];
			}
		}

		void multiply(const vec3_t veca, const vec3_t vecb, vec3_t out)
		{
			out[0] = veca[0] * vecb[0];
			out[1] = veca[1] * vecb[1];
			out[2] = veca[2] * vecb[2];
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

#pragma warning( push )
#pragma warning( disable : 6385 )
#pragma warning( disable : 6386 )
		void matrix_multiply(const float* in2, const float* in1, float* out)
		{
			out[0] = in1[0] * in2[0] + in1[1] * in2[3] + in1[2] * in2[6];
			out[1] = in2[4] * in1[1] + in2[1] * in1[0] + in1[2] * in2[7];
			out[2] = in2[5] * in1[1] + in2[2] * in1[0] + in1[2] * in2[8];
			out[3] = in1[4] * in2[3] + in1[3] * in2[0] + in2[6] * in1[5];
			out[4] = in1[4] * in2[4] + in1[3] * in2[1] + in1[5] * in2[7];
			out[5] = in1[4] * in2[5] + in1[3] * in2[2] + in1[5] * in2[8];
			out[6] = in1[7] * in2[3] + in1[6] * in2[0] + in2[6] * in1[8];
			out[7] = in1[7] * in2[4] + in1[6] * in2[1] + in1[8] * in2[7];
			out[8] = in1[7] * in2[5] + in1[6] * in2[2] + in1[8] * in2[8];
		}
#pragma warning( pop )
	}
}