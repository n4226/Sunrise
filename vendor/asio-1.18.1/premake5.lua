project "Asio"
-- NOT IFNISHED AND NOT WORKING not needed anyway it is essancially header only
	kind "SharedLib"
	language "C++"

	mainProjDir = "FlightSimTerrainSystemXplanePlugin"
	sunriseLocation = "%{wks.location}/Sunrise"

	-- REMEMBER: TODO: X-plane makes plugins rename their .dll file to a .xpl 
	-- which I don't think can't be done automatically through premake so i have to do it manually 
	-- with the following windows terminal commands:
	--  del E:\dev\devXplaneInstall\X-Plane_11\Resources\plugins\FlightSimTerrainSystemXplanePlugin\64\win.xpl - if 
	-- it already exists
	-- and Ren E:\dev\devXplaneInstall\X-Plane_11\Resources\plugins\FlightSimTerrainSystemXplanePlugin\64/win.dll E:\dev\devXplaneInstall\X-Plane_11\Resources\plugins\FlightSimTerrainSystemXplanePlugin\64\win.xpl
	-- to actually copy the dll to a new file with the correct file extension
	targetname ("win")

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}/64")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	defines {
        "DASIO_STANDALONE",
        "D_WIN32_WINNT=0x0501"
	}

	files {
		"src/**.h",
		"src/**.cpp",
	}

	includedirs {
		"C:/VulkanSDK/1.2.154.1/Include",

		"%{sunriseLocation}/src",
		"%{sunriseLocation}/vendor",
		"%{sunriseLocation}/vendor/spdlog/include",

		"%{wks.location}/Sunrise/vendor/glm-master",
		"%{wks.location}/Sunrise/vendor/marl-main/marl-main/include",
		"%{wks.location}/Sunrise/vendor/stb",
		"%{wks.location}/Sunrise/vendor/mango-master/mango-master/include",
		"%{wks.location}/Sunrise/vendor/libigl/include",
		"%{wks.location}/Sunrise/vendor/HTTPRequest/include",
		"%{wks.location}/Sunrise/vendor/httplib/include",
		"%{wks.location}/Sunrise/vendor/rapidjson/include",
		"%{wks.location}/Sunrise/vendor/libguarded/src",
		"%{wks.location}/Sunrise/vendor/nlohmann/include",

		"%{wks.location}/Sunrise/vendor/bin/glfw/windows/glfw-3.3.2.bin.WIN64/glfw-3.3.2.bin.WIN64/lib-vc2019",
		
		"%{wks.location}/Sunrise/vendor/date/include",
		"%{wks.location}/Sunrise/vendor/entt/single_include",

		"C:/dev/x-plane sdk/XPSDK303/SDK/CHeaders",
	}	

	libdirs {
		"C:/dev/x-plane sdk/XPSDK303/SDK/Libraries/Win"
	}

	links {
		"Sunrise",
		"XPLM_64",
		"XPWidgets_64",
		"OpenGL32",
	}

	-- xcopy E:/dev/devXplaneInstall/X-Plane_11/Resources/plugins/FlightSimTerrainSystemXplanePlugin/64/win.dll E:/dev/devXplaneInstall/X-Plane_11/Resources/plugins/FlightSimTerrainSystemXplanePlugin/64/win.xpl
	postbuildcommands {
		-- NEEDS aftermath for sunrise even though it does not need it for this plugin
		("{COPY} %{sunriseLocation}/vendor/bin//NVIDIA_Nsight_Aftermath_SDK_2021.1.0.21090/lib/x64/GFSDK_Aftermath_Lib.x64.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/64"),
		
		-- copy sunrise dll into build dur
		("{COPY} ../bin/" .. outputdir .. "/Sunrise/Sunrise.dLL ../bin/" .. outputdir .. "/%{prj.name}/64"),

		-- copy built plugin into xplane folder on e drive - E:/dev/devXplaneInstall/X-Plane_11/Resources/plugins/
		("{COPY} ../bin/" .. outputdir .. "/%{prj.name}/ E:/dev/devXplaneInstall/X-Plane_11/Resources/plugins/%{prj.name}/"),
		


		-- not working dont know why
		--("Ren E:/dev/devXplaneInstall/X-Plane_11/Resources/plugins/%{prj.name}/64/win.dll E:/dev/devXplaneInstall/X-Plane_11/Resources/plugins/%{prj.name}/64/win.xpl"),
	}


	filter "system:windows"
		
		cppdialect "C++17"
		staticruntime "on"
		systemversion "latest"

		defines {
			"SR_PLATFORM_WINDOWS",
			"IBM"
		}


	filter "configurations:Debug"
		defines "SR_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SR_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SR_DIST"
		runtime "Release"
		optimize "on"
