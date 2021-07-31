#include "std_include.hpp"

namespace utils
{
	void* memory::allocate_align(size_t length, size_t alignment)
	{
		void* data = _aligned_malloc(length, alignment);
		assert(data != nullptr);
		return data;
	}

	void* memory::allocate(size_t length)
	{
		void* data = calloc(length, 1);
		assert(data != nullptr);
		return data;
	}

	char* memory::duplicate_string(std::string string)
	{
		char* newString = memory::AllocateArray<char>(string.size() + 1);
		std::memcpy(newString, string.data(), string.size());
		return newString;
	}

	void memory::free(void* data)
	{
		if (data)
		{
			free(data);
		}
	}

	void memory::free(const void* data)
	{
		memory::free(const_cast<void*>(data));
	}

	void memory::free_align(void* data)
	{
		if (data)
		{
			_aligned_free(data);
		}
	}

	void memory::free_align(const void* data)
	{
		memory::free_align(const_cast<void*>(data));
	}

	// Complementary function for memset, which checks if memory is filled with a char
	bool memory::is_set(void* mem, char chr, size_t length)
	{
		char* memArr = reinterpret_cast<char*>(mem);

		for (size_t i = 0; i < length; ++i)
		{
			if (memArr[i] != chr)
			{
				return false;
			}
		}

		return true;
	}
}
