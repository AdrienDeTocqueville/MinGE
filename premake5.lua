workspace "MinGE"
	architecture "x64"
	startproject "Sandbox"
	characterset "ASCII"

	configurations
	{
		"debug",
		"dev",
		"release"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

LibsDir = "C:/Users/Adrien/Documents/Programs/C++/libs"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["SFML"] = LibsDir .. "/SFML-2.5.1/include"
IncludeDir["GLEW"] = LibsDir .. "/glew-2.1.0/include"
IncludeDir["glm"]  = LibsDir .. "/glm"

project "Engine"
	targetname "%{prj.name}_%{cfg.buildcfg}"
	location "Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	staticruntime "on"

	targetdir ("bin")
	objdir ("obj")

	files
	{
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}

	includedirs
	{
		"%{prj.name}",
		"%{IncludeDir.SFML}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.glm}"
	}

	-- Link libraries
	filter "configurations:debug or dev"
		links {
			"sfml-audio-d.lib",
			"sfml-graphics-d.lib",
			"sfml-window-d.lib",
			"sfml-network-d.lib",
			"sfml-system-d.lib"
		}

	filter "configurations:release"
		links {
			"sfml-audio.lib",
			"sfml-graphics.lib",
			"sfml-window.lib",
			"sfml-network.lib",
			"sfml-system.lib"
		}

	filter {} -- Reset filters

	links {
		"opengl32.lib",
		"glew32.lib"
	}

	libdirs {
		"%{LibsDir}/glew-2.1.0/lib/Release/x64",
		"%{LibsDir}/SFML-2.5.1/lib"
	}


	
	filter "system:windows"
			systemversion "latest"

	filter "configurations:debug"
		defines {
			"DEBUG",
			"DRAWAABB"
		}
		symbols "on"

	filter "configurations:dev"
		defines {
			"DEBUG",
			"PROFILE"
		}
		optimize "on"

	filter "configurations:release"
		defines "NDEBUG"
		optimize "on"

project "Tests"
	targetname "%{cfg.buildcfg}"
	location "test"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++14"
	staticruntime "on"

	targetdir ("bin")
	objdir ("obj")
	debugdir ("bin")

	files
	{
		"%{prj.location}/**.h",
		"%{prj.location}/**.cpp"
	}

	includedirs
	{
		"Engine",
		"%{IncludeDir.SFML}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.glm}"
	}

	links { "Engine" }

	filter "system:windows"
			systemversion "latest"

	filter "configurations:debug"
		defines {
			"DEBUG",
			"DRAWAABB"
		}
		symbols "on"

	filter "configurations:dev"
		defines {
			"DEBUG",
			"PROFILE"
		}
		optimize "on"

	filter "configurations:release"
		defines "NDEBUG"
		optimize "on"
