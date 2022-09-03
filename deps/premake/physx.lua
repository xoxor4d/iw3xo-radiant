physx  = {
	source = path.join(dependencies.basePath, "physx"),
}

function physx.import()
	--[[ filter "configurations:Debug" ]]
	libdirs { path.join(physx.source, "lib") }
	--[[ filter "configurations:Release"
		libdirs { path.join(physx.source, "bin/release") } ]]

	filter "configurations:Debug"
		links { "/debug/PhysX_32" }
		links { "/debug/PhysXCharacterKinematic_static_32" }
		links { "/debug/PhysXCommon_32" }
		links { "/debug/PhysXCooking_32" }
		links { "/debug/PhysXExtensions_static_32" }
		links { "/debug/PhysXFoundation_32" }
		links { "/debug/PhysXPvdSDK_static_32" }
		filter {}

	filter "configurations:Release"
		links { "/profile/PhysX_32" }
		links { "/profile/PhysXCharacterKinematic_static_32" }
		links { "/profile/PhysXCommon_32" }
		links { "/profile/PhysXCooking_32" }
		links { "/profile/PhysXExtensions_static_32" }
		links { "/profile/PhysXFoundation_32" }
		links { "/profile/PhysXPvdSDK_static_32" }
		filter {}
	
	physx.includes()
end

function physx.includes()
	includedirs {
		path.join(physx.source, "include"),
		path.join(physx.source, "include/characterkinematic"),
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
