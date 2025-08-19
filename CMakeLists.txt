cmake_minimum_required(VERSION 3.5)
project(mnn-asr)
set(PROJECT_NAME asr_demo)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(MNN_LOW_MEMORY ON CACHE BOOL "Open MNN_LOW_MEMORY" FORCE)
set(MNN_SUPPORT_TRANSFORMER_FUSE ON CACHE BOOL "Open MNN_SUPPORT_TRANSFORMER_FUSE" FORCE)
set(MNN_BUILD_AUDIO ON CACHE BOOL "Open MNN_BUILD_AUDIO" FORCE)

# include dir
include_directories(src/include/
                    ${MNN_DIR}/include/
#                    ${MNN_DIR}/MNN/3rd_party/
                    ${MNN_DIR}/audio/
                    )

# source files
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SRC_FILES ${ROOT_DIR})
include(${ROOT_DIR}/cmake/find_lib_files.cmake)
list(APPEND SRC_DIR ${sub_dirs})

include_directories(${SRC_FILES})
find_source_file(SRC_FILES "*.h" "*.cpp" "*.c" "*.cc" "*.hpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})


# 库目录
target_link_directories(${PROJECT_NAME} PRIVATE
        ${MNN_DIR}/lib
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
        MNN
)