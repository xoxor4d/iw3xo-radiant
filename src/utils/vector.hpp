#pragma once

namespace utils
{
	namespace vector // https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_shared.h
	{
		typedef float vec_t;
		typedef vec_t vec2_t[2];
		typedef vec_t vec3_t[3];
		typedef vec_t vec4_t[4];
		typedef vec_t vec5_t[5];

		#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
		#define VectorLength(x)			(sqrt((x)[0] * (x)[0] + (x)[1] * (x)[1] + (x)[2] * (x)[2]))
		#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
		#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
		#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
		#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
		#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
		#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
		#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
		#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
		#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

		// just in case you do't want to use the macros
		int		compare(const vec3_t v1, const vec3_t v2);
		void	clamp_vec3(const vec3_t a, const vec3_t b, vec3_t o1, vec3_t o2);
		vec_t	dot(const vec3_t v1, const vec3_t v2);
		vec_t	length(const vec3_t v);
		vec_t	length_squared(const vec3_t v);
		vec_t	distance(const vec3_t p1, const vec3_t p2);
		vec_t	distance_squared(const vec3_t p1, const vec3_t p2);
		vec_t	normalize(vec3_t v);		// returns vector length
		vec_t	normalize2(const vec3_t v, vec3_t out);
		void	normalize_fast(vec3_t v);
		void	zero(vec3_t v1);
		void	set_vec3(vec3_t v, float val);
		void	set_vec3(vec3_t v, float x, float y, float z);
		void	set_vec4(vec4_t v, float val);
		void	set_vec4(vec4_t v, float x, float y, float z, float w);
		void	subtract(const vec3_t veca, const vec3_t vecb, vec3_t out);
		void	add(const vec3_t veca, const vec3_t vecb, vec3_t out);
		void	copy(const vec3_t in, vec3_t out);
		void	scale(const vec3_t in, float scale, vec3_t out);
		void	ma(const vec3_t veca, float scale, const vec3_t vecb, vec3_t out);
		void	inverse(vec3_t v);
		void	cross_product(const vec3_t v1, const vec3_t v2, vec3_t cross);
		void	vec4_scale(const vec4_t in, vec_t scale, vec4_t out);
		void	vec4_to_vec3(const vec4_t in, vec3_t out);
	}
}