#pragma once

namespace utils
{
	class memory
	{
	public:
		class allocator
		{
		public:
			typedef void(*FreeCallback)(void*);

			allocator()
			{
				this->pool.clear();
				this->refMemory.clear();
			}
			~allocator()
			{
				this->clear();
			}

			void clear()
			{
				std::lock_guard<std::mutex> _(this->mutex);

				for (auto i = this->refMemory.begin(); i != this->refMemory.end(); ++i)
				{
					if (i->first && i->second)
					{
						i->second(i->first);
					}
				}

				this->refMemory.clear();

				for (auto data : this->pool)
				{
					memory::free(data);
				}

				this->pool.clear();
			}

			void free(void* data)
			{
				std::lock_guard<std::mutex> _(this->mutex);

				auto i = this->refMemory.find(data);
				if (i != this->refMemory.end())
				{
					i->second(i->first);
					this->refMemory.erase(i);
				}

				auto j = std::find(this->pool.begin(), this->pool.end(), data);
				if (j != this->pool.end())
				{
					memory::free(data);
					this->pool.erase(j);
				}
			}

			void free(const void* data)
			{
				this->free(const_cast<void*>(data));
			}

			void reference(void* memory, FreeCallback callback)
			{
				std::lock_guard<std::mutex> _(this->mutex);

				this->refMemory[memory] = callback;
			}

			void* allocate(size_t length)
			{
				std::lock_guard<std::mutex> _(this->mutex);

				void* data = memory::allocate(length);
				this->pool.push_back(data);
				return data;
			}
			template <typename T> inline T* allocate()
			{
				return this->allocateArray<T>(1);
			}
			template <typename T> inline T* allocateArray(size_t count = 1)
			{
				return static_cast<T*>(this->allocate(count * sizeof(T)));
			}

			bool empty()
			{
				return (this->pool.empty() && this->refMemory.empty());
			}

			char* duplicateString(std::string string)
			{
				std::lock_guard<std::mutex> _(this->mutex);

				char* data = memory::duplicate_string(string);
				this->pool.push_back(data);
				return data;
			}

		private:
			std::vector<void*> pool;
			std::map<void*, FreeCallback> refMemory;
			std::mutex mutex;
		};

		static void* allocate_align(size_t length, size_t alignment);
		static void* allocate(size_t length);
		template <typename T> static inline T* allocate()
		{
			return AllocateArray<T>(1);
		}
		template <typename T> static inline T* AllocateArray(size_t count = 1)
		{
			return static_cast<T*>(allocate(count * sizeof(T)));
		}

		static char* duplicate_string(std::string string);

		static void free(void* data);
		static void free(const void* data);

		static void free_align(void* data);
		static void free_align(const void* data);

		static bool is_set(void* mem, char chr, size_t length);
	};
}
