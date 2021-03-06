newaction {
	trigger = "generate-buildinfo",
	description = "Sets up build information file like version.h.",
	onWorkspace = function(wks)

		local oldRevNumber = "(none)"
		print ("Reading :: " .. path.getdirectory(wks.location) .. "/src/version.hpp")
		local oldVersionHeader = io.open(path.getdirectory(wks.location) .. "/src/version.hpp", "r")
		if oldVersionHeader ~=nil then
			local oldVersionHeaderContent = assert(oldVersionHeader:read('*a'))
			oldRevNumber = string.match(oldVersionHeaderContent, "#define REVISION (%d+)")
			if oldRevNumber == nil then
				oldRevNumber = 0
			end
		end

		-- generate version.hpp with a revision number if not equal
		local revNumber = oldRevNumber + 1
			print ("Update " .. oldRevNumber .. " -> " .. revNumber)
			local versionHeader = assert(io.open(path.getdirectory(wks.location) .. "/src/version.hpp", "w"))
			versionHeader:write("/* Automatically generated by premake5. */\n")
			versionHeader:write("\n")
			versionHeader:write("#define REVISION " .. revNumber .. "\n")
			versionHeader:close()
	end
}

workspace "iw3xo-radiant"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	buildlog "%{wks.location}/obj/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}/%{prj.name}.log"
	configurations { "Debug", "Release" }
	architecture "x32"
	platforms "x86"
	systemversion "10.0.17763.0"
	startproject "iw3r"

	disablewarnings {
		"4239",
		"4505",
		"4996",
	}

	buildoptions {
		"/std:c++latest"
	}
	systemversion "10.0.17763.0"
	defines { "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" }

	configuration "windows"
		defines { "_WINDOWS", "WIN32" }
		staticruntime "On"
		
		if symbols ~= nil then
			symbols "On"
		else
			flags { "Symbols" }
		end

	configuration "Release"
		defines { "NDEBUG" }
		flags { "MultiProcessorCompile", "LinkTimeOptimization", "No64BitChecks" }
		optimize "Full"

	configuration "Debug"
		defines { "DEBUG", "_DEBUG" }
		flags { "MultiProcessorCompile", "No64BitChecks" }
		optimize "Debug"

	project "iw3r"
		kind "SharedLib"
		language "C++"
		files {
			"./src/**.rc",
			"./src/**.hpp",
			"./src/**.cpp",
		}
		includedirs {
			"%{prj.location}/src",
			"./src",
		}
		resincludedirs {
			"$(ProjectDir)src" -- fix for VS IDE
		}

		-- Pre-compiled header
		pchheader "STDInclude.hpp" -- must be exactly same as used in #include directives
		pchsource "src/STDInclude.cpp" -- real path
		buildoptions { "/Zm100 -Zm100" }

		-- Virtual paths
		if not _OPTIONS["no-new-structure"] then
			vpaths {
				["Headers/*"] = { "./src/**.hpp" },
				["Sources/*"] = { "./src/**.cpp" },
				["Resource/*"] = { "./src/**.rc" },
			}
		end

		vpaths {
			["Docs/*"] = { "**.txt","**.md" },
		}
		
		-- Pre-build
		prebuildcommands {
			"cd %{_MAIN_SCRIPT_DIR}",
			"tools\\premake5 generate-buildinfo"
		}

		-- Specific configurations
		flags { "UndefinedIdentifiers" }
		warnings "Extra"

		configuration "Release"
		--	flags { "FatalCompileWarnings" }
		configuration {}