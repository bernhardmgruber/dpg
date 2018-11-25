cmake_minimum_required(VERSION 3.0)

project(dpg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find packages
find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(imgui CONFIG REQUIRED)
set_target_properties(imgui::imgui PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

if (MSVC)
	find_package(glfw3 REQUIRED)
else()
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
endif()
include_directories(${GLFW3_INCLUDE_DIRS})

find_package(glm)
if (glm_FOUND)
	include_directories(${glm_INCLUDE_DIR})
else()
	find_package(GLM) # older versions
	include_directories(${GLM_INCLUDE_DIR})
endif()

include_directories(thirdparty)

# executable
file(GLOB_RECURSE source_files src/*.cpp src/*.h src/*.vert src/*.frag src/*.geom thirdparty/*.cpp thirdparty/*.h)
add_executable(${PROJECT_NAME} ${source_files})
source_group("" FILES ${source_files})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${source_files})

target_link_libraries(${PROJECT_NAME} PRIVATE
	GLEW::GLEW
	imgui::imgui
	${Boost_LIBRARIES}
)
if(MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		glfw
		opengl32
	)
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE
		${GLFW_LIBRARIES}
		GL
		stdc++fs
	)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
	-DGLFW_INCLUDE_NONE
	-DGLM_ENABLE_EXPERIMENTAL
	-DGLM_FORCE_RADIANS
	-DIMGUI_IMPL_OPENGL_LOADER_GLEW
)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# clang-format
find_program(CLANG_FORMAT NAMES "clang-format")
if(NOT CLANG_FORMAT)
	message("clang-format not found")
else()
	message("clang-format found: ${CLANG_FORMAT}")
	add_custom_target(clang-format COMMAND ${CLANG_FORMAT} -style=file -i ${source_files})
endif()

# clang-tidy
find_program(CLANG_TIDY NAMES "clang-tidy")
if(NOT CLANG_TIDY)
	message("clang-tidy not found")
else()
	message("clang-tidy found: ${CLANG_TIDY}")
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}")
	if(EXISTS .clang-tidy)
		configure_file(.clang-tidy .clang-tidy COPYONLY)
	endif()
endif()
