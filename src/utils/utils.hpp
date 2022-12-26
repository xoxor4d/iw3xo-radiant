#pragma once

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace utils
{
	class benchmark
	{
	public:
		benchmark() { start(); }
		benchmark(const char* operation)
		{
			m_operation_str = operation;
			start();
		}

		~benchmark()
		{
			now();
		}

		void now(const char* sub_operation_str = nullptr)
		{
#if defined(DEBUG) || defined(BENCHMARK)
			const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
			const auto end_time = std::chrono::high_resolution_clock::now();

			const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();
			const auto ms = static_cast<float>((end - start)) * 0.001f;

			const auto last_end = std::chrono::time_point_cast<std::chrono::microseconds>(m_last).time_since_epoch().count();
			const auto ms_diff = static_cast<float>(end - last_end) * 0.001f;

			const char* op_str = sub_operation_str ? sub_operation_str : m_operation_str ? m_operation_str : nullptr;
			if (op_str)
			{
				game::printf_to_console(">> [ %.3f ms ]\t[ ~ %.3f ms ]\tfor operation [ %s ]\n", ms, ms_diff, op_str);
			}
			else
			{
				game::printf_to_console(">> [ %.3f ms ]\t[ ~ %.3f ms ]\tbenchmark end\n", ms, ms_diff);
			}

			m_last = end_time;
#endif
		}

	private:

		void start()
		{
#if defined(DEBUG) || defined(BENCHMARK)
			m_start = std::chrono::high_resolution_clock::now();
			m_last = m_start;
#endif
		}

		const char* m_operation_str = nullptr;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_last;
	};

	void show_external_console(bool state);

	void mtx4x4_mul(game::GfxMatrix* mtx_out, game::GfxMatrix* a, game::GfxMatrix* b);
	void normalize_color3(float* src, float* dest);
	char pack_float(const float from);
	float unpack_float(const char from);

	// -----------
	
	static std::vector<std::string> split(const std::string& rawInput, const std::vector<char>& delims)
	{
		std::vector<std::string> strings;

		auto findFirstDelim = [](const std::string& input, const std::vector<char>& delims) -> std::pair<char, std::size_t>
		{
			auto firstDelim = 0;
			auto firstDelimIndex = static_cast<std::size_t>(-1);
			auto index = 0u;

			for (auto& delim : delims)
			{
				if ((index = input.find(delim)) != std::string::npos)
				{
					if (firstDelimIndex == -1 || index < firstDelimIndex)
					{
						firstDelim = delim;
						firstDelimIndex = index;
					}
				}
			}

			return { firstDelim, firstDelimIndex };
		};

		std::string input = rawInput;

		while (!input.empty())
		{
			auto splitDelim = findFirstDelim(input, delims);
			if (splitDelim.first != 0)
			{
				strings.push_back(input.substr(0, splitDelim.second));
				input = input.substr(splitDelim.second + 1);
			}
			else
			{
				break;
			}
		}

		strings.push_back(input);
		return strings;
	}

	static std::vector<std::string> split(std::string &str, char delimiter)
	{
		return split(str, std::vector < char >({ delimiter }));
	}

	static std::vector<std::string> split(const std::string& str, char delimiter)
	{
		return split(str, std::vector < char >({ delimiter }));
	}

	int		try_stoi(const std::string str, bool quite);
	float	try_stof(const std::string str, bool quite);

	int		Q_stricmpn(const char *s1, const char *s2, int n);
	int		Q_stricmp(const char *s1, const char *s2);

	const char *va(const char *fmt, ...);

	std::string str_to_lower(std::string input);
	bool starts_with(std::string& haystack, const std::string& needle, bool erase = false);
	bool starts_with(const std::string& haystack, const std::string& needle);
	bool ends_with(std::string haystack, std::string needle);
	std::vector<std::string> explode(const std::string& str, char delim);
	void replace(std::string &string, const std::string& find, const std::string& replace);
	bool erase_substring(std::string& base, const std::string& replace);


	bool string_contains(const std::string& s1, const std::string s2);
	bool string_contains(std::string& s1, const std::string s2, bool erase);

	static bool string_equals(const char* s1, const char* s2)
	{
		return !Q_stricmp(s1, s2);
	}
	
	std::string &ltrim(std::string &s);
	std::string &rtrim(std::string &s);
	std::string &trim(std::string &s);

	void tga_write(const char* filename, uint32_t width, uint32_t height, uint8_t* dataBGRA, uint8_t dataChannels = 4, uint8_t fileChannels = 3);
}
