@echo off

echo "building engine"

set VULKAN_SDK=C:\VulkanSDK\1.3.250.1
set include_paths= /I..\Engine\src /I%VULKAN_SDK%\include

mkdir ..\bin

pushd ..\bin
cl /EHsc /WX /Zi %include_paths% /DDEBUG ..\Engine\src\*.cpp /link /LIBPATH:%VULKAN_SDK%\Lib vulkan-1.lib
lib /out:engine.lib *.obj
popd