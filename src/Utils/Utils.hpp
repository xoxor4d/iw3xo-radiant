#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace Utils
{
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

	int		try_stoi(const std::string str, bool quite);
	float	try_stof(const std::string str, bool quite);

	int		Q_stricmpn(const char *s1, const char *s2, int n);
	int		Q_stricmp(const char *s1, const char *s2);

	const char *VA(const char *fmt, ...);

	std::string StrToLower(std::string input);
	bool StartsWith(std::string haystack, std::string needle);
	bool EndsWith(std::string haystack, std::string needle);
	std::vector<std::string> Explode(const std::string& str, char delim);
	void Replace(std::string &string, std::string find, std::string replace);
	void EraseSubstring(std::string &base, std::string replace);

	std::string &LTrim(std::string &s);
	std::string &RTrim(std::string &s);
	std::string &Trim(std::string &s);
}
