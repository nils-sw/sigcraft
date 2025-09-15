cmake_minimum_required(VERSION 3.28)
project(sigcraft)

set(CMAKE_CXX_STANDARD 20)

set(IMR_FOLDER "C:/Users/darkg/HAG/imr_mesh" CACHE STRING "Folder where IMR is found")
add_subdirectory(${IMR_FOLDER} imr_build)

find_package(nasl)

if (NOT TARGET imr)
    message("IMR not found. Please set IMR_FOLDER accordingly.")
endif ()

add_subdirectory(enklume)

find_package(nasl)

add_executable(sigcraft main.cpp camera.cpp chunk_mesh.cpp world.cpp)
target_link_libraries(sigcraft imr enklume nasl::nasl)

add_custom_target(mesh_shading_mesh_spv COMMAND ${GLSLANG_EXE} -V --target-env vulkan1.3 -S mesh ${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh_shader.mesh -o ${CMAKE_CURRENT_BINARY_DIR}/mesh_shader.mesh.spv)
add_dependencies(sigcraft mesh_shading_mesh_spv)
add_custom_target(mesh_shading_task_spv COMMAND ${GLSLANG_EXE} -V --target-env vulkan1.3 -S task ${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh_shader.task -o ${CMAKE_CURRENT_BINARY_DIR}/mesh_shader.task.spv)
add_dependencies(sigcraft mesh_shading_task_spv)
add_custom_target(basic_frag_spv COMMAND ${GLSLANG_EXE} -V --target-env vulkan1.3 -S frag ${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.frag -o ${CMAKE_CURRENT_BINARY_DIR}/basic.frag.spv)
add_dependencies(sigcraft basic_frag_spv) 
