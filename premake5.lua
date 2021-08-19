
-- Must be built with /bigobj command line argument which now has to be done manually after every vs proj rebuild in the project settings`
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
		"vendor/bin/NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/lib/x64",
	}

	links {
		--TODO add back mango
		--"mango",
		"vulkan-1",
		"marl",
		"GFSDK_Aftermath_Lib.x64"
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
		"vendor/bin/NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/include",
		"vendor/asio-1.18.1/include",
		"vendor/CGAL-5.3/include",
		"vendor/boost_1_76_0",
	}	

	postbuildcommands {
	
		-- temp for FSTS

		("echo on"),

		-- important: for FSTS project, mainProjDir has to be manuually switched between main program and mesh gen by chosing which set of the varible happens last
		("{COPYDIR} %{wks.location}/bin/" .. outputdir .. "/Sunrise/ %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),
		
		--{ generic commands but dont owrk currently
		--("{COPYFILE} %{wks.location}bin/" .. outputdir .. "/Sunrise/Sunrise.dLL %{wks.location}bin/" .. outputdir .. "/%{mainProjDir}/"),
		--("{COPY} %{wks.location}/bin/" .. outputdir .. "/Sunrise/Sunrise.pdb %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),

		("{COPYFILE} %{sunriseLocation}/vendor/bin//NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/lib/x64/GFSDK_Aftermath_Lib.x64.dll %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),
		--}



		--("xcopy /Q /E /Y /I %{wks.location}bin\\" .. outputdir .. "\\Sunrise\\Sunrise.dLL %{wks.location}bin\\" .. outputdir .. "\\%{mainProjDir}\\"),
		--("xcopy /Q /E /Y /I %{wks.location}bin\\" .. outputdir .. "\\Sunrise\\Sunrise.pdb %{wks.location}bin\\" .. outputdir .. "\\%{mainProjDir}\\"),
		--("{COPYFILE} %{sunriseLocation}/vendor/bin//NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/lib/x64/GFSDK_Aftermath_Lib.x64.dll %{wks.location}/bin/" .. outputdir .. "/%{mainProjDir}/"),
		--("xcopy /Q /E /Y /I %{wks.location}\\Sunrise\\vendor\\bin\\NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090\\lib\\x64\\GFSDK_Aftermath_Lib.x64.dll %{wks.location}bin\\" .. outputdir .. "\\%{mainProjDir}\\"),
		

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