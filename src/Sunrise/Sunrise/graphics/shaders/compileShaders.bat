for %%i in (*.vert *.frag) do "C:\VulkanSDK\1.2.154.1\Bin\glslangValidator.exe" -V "%%~i" -o "%%~i.spv"
Rem C:\code\visual_studio\SunriseWorldlMeshGen\extern\Sunrise\src\Sunrise\Sunrise\graphics\shaders\compileShaders.bat
Rem ../../../../../../
set /p outDir=<%~dp0\..\..\..\..\..\..\shaderHotSwapDir.txt
echo %outDir%
xcopy /s /y %cd% %outDir%
pause