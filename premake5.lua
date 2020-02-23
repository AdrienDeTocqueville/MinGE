filter "system:windows"
	-- Update these variables if necessary
	lib_dir = "../Libs"
	ultralight_path = lib_dir .. "/ultralight-sdk-1.1.0-win-x64"
	sfml_path = lib_dir .. "/SFML-2.5.1-windows-vc15-64-bit"
	glew_path = lib_dir .. "/glew-2.1.0"
	glm_path  = lib_dir .. "/glm-0.9.9.7"



workspace "MinGE"
	architecture "x64"
	startproject "test"
	flags "MultiProcessorCompile"

	configurations { "debug", "dev", "release" }



project "Engine"
	targetname "%{prj.name}_%{cfg.buildcfg}"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
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
			ultralight_path .. "/include",
			sfml_path .. "/include",
			glew_path .. "/include",
			glm_path,
		}

		libdirs {
			ultralight_path .. "/lib",
			sfml_path .. "/lib",
			glew_path .. "/lib/Release/x64",
		}

	-- Libraries
	filter "system:windows"
		filter "configurations:debug or dev"
			links {
				"sfml-audio-d.lib",
				"sfml-graphics-d.lib",
				"sfml-window-d.lib",
				"sfml-network-d.lib",
				"sfml-system-d.lib",
			}

		filter "configurations:release"
			links {
				"sfml-audio.lib",
				"sfml-graphics.lib",
				"sfml-window.lib",
				"sfml-network.lib",
				"sfml-system.lib",
			}

	filter "system:windows"
		links {
			"Ultralight", "UltralightCore", "WebCore",
			"opengl32.lib",
			"glew32.lib",
		}

	-- Defines and flags
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
		optimize "debug"

	filter "configurations:release"
		defines "NDEBUG"
		optimize "speed"


project "test"
	targetname "%{cfg.buildcfg}"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++14"
	staticruntime "on"

	targetdir ("bin")
	objdir ("obj")
	debugdir ("bin")

	-- Sources
	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}

	includedirs { "Engine" }

	filter "system:windows"
		includedirs {
			sfml_path .. "/include",
			glew_path .. "/include",
			glm_path
		}

	filter {} -- Reset filters

	-- Libraries
	links { "Engine" }

	filter "system:linux"
		links {
			"GL", "GLEW", "pthread",
			"sfml-audio",
			"sfml-graphics",
			"sfml-window",
			"sfml-network",
			"sfml-system"
		}

	-- Defines and flags
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
		optimize "debug"

	filter "configurations:release"
		defines "NDEBUG"
		optimize "speed"

