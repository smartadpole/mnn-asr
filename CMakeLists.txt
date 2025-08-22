cmake_minimum_required(VERSION 3.5)
project(mnn-asr)
set(MODULE_NAME ASL)
set(PROJECT_NAME asr_sample)
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} sample/asr_sample.cpp)

set(ALG_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory(src ${ALG_DIR})
add_dependencies(${PROJECT_NAME} ${MODULE_NAME})

# CNN lib
include(${ROOT_DIR}/cmake/find_device.cmake)
target_link_libraries(${PROJECT_NAME}  ${CNN_LIBRARY})

include_directories(${ROOT_DIR}/src)
message(STATUS "build on processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
set(MODULE_LIB ${ALG_DIR}/lib${MODULE_NAME}.so)
target_link_libraries(${PROJECT_NAME} ${MODULE_LIB})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -Wl,--no-as-needed)