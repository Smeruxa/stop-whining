cmake_minimum_required (VERSION 3.8)

project(StopWhining)

#ГЛАВНЫЕ АРГУМЕНТЫ
set(PATH_TO_GAME "D:/GAMES/SAMP")
set(OUT_NAME "SW")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PATH_TO_GAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(DIRECTX_LIB "${CMAKE_SOURCE_DIR}/3rdparty/DirectX/d3dx9.lib")

include(FetchContent)

FetchContent_Declare(
	sampapi
	GIT_REPOSITORY https://github.com/BlastHackNet/SAMP-API.git
	GIT_TAG 6d4db99ab41f19d1a6a7c6cd48f5878bd1e14b62#f89b35edb7eaf5db374e32a63f2366c4bb714418
)
FetchContent_Declare(
	ktsignal
	GIT_REPOSITORY https://github.com/kin4stat/ktsignal.git
	GIT_TAG f08a19d9a00ad5284871bf44178d835da7b26ebf
)
FetchContent_Declare(
	kthook
	GIT_REPOSITORY https://github.com/kin4stat/kthook.git
	GIT_TAG 21ca70481273d1efcc98ca49063178e46277009b
)
FetchContent_Declare(
	RakHook
	GIT_REPOSITORY https://github.com/imring/RakHook.git
)
FetchContent_MakeAvailable(sampapi ktsignal kthook rakhook)

execute_process(COMMAND git submodule update --init imgui WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/DirectX
    ${CMAKE_SOURCE_DIR}/3rdparty
    ${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/3rdparty/libdetours/Detours/src
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui ${PROJECT_NAME}_LIST)
list(APPEND ${PROJECT_NAME}_LIST 
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_win32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_dx9.cpp"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(PluginSDK REQUIRED gtasa)

add_subdirectory(3rdparty/libdetours)
set(DETOURS_LIB $<TARGET_FILE:libdetours> CACHE PATH "Detours library path")

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    sampapi 
    ktsignal 
    kthook 
    rakhook
    d3d9 
    PluginSDK::gtasa 
    ${DIRECTX_LIB}
	${DETOURS_LIB}
)

add_dependencies(${PROJECT_NAME} libdetours)

target_sources(${PROJECT_NAME} PRIVATE
	${${PROJECT_NAME}_LIST} 
	"src/main.cpp"
	"src/plugin/lpPlugin.cpp" 
	"src/imgui/imguisdraw.cpp" 
	"src/d3d9/d3dhook.cpp" 
	"src/d3d9/keyhook.cpp" 
	"src/config/config.cpp" 
	"src/rakhook/rakhookinstall.cpp" 
	"src/imgui/Widgets/Widgets.cpp" 
	"src/imgui/HotKey/ImHotKey.cpp" 
	"src/plugin/cheats/player/lpPlayer.cpp" 
	"src/plugin/cheats/utils/lpUtils.cpp" 
	"src/plugin/cheats/crosshair/lpCrosshair.cpp" 
	"src/plugin/cheats/crosshair/secure/Secure.cpp" 
	"src/plugin/cheats/crosshair/secure/memory/Memory.cpp" 
	"src/sampver/sampver.cpp" 
	"src/plugin/cheats/vehicle/lpVehicle.cpp"
	"src/plugin/cheats/additional/lpAdditional.cpp")

set_target_properties(${PROJECT_NAME}
	PROPERTIES
	OUTPUT_NAME ${OUT_NAME}
	SUFFIX ".asi"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)