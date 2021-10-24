ImGuizmo = {
	source = path.join(dependencies.basePath, "ImGuizmo"),
}

function ImGuizmo.import()
    links { "ImGuizmo" }
	ImGuizmo.includes()
end

function ImGuizmo.includes()
	includedirs {
		ImGuizmo.source,
        path.join(dependencies.basePath, "imgui")
	}
end

function ImGuizmo.project()
	project "ImGuizmo"
		language "C++"

		ImGuizmo.includes()

		files {
            path.join(ImGuizmo.source, "ImGuizmo.cpp"),
			path.join(ImGuizmo.source, "ImGuizmo.h"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, ImGuizmo)
