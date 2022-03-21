for %%i in (*.vert *.frag *.comp) do "C:\VulkanSDK\1.2.154.1\Bin\glslangValidator.exe" -V "%%~i" -o "%%~i.spv" -g
echo off
Rem C:\code\visual_studio\SunriseWorldlMeshGen\extern\Sunrise\src\Sunrise\Sunrise\graphics\shaders\compileShaders.bat
Rem ../../../../../../
set /p outDir=<%~dp0\..\..\..\..\..\..\shaderHotSwapDir.txt
xcopy /s /y %cd% %outDir%