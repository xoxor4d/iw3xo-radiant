#include "std_include.hpp"

#define VA_BUFFER_COUNT		32
#define VA_BUFFER_SIZE		65536

namespace utils
{
	void mtx4x4_mul(game::GfxMatrix* mtx_out, game::GfxMatrix* a, game::GfxMatrix* b)
	{
		mtx_out->m[0][0] = a->m[0][0] * b->m[0][0] + a->m[0][1] * b->m[1][0] + b->m[2][0] * a->m[0][2] + b->m[3][0] * a->m[0][3];
		mtx_out->m[0][1] = b->m[1][1] * a->m[0][1] + a->m[0][0] * b->m[0][1] + b->m[2][1] * a->m[0][2] + a->m[0][3] * b->m[3][1];
		mtx_out->m[0][2] = b->m[1][2] * a->m[0][1] + b->m[0][2] * a->m[0][0] + a->m[0][2] * b->m[2][2] + a->m[0][3] * b->m[3][2];
		mtx_out->m[0][3] = b->m[1][3] * a->m[0][1] + a->m[0][0] * b->m[0][3] + b->m[2][3] * a->m[0][2] + a->m[0][3] * b->m[3][3];
		mtx_out->m[1][0] = a->m[1][1] * b->m[1][0] + a->m[1][0] * b->m[0][0] + b->m[2][0] * a->m[1][2] + b->m[3][0] * a->m[1][3];
		mtx_out->m[1][1] = a->m[1][0] * b->m[0][1] + b->m[1][1] * a->m[1][1] + b->m[2][1] * a->m[1][2] + a->m[1][3] * b->m[3][1];
		mtx_out->m[1][2] = a->m[1][1] * b->m[1][2] + a->m[1][0] * b->m[0][2] + a->m[1][2] * b->m[2][2] + b->m[3][2] * a->m[1][3];
		mtx_out->m[1][3] = a->m[1][0] * b->m[0][3] + b->m[1][3] * a->m[1][1] + b->m[2][3] * a->m[1][2] + a->m[1][3] * b->m[3][3];
		mtx_out->m[2][0] = a->m[2][1] * b->m[1][0] + a->m[2][0] * b->m[0][0] + b->m[2][0] * a->m[2][2] + b->m[3][0] * a->m[2][3];
		mtx_out->m[2][1] = a->m[2][0] * b->m[0][1] + b->m[1][1] * a->m[2][1] + b->m[2][1] * a->m[2][2] + a->m[2][3] * b->m[3][1];
		mtx_out->m[2][2] = a->m[2][1] * b->m[1][2] + a->m[2][0] * b->m[0][2] + a->m[2][2] * b->m[2][2] + b->m[3][2] * a->m[2][3];
		mtx_out->m[2][3] = a->m[2][0] * b->m[0][3] + b->m[1][3] * a->m[2][1] + b->m[2][3] * a->m[2][2] + a->m[2][3] * b->m[3][3];
		mtx_out->m[3][0] = b->m[0][0] * a->m[3][0] + a->m[3][1] * b->m[1][0] + a->m[3][2] * b->m[2][0] + b->m[3][0] * a->m[3][3];
		mtx_out->m[3][1] = b->m[1][1] * a->m[3][1] + a->m[3][0] * b->m[0][1] + a->m[3][2] * b->m[2][1] + a->m[3][3] * b->m[3][1];
		mtx_out->m[3][2] = b->m[0][2] * a->m[3][0] + b->m[1][2] * a->m[3][1] + a->m[3][2] * b->m[2][2] + b->m[3][2] * a->m[3][3];
		mtx_out->m[3][3] = b->m[1][3] * a->m[3][1] + a->m[3][0] * b->m[0][3] + a->m[3][2] * b->m[2][3] + a->m[3][3] * b->m[3][3];
	}

	void normalize_color3(float* src, float* dest)
	{
		float max = src[0];
		
		if (max < src[1]) {
			max = src[1];
		}
		
		if (max < src[2]) {
			max = src[2];
		}
		
		if (max == 0.0f)
		{
			utils::vector::set_vec3(dest, 1.0f);
		}
		else
		{
			const float length = 1.0f / max;
			utils::vector::scale(src, length, dest);
		}
	}
	
	// ----------------
	
	int try_stoi(const std::string str, bool quite)
	{
		int ret = 0;

		try
		{
			ret = std::stoi(str);
		}
		catch (const std::invalid_argument)
		{
			if (!quite)
			{
				printf(utils::va("[!] (%s) is not a valid argument! Defaulting to integer 0!\n", str.c_str()));
			}
		}

		return ret;
	}

	float try_stof(const std::string str, bool quite)
	{
		float ret = 0.0f;

		try
		{
			ret = std::stof(str);
		}
		catch (const std::invalid_argument)
		{
			if (!quite)
			{
				printf(utils::va("[!] (%s) is not a valid argument! Defaulting to float 0.0f!\n", str.c_str()));
			}
		}

		return ret;
	}

	int Q_stricmpn(const char *s1, const char *s2, int n)
	{
		int	c1, c2;

		if (s1 == NULL) 
		{
			if (s2 == NULL)
			{
				return 0;
			}
			else
			{
				return -1;
			}
				
		}
		else if (s2 == NULL)
		{
			return 1;
		}

		do 
		{
			c1 = *s1++;
			c2 = *s2++;

			if (!n--) 
			{
				return 0;		// strings are equal until end point
			}

			if (c1 != c2) 
			{
				if (c1 >= 'a' && c1 <= 'z') 
				{
					c1 -= ('a' - 'A');
				}

				if (c2 >= 'a' && c2 <= 'z') 
				{
					c2 -= ('a' - 'A');
				}

				if (c1 != c2) 
				{
					return c1 < c2 ? -1 : 1;
				}
			}
		} while (c1);

		return 0;		// strings are equal
	}

	int Q_stricmp(const char *s1, const char *s2)
	{
		return (s1 && s2) ? Q_stricmpn(s1, s2, 99999) : -1;
	}

	const char *va(const char *fmt, ...)
	{
		static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
		static int g_vaNextBufferIndex = 0;

		va_list ap;
		va_start(ap, fmt);
		char* dest = g_vaBuffer[g_vaNextBufferIndex];
		vsnprintf(g_vaBuffer[g_vaNextBufferIndex], VA_BUFFER_SIZE, fmt, ap);
		g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
		va_end(ap);
		return dest;
	}

	std::string str_to_lower(std::string input)
	{
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);
		return input;
	}

	std::vector<std::string> explode(const std::string& str, char delim)
	{
		std::vector<std::string> result;
		std::istringstream iss(str);

		for (std::string token; std::getline(iss, token, delim);)
		{
			std::string _entry = std::move(token);

			// Remove trailing 0x0 bytes
			while (_entry.size() && !_entry[_entry.size() - 1])
			{
				_entry = _entry.substr(0, _entry.size() - 1);
			}

			result.push_back(_entry);
		}

		return result;
	}

	void replace(std::string &string, std::string find, std::string replace)
	{
		size_t nPos = 0;

		while ((nPos = string.find(find, nPos)) != std::string::npos)
		{
			string = string.replace(nPos, find.length(), replace);
			nPos += replace.length();
		}
	}

	bool starts_with(std::string haystack, std::string needle)
	{
		return (haystack.size() >= needle.size() && !strncmp(needle.data(), haystack.data(), needle.size()));
	}

	bool ends_with(std::string haystack, std::string needle)
	{
		return (strstr(haystack.data(), needle.data()) == (haystack.data() + haystack.size() - needle.size()));
	}

	void erase_substring(std::string &base, std::string replace)
	{
		auto it = base.find(replace.c_str());
		if (it != std::string::npos)
		{
			base.erase(it, replace.size());
		}
	}

	int is_space(int c)
	{
		if (c < -1) return 0;
		return _isspace_l(c, nullptr);
	}

	// trim from start
	std::string &ltrim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int val)
		{
			return !is_space(val);
		}));
		return s;
	}

	// trim from end
	std::string &rtrim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int val)
		{
			return !is_space(val);
		}).base(), s.end());
		return s;
	}

	// trim from both ends
	std::string &trim(std::string &s)
	{
		return ltrim(rtrim(s));
	}
}
