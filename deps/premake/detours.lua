detours = {
	source = path.join(dependencies.basePath, "detours"),
}

function detours.import()
	links { "detours" }
	detours.includes()
end

function detours.includes()
	includedirs {
		detours.source,
		path.join(detours.source, "detours/zydis/msvc"),
		path.join(detours.source, "detours/zydis/src"),
		path.join(detours.source, "detours/zydis/dependencies/zycore/include"),
		path.join(detours.source, "detours/zydis/include")
		--path.join(detours.source, "detours")
	}
	
	defines { "ZYDIS_STATIC_DEFINE", "_LIB" }
end

function detours.project()
	project "detours"
		language "C++"

		detours.includes()

		files {
			path.join(detours.source, "detours/*.cpp"),
			path.join(detours.source, "detours/*.hpp"),
			path.join(detours.source, "detours/*.c"),
			path.join(detours.source, "detours/*.h"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, detours)
