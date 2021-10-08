#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

#define HK_JUMP Detours::X86Option::USE_JUMP
#define HK_CALL Detours::X86Option::USE_CALL

namespace utils
{
	class hook
	{
	public:

		static uintptr_t detour(uintptr_t offset, void* stub, Detours::X86Option option);

		
		class signature
		{
		public:
			struct container
			{
				const char* signature;
				const char* mask;
				std::function<void(char*)> callback;
			};

			signature(void* _start, size_t _length) : start(_start), length(_length) {}
			signature(DWORD _start, size_t _length) : signature(reinterpret_cast<void*>(_start), _length) {}
			signature() : signature(0x400000, 0x800000) {}

			void process();
			void add(container& container);

		private:
			void* start;
			size_t length;
			std::vector<container> signatures;
		};

		hook() : initialized(false), installed(false), place(nullptr), stub(nullptr), original(nullptr), useJump(false), protection(0) { ZeroMemory(this->buffer, sizeof(this->buffer)); }

		hook(void* place, void* stub, bool useJump = true) : hook() { this->initialize(place, stub, useJump); }
		hook(void* place, void(*stub)(), bool useJump = true) : hook(place, reinterpret_cast<void*>(stub), useJump) {}

		hook(DWORD place, void* stub, bool useJump = true) : hook(reinterpret_cast<void*>(place), stub, useJump) {}
		hook(DWORD place, DWORD stub, bool useJump = true) : hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}
		hook(DWORD place, void(*stub)(), bool useJump = true) : hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}

		~hook();

		hook* initialize(void* place, void* stub, bool useJump = true);
		hook* initialize(DWORD place, void* stub, bool useJump = true);
		hook* initialize(DWORD place, void(*stub)(), bool useJump = true); // For lambdas
		hook* install(bool unprotect = true, bool keepUnportected = false);
		hook* uninstall(bool unprotect = true);

		void* get_address();
		void quick();

		template <typename T> static std::function<T> call(DWORD function)
		{
			return std::function<T>(reinterpret_cast<T*>(function));
		}

		template <typename T> static std::function<T> call(FARPROC function)
		{
			return call<T>(reinterpret_cast<DWORD>(function));
		}

		template <typename T> static std::function<T> call(void* function)
		{
			return call<T>(reinterpret_cast<DWORD>(function));
		}

		static void set_string(void* place, const char* string, size_t length);
		static void set_string(DWORD place, const char* string, size_t length);

		static void set_string(void* place, const char* string);
		static void set_string(DWORD place, const char* string);

		static void nop(void* place, size_t length);
		static void nop(DWORD place, size_t length);

		static void redirect_jump(void* place, void* stub);
		static void redirect_jump(DWORD place, void* stub);

		template <typename T> static void set(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) = value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void set(DWORD place, T value)
		{
			return set<T>(reinterpret_cast<void*>(place), value);
		}

	private:
		bool initialized;
		bool installed;

		void* place;
		void* stub;
		void* original;
		char buffer[5];
		bool useJump;

		DWORD protection;

		std::mutex stateMutex;
	};
}
