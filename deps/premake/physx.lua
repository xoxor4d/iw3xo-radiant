physx  = {
	source = path.join(dependencies.basePath, "physx"),
}

function physx.import()
	--[[ filter "configurations:Debug" ]]
	libdirs { path.join(physx.source, "bin") }
	--[[ filter "configurations:Release"
		libdirs { path.join(physx.source, "bin/release") } ]]

	filter "configurations:Debug"
		links { "/debug/PhysX_32" }
		links { "/debug/PhysXCooking_32" }
		links { "/debug/PhysXCommon_32" }
		links { "/debug/PhysXExtensions_static_32" }
		links { "/debug/PhysXFoundation_32" }
		links { "/debug/PhysXPvdSDK_static_32" }
		filter {}

	filter "configurations:Release"
		links { "/release/PhysX_32" }
		links { "/release/PhysXCooking_32" }
		links { "/release/PhysXCommon_32" }
		links { "/release/PhysXExtensions_static_32" }
		links { "/release/PhysXFoundation_32" }
		links { "/release/PhysXPvdSDK_static_32" }
		filter {}
	
	physx.includes()
end

function physx.includes()
	includedirs {
		path.join(physx.source, "include"),
		path.join(physx.source, "include/foundation"),
		path.join(physx.source, "include/common"),
		path.join(physx.source, "include/extensions"),
	}
end

function physx.project()
	project "physx"
		language "C++"

		physx.includes()

		files {
			path.join(physx.source, "include/**.h"),
		}

		warnings "Off"
		kind "None"
end

table.insert(dependencies, physx)
