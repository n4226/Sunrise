for %%i in (*.vert *.frag) do "C:\VulkanSDK\1.2.154.1\Bin\glslangValidator.exe" -V "%%~i" -o "%%~i.spv"

