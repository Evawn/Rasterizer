cmake_minimum_required (VERSION 3.2)

project(cs5625solution LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBS "")
set(DEFS "-DGL_SILENCE_DEPRECATION")
set(INCS "")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# ----------------------------------------------------------------
# Include stb as a subproject (header only, very simple)

list(APPEND INCS ${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)


# ----------------------------------------------------------------
# Include nanogui as a subproject

# Disable building extras we won't need (pure C++ project)
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)

set(NANOGUI_BASE_DIR "ext/nanogui")

# Add the configurations from nanogui
add_subdirectory(${NANOGUI_BASE_DIR})

# For reliability of parallel build, make the NanoGUI targets dependencies
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")

list(APPEND DEFS ${NANOGUI_EXTRA_DEFS})
list(APPEND LIBS nanogui ${NANOGUI_EXTRA_LIBS})
list(APPEND INCS ${NANOGUI_EXTRA_INCS} ${NANOGUI_BASE_DIR}/include)


# ----------------------------------------------------------------
# Include cpplocate as a subproject

# Disable building extras we won't need
set(OPTION_BUILD_TESTS OFF CACHE BOOL " " FORCE)

set(CPPLOCATE_BASE_DIR "ext/cpplocate")

# Add the configurations from cpplocate
add_subdirectory(${CPPLOCATE_BASE_DIR} EXCLUDE_FROM_ALL)

# For reliability of parallel build, make the cpplocate target a dependency
set_property(TARGET cpplocate PROPERTY FOLDER "dependencies")

# Override library and runtime directories set by cpplocate's CMakeLists.txt
set_target_properties(cpplocate PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

list(APPEND INCS "${CPPLOCATE_BASE_DIR}/source/cpplocate/include")
list(APPEND LIBS cpplocate::cpplocate)


# ----------------------------------------------------------------
# Include assimp as a subproject

# Disable building extras we won't need
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_TESTS        OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT FALSE CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER TRUE CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_COLLADA_IMPORTER TRUE CACHE BOOL " " FORCE)

# Force CMake to build zlib so we can find it easily on Windows
if(WIN32)
  set(ASSIMP_BUILD_ZLIB TRUE CACHE BOOL " " FORCE)
endif(WIN32)

set(ASSIMP_BASE_DIR "ext/assimp")

# Add the configurations from assimp
add_subdirectory(${ASSIMP_BASE_DIR} EXCLUDE_FROM_ALL)

# For reliability of parallel build, make the assimp target a dependency
set_property(TARGET assimp PROPERTY FOLDER "dependencies")

# Use RPath on OS X and iOS (see CMP0042)
set_property(TARGET assimp PROPERTY MACOSX_RPATH TRUE)

list(APPEND INCS "${ASSIMP_BASE_DIR}/include")
list(APPEND LIBS assimp)



# ----------------------------------------------------------------
# GLWrap library

set(GLWRAP_BASE_DIR "GLWrap")
file(GLOB GLWRAP_SRC "${GLWRAP_BASE_DIR}/*[.cpp|.h|.hpp]")

add_library(GLWrap ${GLWRAP_SRC})
target_compile_definitions(GLWrap PUBLIC ${DEFS})
target_include_directories(GLWrap PUBLIC ${INCS})
target_link_libraries(GLWrap PUBLIC ${LIBS})
set_property(TARGET GLWrap PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET GLWrap APPEND PROPERTY COMPILE_DEFINITIONS "GLWRAP_BUILD")

list(APPEND LIBS GLWrap)
list(APPEND INCS ${CMAKE_CURRENT_SOURCE_DIR})


# ----------------------------------------------------------------
# RTUtil library

set(RTUTIL_BASE_DIR "RTUtil")
file(GLOB RTUTIL_SRC "${RTUTIL_BASE_DIR}/*[.cpp|.h|.hpp]")

add_library(RTUtil ${RTUTIL_SRC})
target_compile_definitions(RTUtil PUBLIC ${DEFS})
target_include_directories(RTUtil PUBLIC ${INCS})
target_link_libraries(RTUtil PUBLIC ${LIBS})
set_property(TARGET RTUtil PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET RTUtil APPEND PROPERTY COMPILE_DEFINITIONS "RTUTIL_BUILD")

list(APPEND LIBS RTUtil)
list(APPEND INCS ${CMAKE_CURRENT_SOURCE_DIR})


# ----------------------------------------------------------------
# Create executable targets, one per subdirectory.

# Creates an executable
#   This assumes that the executable's C++ source code is contained in a folder in the same directory as this file.
#   This adds all source files in the given folder to a new executable target with the same name as the directory.
function(createExecutable EXE_NAME)
  set(EXE_INCLUDE_DIR ${EXE_NAME})
  file(GLOB EXE_SRC "${EXE_INCLUDE_DIR}/*[.cpp|.h|.hpp]")
  set(COMMON_INCLUDE_DIR "Common")
  file(GLOB COMMON_SRC "${COMMON_INCLUDE_DIR}/*[.cpp|.h|.hpp]")
  set(EXE_INCS ${INCS})
  list(APPEND EXE_INCS ${EXE_INCLUDE_DIR})
  add_executable(${EXE_NAME} ${EXE_SRC} ${COMMON_SRC})
  target_compile_definitions(${EXE_NAME} PUBLIC ${DEFS})
  target_include_directories(${EXE_NAME} PUBLIC ${EXE_INCS})
  target_link_libraries(${EXE_NAME} ${LIBS})
endfunction(createExecutable)

# Create the targets for all desired executables
# Can comment out ones you are not currently concerned with to save compile time.
createExecutable("Demo")
createExecutable("Rast")
