@echo off

echo "building sandbox"

set VULKAN_SDK=C:\VulkanSDK\1.3.250.1
set include_paths= /I..\Engine\src /I%VULKAN_SDK%\include /I..\ThirdParty
set file_paths= ..\Sandbox\main.cpp

pushd ..\bin
cl /EHsc /WX /Zi %include_paths% /DDEBUG %file_paths% /link engine.lib user32.lib Gdi32.lib C:\VulkanSDK\1.3.250.1\Lib\vulkan-1.lib
popd