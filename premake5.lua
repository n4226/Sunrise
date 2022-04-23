
-- Must be built with /bigobj command line argument which now has to be done manually after every vs proj rebuild in the project settings`
project "Sunrise"

	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "srpch.h"
	pchsource "src/srpch.cpp"

	imguiFiles = {
		"vendor/imgui/*.cpp","vendor/imgui/backends/imgui_impl_vulkan.cpp", "vendor/imgui/backends/imgui_impl_vulkan.cpp"
	}

	files {
		"src/**.h",
		"src/**.cpp",
		"vendor/imgui/*.cpp",
		"vendor/imgui/misc/cpp/**.cpp",
		"vendor/imgui/backends/imgui_impl_vulkan.cpp",
		"vendor/imgui/backends/imgui_impl_glfw.cpp",

		"vendor/optick/src/*.cpp",
	}

	libdirs {
		--"C:/code/visual studio/FSTS/Sunrise/vendor/mango-master/mango-master/build/vs2019/x64/Debug",
		"C:/code/visual studio/GPUObjectsV6/Dependencies/mango-master/mango-master/build/Debug",
		"C:/VulkanSDK/1.3.211.0/Lib",
		
		"vendor/bin/NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/lib/x64",

		
	}
	

	
	links {
		--TODO add back mango
		--"mango",
		"vulkan-1",
		"marl",
		"geos",
		"geos_c"
	}

	includedirs {
		"src", "src/Sunrise",
		"vendor/spdlog/include",
		"C:/VulkanSDK/1.3.211.0/Include",
		"vendor/glm-master",
		"vendor/marl-main/marl-main/include",
		"vendor/stb",
		"vendor/mango-master/mango-master/include",
		"vendor/libigl/include",
		"vendor/HTTPRequest/include",
		"vendor/httplib/include",
		"vendor/rapidjson/include",
		"vendor/rapidjson/include",
		"vendor/libguarded/src",
		"vendor/nlohmann/include",
		"vendor/date/include",
		"vendor/entt/single_include",
		"vendor/imgui/",
		"vendor/asio-1.18.1/include",
		"vendor/CGAL-5.3/include",
		"vendor/boost_1_76_0",
		"vendor/eigen",
		"vendor/earcut-hpp/include/",
		"vendor/geos/capi/",
		"vendor/geos/include/",
		"vendor/DirectX/Inc",
	}	

	postbuildcommands {
		("echo on"),

		-- important: for FSTS project, mainProjDir has to be manuually switched between main program and mesh gen by chosing which set of the varible happens last
		-- no longer needed for static linking ("{COPYDIR} %{wks.location}/bin/" .. outputdir .. "/Sunrise/ %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),

		-- if this command causes build to fail, try re genrgating sultion through premake script since main proj might be set to other project i.e sunrise might have bee ncommited from world gen not FSTS
		("{COPYFILE} %{sunriseLocation}/vendor/bin//NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/lib/x64/GFSDK_Aftermath_Lib.x64.dll %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),
	}

	filter "system:windows"
		
		systemversion "latest"

		defines {
			"SR_PLATFORM_WINDOWS",
			"SR_BUILD_DLL"
		}

		libdirs {
			"vendor/bin/glfw/windows/glfw-3.3.2.bin.WIN64/glfw-3.3.2.bin.WIN64/lib-vc2019",
		}

		links {
			"GFSDK_Aftermath_Lib.x64",
			"DirectXTK"
		}
		includedirs {
			"vendor/bin/NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/include",
		}

	filter "system:macosx"

		defines {
			"SR_PLATFORM_MACOS",
			"SR_BUILD_DLL"
		}
		pchheader "src/srpch.h" 
		includedirs {
			"/usr/local/include",
			"vendor/bin/glfw/macos/glfw-3.3.6.bin.MACOS/include",
			--"/Users/michaelbaron/code/FSTS/Sunrise/vendor/bin/glfw/macos/glfw-3.3.6.bin.MACOS/include"
		}
		xcodebuildsettings { ["ALWAYS_SEARCH_USER_PATHS"] = "YES" }

		libdirs {
			"vendor/bin/glfw/macos/glfw-3.3.6.bin.MACOS/lib-universal",
		}

	filter "configurations:Debug"
		defines "SR_DEBUG"
		runtime "Debug"
		symbols "on"

		libdirs {
			"vendor/marl-main/marl-main/build/Debug",
			"vendor/geos/bin/Debug",
			"vendor/DirectX/Bin/Windows10_2022/x64/Debug"
		}

	filter "configurations:Release"
		defines "SR_RELEASE"
		--runtime "Debug"
		runtime "Release"
		optimize "on"	

		libdirs {
			"vendor/marl-main/marl-main/build/Release",
			"vendor/geos/bin/Release",
			"vendor/DirectX/Bin/Windows10_2022/x64/Release"
		}

	filter "configurations:Dist"
		defines "SR_DIST"
		runtime "Release"
		optimize "on"

		libdirs {
			"vendor/marl-main/marl-main/build/Release",
			"vendor/geos/bin/Release",
			"vendor/DirectX/Bin/Windows10_2022/x64/Release"
		}

	filter { 'files:src/Sunrise/Sunrise/graphics/vulkan/generalAbstractions/vma.cpp or files:vendor/imgui/**.cpp or files:vendor/optick/src/**.cpp' }
		flags {"NoPCH"}