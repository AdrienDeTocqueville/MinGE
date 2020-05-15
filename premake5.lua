-- Modify this section
local prj_names = {"Editor", "graphics_tests", "unit_tests"}

filter "system:windows"
	lib_dir = "../Libs"
	sdl_path = lib_dir .. "/SDL2-2.0.12"
	glew_path = lib_dir .. "/glew-2.1.0"
	glm_path  = lib_dir .. "/glm-0.9.9.7"

function build_settings()
	filter "system:windows"
		systemversion "latest"
		defines "_CRT_SECURE_NO_DEPRECATE"

	filter "configurations:debug"
		defines { "DEBUG", "DRAWAABB" }
		symbols "on"
		optimize "off"

	filter "configurations:dev"
		defines { "DEBUG", "PROFILE" }
		symbols "on"
		optimize "size"

	filter "configurations:release"
		defines "NDEBUG"
		optimize "speed"
end
-- End of modifiable section


workspace "MinGE"
	architecture "x64"
	startproject(prj_names[1])
	flags "MultiProcessorCompile"

	configurations { "debug", "dev", "release" }


project "Engine"
	targetname "%{prj.name}_%{cfg.buildcfg}"
	kind "StaticLib"
	language "C++"
	cppdialect "C++11"
	staticruntime "on"

	targetdir ("bin")
	objdir ("obj")

	-- Sources
	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.inl",
		"%{prj.name}/**.cpp"
	}

	includedirs { "Engine" }

	filter "system:windows"
		includedirs {
			sdl_path .. "/include",
			glew_path .. "/include",
			glm_path,
		}

		libdirs {
			sdl_path .. "/lib/x64",
			glew_path .. "/lib/Release/x64",
		}

	-- Libraries
	filter "system:windows"
		links {
			"SDL2.lib",
			"SDL2main.lib",

			"glew32.lib",
			"opengl32.lib",
		}

	build_settings()


for i = 1, #prj_names do
	project (prj_names[i])
		targetname "%{prj.name}_%{cfg.buildcfg}"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++11"
		staticruntime "on"

		targetdir "bin"
		objdir "obj"
		debugdir "bin"

		-- Sources
		files {
			"%{prj.name}/**.h",
			"%{prj.name}/**.cpp"
		}

		includedirs { "Engine" }

		filter "system:windows"
			includedirs {
				sdl_path .. "/include",
				glew_path .. "/include",
				glm_path
			}

		filter {} -- Reset filters

		-- Libraries
		links { "Engine" }

		filter "system:linux"
			links {
				"GL", "GLEW", "pthread",
				"SDL2main", "SDL2"
			}

		build_settings()
end


-- Clean projects

function getfilename(prj, pattern)
	local fname = pattern:gsub("%%%%", prj.name)
	fname = path.join(prj.location, fname)
	return path.getrelative(os.getcwd(), fname)
end

function cleanfile(name)
	os.remove(name)
	print("Removed " .. name .. "...")
end

function cleandir(name)
	os.rmdir(name)
	print("Removed " .. name .. "...")
end

newaction {
	trigger = "clean",
	description = "Remove project files",

	onWorkspace = function(wks)
		cleanfile(wks.name .. ".sln")
	end,

	onProject = function(prj)
		if os.istarget("windows") then
			cleanfile(getfilename(prj, "%%.vcxproj"))
			cleanfile(getfilename(prj, "%%.vcxproj.filters"))
			cleanfile(getfilename(prj, "%%.vcxproj.user"))
		elseif os.istarget("linux") then
				print("TODO");
		end
	end,

	execute = function()
		cleandir("obj");
	end
}
