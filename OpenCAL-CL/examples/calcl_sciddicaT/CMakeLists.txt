# opencal/OpenCAL-CL/examples/sciddicaT-calcl/CMakeLists.txt

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package( OpenCL REQUIRED )
include_directories( ${OpenCL_INCLUDE_DIRS} )
link_directories(${OpenCL_LIBRARIES})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/OpenCAL/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/OpenCAL-CL/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/OpenCAL-CL/OpenCL-Utils/lib)


file(GLOB SRC
    "include/*.h"
    "source/*.cpp"
    "source/*.c"
)

add_executable(calcl_sciddicaT ${SRC})

if(WIN32)
  target_link_libraries(calcl_sciddicaT opencal-cl opencal opencl_utils ${OpenCL_LIBRARY})
else()
  target_link_libraries(calcl_sciddicaT m opencal-cl opencal opencl_utils ${OpenCL_LIBRARY})
endif()




