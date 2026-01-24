cmake_minimum_required(VERSION 3.21)

project(VulkanRenderer
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(external/GLFW)
add_subdirectory(external/glm)

file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external/stb
)


target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        Vulkan::Vulkan
        glm::glm

)


if (WIN32)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            VK_USE_PLATFORM_WIN32_KHR
    )
endif()

find_program(GLSL_VALIDATOR glslangValidator
    HINTS
        $ENV{VULKAN_SDK}/Bin
        $ENV{VULKAN_SDK}/Bin32
)

if (GLSL_VALIDATOR)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/shaders/*.vert
        ${PROJECT_SOURCE_DIR}/shaders/*.frag
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV ${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv)

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
        )

        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach()

    add_custom_target(Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${PROJECT_NAME} Shaders)
endif()
