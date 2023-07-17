@echo off

echo "building engine"

set VULKAN_SDK=C:\VulkanSDK\1.3.250.1
set include_paths= /I..\Engine\src /I%VULKAN_SDK%\include /I..ThirdParty\VulkanBootstrap

mkdir ..\bin

pushd ..\bin
cl /EHsc /WX /Zi %include_paths% /DDEBUG /c ..\Engine\src\*.cpp ..\Engine\src\core\*cpp ..\Engine\src\renderer\*cpp ..\ThirdParty\VulkanBootstrap\*cpp
lib /out:engine.lib *.obj
popd