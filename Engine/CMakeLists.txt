find_package(Vulkan REQUIRED)

# Include directories
include_directories(
    ${VULKAN_SDK}/include
    ${CMAKE_SOURCE_DIR}/ThirdParty
    ${CMAKE_SOURCE_DIR}/ThirdParty/glfw/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/imgui
    ${CMAKE_SOURCE_DIR}/Engine/src
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdbs")

# Collect Engine and Third-Party Source Files
file(GLOB_RECURSE ENGINE_SOURCES ${CMAKE_SOURCE_DIR}/Engine/src/*.cpp)
file(GLOB_RECURSE VULKAN_BOOTSTRAP ${CMAKE_SOURCE_DIR}/ThirdParty/VulkanBootstrap/*.cpp)

# ImGui Source Files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/ThirdParty/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)

set(IMGUI_BACKENDS
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

# Add ImGui sources to engine sources
list(APPEND ENGINE_SOURCES ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

# Add the Engine Library (Now with ImGui + VulkanBootstrap)
add_library(engine STATIC ${ENGINE_SOURCES} ${VULKAN_BOOTSTRAP})

# Include directories for engine
target_include_directories(engine PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Link necessary libraries
target_link_libraries(engine PRIVATE
    Vulkan::Vulkan
    ${CMAKE_SOURCE_DIR}/ThirdParty/glfw/lib-vc2022/glfw3.lib
)

# Manually define Vulkan SDK path if it's not set
if(NOT DEFINED VULKAN_SDK)
    set(VULKAN_SDK "C:/VulkanSDK/1.3.250.1")
endif()

# 🔹 Shader Compilation Rules
file(GLOB FRAGMENT_SHADERS "${CMAKE_SOURCE_DIR}/Engine/shaders/*.frag")
file(GLOB VERTEX_SHADERS "${CMAKE_SOURCE_DIR}/Engine/shaders/*.vert")

set(SHADER_SPV_FILES "")

foreach(SHADER ${FRAGMENT_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}_frag.spv")

    file(TO_NATIVE_PATH "${SHADER}" SHADER_NATIVE)
    file(TO_NATIVE_PATH "${OUTPUT_FILE}" OUTPUT_NATIVE)

    add_custom_command(
        OUTPUT ${OUTPUT_NATIVE}
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shader: ${SHADER_NATIVE}"
        COMMAND "${VULKAN_SDK}/Bin/glslc.exe" "${SHADER_NATIVE}" -o "${OUTPUT_NATIVE}"
        DEPENDS ${SHADER_NATIVE}
        VERBATIM
    )
    list(APPEND SHADER_SPV_FILES ${OUTPUT_NATIVE})
endforeach()

foreach(SHADER ${VERTEX_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}_vert.spv")

    file(TO_NATIVE_PATH "${SHADER}" SHADER_NATIVE)
    file(TO_NATIVE_PATH "${OUTPUT_FILE}" OUTPUT_NATIVE)

    add_custom_command(
        OUTPUT ${OUTPUT_NATIVE}
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shader: ${SHADER_NATIVE}"
        COMMAND "${VULKAN_SDK}/Bin/glslc.exe" "${SHADER_NATIVE}" -o "${OUTPUT_NATIVE}"
        DEPENDS ${SHADER_NATIVE}
        VERBATIM
    )
    list(APPEND SHADER_SPV_FILES ${OUTPUT_NATIVE})
endforeach()

# 🔹 Create Shader Build Target
add_custom_target(compile_shaders DEPENDS ${SHADER_SPV_FILES})
add_dependencies(engine compile_shaders)