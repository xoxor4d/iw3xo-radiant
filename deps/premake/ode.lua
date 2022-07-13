ode_version = "0.16"

ode = {
	source = path.join(dependencies.basePath, "ode"),
}

function ode.import()
	links { "ode" }
	ode.includes()
end

function ode.includes()
	includedirs {
		path.join(ode.source, "include"),
		path.join(ode.source, "ode/src"),
    	path.join(ode.source, "ode/src/joints"),
    	path.join(ode.source, "OPCODE"),
    	path.join(ode.source, "GIMPACT/include"),
    	path.join(ode.source, "libccd/src/custom"),
    	path.join(ode.source, "libccd/src"),

		path.join(ode.source, "ou/include")
	}

	excludes {
        path.join(ode.source, "/ode/src/collision_trimesh_colliders.h"),
        path.join(ode.source, "/ode/src/gimpact_contact_export_helper.cpp"),
        path.join(ode.source, "/ode/src/gimpact_contact_export_helper.h"),
        path.join(ode.source, "/ode/src/gimpact_gim_contact_accessor.h"),
        path.join(ode.source, "/ode/src/gimpact_plane_contact_accessor.h"),
        path.join(ode.source, "/ode/src/collision_trimesh_internal.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_opcode.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_gimpact.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_box.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_ccylinder.cpp"),
        path.join(ode.source, "/ode/src/collision_cylinder_trimesh.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_ray.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_sphere.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_trimesh.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_trimesh_old.cpp"),
        path.join(ode.source, "/ode/src/collision_trimesh_plane.cpp"),
        path.join(ode.source, "/ode/src/collision_convex_trimesh.cpp")
      }
end


function ode.project()
	project "ode"
		language "C++"

		ode.includes()

    	files {
    		path.join(ode.source, "include/ode/*.h"),
    		path.join(ode.source, "ode/src/joints/*.h"),
    		path.join(ode.source, "ode/src/joints/*.cpp"),
    		path.join(ode.source, "ode/src/*.h"),
    		path.join(ode.source, "ode/src/*.c"),
    		path.join(ode.source, "ode/src/*.cpp"),

			path.join(ode.source, "ou/include/**.h"),
			path.join(ode.source, "ou/src/**.h"),
			path.join(ode.source, "ou/src/**.cpp")
    	}

    	--[[ includedirs { "../ou/include" }
    	files   { "../ou/include/**.h", "../ou/src/**.h", "../ou/src/**.cpp" }

    	defines { "_OU_NAMESPACE=odeou" }
		defines { "_OU_FEATURE_SET=_OU_FEATURE_SET_ATOMICS" } ]]

		warnings "Off"
		kind "StaticLib"
		defines "ODE_LIB"
		defines { "dIDESINGLE", "CCD_IDESINGLE" }
end

table.insert(dependencies, ode)


if _ACTION and _ACTION ~= "clean" then

	local p = os.getcwd()
	p = path.getdirectory(p)

	--[[ local asd = path.join(p, "ode/build/config-default.h")
	print("writing to " .. asd) ]]

    local infile = io.open(path.join(p, "ode/build/config-default.h"), "r")
    local text = infile:read("*a")

    --[[ if _OPTIONS["no-trimesh"] then ]]
      text = string.gsub(text, "#define dTRIMESH_ENABLED 1", "/* #define dTRIMESH_ENABLED 1 */")
      text = string.gsub(text, "#define dTRIMESH_OPCODE 1", "/* #define dTRIMESH_OPCODE 1 */")
    --[[ elseif (_OPTIONS["with-gimpact"]) then
      text = string.gsub(text, "#define dTRIMESH_OPCODE 1", "#define dTRIMESH_GIMPACT 1")
    end ]]

    text = string.gsub(text, "/%* #define dOU_ENABLED 1 %*/", "#define dOU_ENABLED 1")
    if _OPTIONS["with-ou"] or not _OPTIONS["no-threading-intf"] then
      text = string.gsub(text, "/%* #define dATOMICS_ENABLED 1 %*/", "#define dATOMICS_ENABLED 1")
    end

    if _OPTIONS["with-ou"] then
      text = string.gsub(text, "/%* #define dTLS_ENABLED 1 %*/", "#define dTLS_ENABLED 1")
    end

    --[[ if _OPTIONS["no-threading-intf"] then ]]
      text = string.gsub(text, "/%* #define dTHREADING_INTF_DISABLED 1 %*/", "#define dTHREADING_INTF_DISABLED 1")
    --[[ elseif not _OPTIONS["no-builtin-threading-impl"] then
      text = string.gsub(text, "/%* #define dBUILTIN_THREADING_IMPL_ENABLED 1 %*/", "#define dBUILTIN_THREADING_IMPL_ENABLED 1")
    end ]]

    if _OPTIONS["16bit-indices"] then
      text = string.gsub(text, "#define dTRIMESH_16BIT_INDICES 0", "#define dTRIMESH_16BIT_INDICES 1")
    end
  
    if _OPTIONS["old-trimesh"] then
      text = string.gsub(text, "#define dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER 0", "#define dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER 1")
    end

	local asd = path.join(p, "ode/build/config-default.h")
	print("writing to " .. asd)

    local outfile = io.open(path.join(p, "ode/ode/src/config.h"), "w")
    outfile:write(text)

	-- MessageBoxW to A in ode/src/error.cpp
	outfile:write("\n")
	outfile:write("#undef UNICODE")
    outfile:close()
  end

----------------------------
-- Write precision headers
----------------------------
if _ACTION and _ACTION ~= "clean" then
    function generateheader(headerfile, placeholder, precstr)
      local outfile = io.open(headerfile, "w")
      for i in io.lines(headerfile .. ".in")
      do
        local j,_ = string.gsub(i, placeholder, precstr)
        --[[ print("writing " .. j .. " into " .. headerfile) ]]
        outfile:write(j .. "\n")
      end
      outfile:close()
    end

	local p = os.getcwd()
	p = path.getdirectory(p)
    
    function generate(precstr)
      generateheader(path.join(p, "ode/include/ode/precision.h"), "@ODE_PRECISION@", "d" .. precstr)
      generateheader(path.join(p, "ode/libccd/src/ccd/precision.h"), "@CCD_PRECISION@", "CCD_" .. precstr)
    end
    
	generate("SINGLE")
	--[[ generate("UNDEFINEDPRECISION") ]]

	generateheader(path.join(p, "ode/include/ode/version.h"), "@ODE_VERSION@", ode_version)

end