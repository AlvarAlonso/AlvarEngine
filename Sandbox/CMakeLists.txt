project(Sandbox)

find_package(Vulkan REQUIRED)

# Include directories for Vulkan and Engine
include_directories(
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ThirdParty
)

add_executable(sandbox ${CMAKE_SOURCE_DIR}/Sandbox/main.cpp)

target_link_libraries(sandbox
    engine
    user32.lib
    Gdi32.lib
    shell32.lib
    ${Vulkan_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/ThirdParty/glfw/lib-vc2022/glfw3_mt.lib
)