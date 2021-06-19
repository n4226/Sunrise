

project "Sunrise"

	kind "SharedLib"
	language "C++"

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
		"vendor/imgui/backends/imgui_impl_vulkan.cpp",
		"vendor/imgui/backends/imgui_impl_glfw.cpp"
	}

	libdirs {
		"vendor/marl-main/marl-main/Debug",
		--"C:/code/visual studio/FSTS/Sunrise/vendor/mango-master/mango-master/build/vs2019/x64/Debug",
		"C:/code/visual studio/GPUObjectsV6/Dependencies/mango-master/mango-master/build/Debug",
		"C:/VulkanSDK/1.2.154.1/Lib",
		"vendor/bin/glfw/windows/glfw-3.3.2.bin.WIN64/glfw-3.3.2.bin.WIN64/lib-vc2019",

	}

	links {
		--TODO add back mango
		--"mango",
		"vulkan-1",
		"marl",
	}

	includedirs {
		"src",
		"vendor/spdlog/include",
		"C:/VulkanSDK/1.2.154.1/Include",
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
	}	

	postbuildcommands {
	
		-- temp for FSTS

		("{COPY} %{wks.location}/bin/" .. outputdir .. "/Sunrise/Sunrise.dLL %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),
		("{COPY} %{wks.location}/bin/" .. outputdir .. "/Sunrise/Sunrise.pdb %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),

		-- shaders again temp
		-- C:/code/visual studio/FSTS/Sunrise/src/Sunrise/Sunrise/graphics/shaders
		--("call C:/code/visual studio/FSTS/Sunrise/src/Sunrise/Sunrise/graphcis/shaders/compileShaders.bat"),
		--("XCOPY /S /Y src/Sunrise/Sunrise/graphcis/shaders/* ../bin/" .. outputdir .. "/FlightSimTerrainSystem/shaders"),
	}

	filter "system:windows"
		
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines {
			"SR_PLATFORM_WINDOWS",
			"SR_BUILD_DLL"
		}


	filter "configurations:Debug"
		defines "SR_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SR_RELEASE"
		--runtime "Debug"
		runtime "Release"
		optimize "on"	

	filter "configurations:Dist"
		defines "SR_DIST"
		runtime "Release"
		optimize "on"

	filter { 'files:src/Sunrise/Sunrise/graphics/vulkan/generalAbstractions/vma.cpp or files:vendor/imgui/**.cpp' }
		flags {"NoPCH"}