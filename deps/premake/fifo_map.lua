fifo_map = {
	source = path.join(dependencies.basePath, "fifo_map/src"),
}

function fifo_map.import()
	fifo_map.includes()
end

function fifo_map.includes()
	includedirs {
		fifo_map.source
	}
end

function fifo_map.project()
	project "fifo_map"
		language "C++"

		fifo_map.includes()

		files {
            path.join(fifo_map.source, "*.cpp"),
			path.join(fifo_map.source, "*.hpp"),
		}

		warnings "Off"
		kind "None"
end

table.insert(dependencies, fifo_map)
