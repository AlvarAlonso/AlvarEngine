@echo off
setlocal EnableDelayedExpansion

echo "building engine"

set VULKAN_SDK=C:\VulkanSDK\1.3.250.1
set include_paths= /I..\Engine\src /I%VULKAN_SDK%\include /I..\ThirdParty /I..\ThirdParty\glfw\include
set SRC_DIR=..\Engine\src

set SOURCES=
rem Find all .cpp files in the source directory and its subdirectories
for /r %SRC_DIR% %%f in (*.cpp) do (
    set SOURCES=!SOURCES! "%%f"
)

pushd .\shaders
%VULKAN_SDK%/Bin/glslc.exe shader.frag -o frag.spv
%VULKAN_SDK%/Bin/glslc.exe shader.vert -o vert.spv
%VULKAN_SDK%/Bin/glslc.exe deferred.frag -o deferred_frag.spv
%VULKAN_SDK%/Bin/glslc.exe deferred.vert -o deferred_vert.spv
%VULKAN_SDK%/Bin/glslc.exe light.frag -o light_frag.spv
%VULKAN_SDK%/Bin/glslc.exe light.vert -o light_vert.spv
popd

mkdir ..\bin

pushd ..\bin
cl /EHsc /WX /Zi /std:c++17 %include_paths% /DDEBUG /c %SOURCES% ..\ThirdParty\VulkanBootstrap\*cpp /link C:\VulkanSDK\1.3.250.1\Lib\vulkan-1.lib ..\ThirdParty\glfw\lib-vc2022\glfw3.lib
lib /out:engine.lib *.obj
popd