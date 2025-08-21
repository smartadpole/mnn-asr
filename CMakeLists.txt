cmake_minimum_required(VERSION 3.5)
project(mnn-asr)
set(PROJECT_NAME mnn_asr)

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

# 创建静态库
add_library(${PROJECT_NAME}_static STATIC ${SRC_FILES})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# 创建动态库
add_library(${PROJECT_NAME}_shared SHARED ${SRC_FILES})
set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# 设置库的属性
if(WIN32)
    target_compile_definitions(${PROJECT_NAME}_shared PRIVATE MNN_ASR_EXPORTS)
else()
    set_target_properties(${PROJECT_NAME}_shared PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

# 创建可执行文件（原始功能）
add_executable(asr_demo ${SRC_FILES})


# 添加CUDA支持选项
option(USE_GPU "Enable GPU(CUDA) support" OFF)
find_library(MNN_LIB NAMES MNN PATHS ${MNN_DIR}/lib REQUIRED)

if(USE_GPU)
    find_library(MNN_CUDA_MAIN NAMES MNN_Cuda_Main PATHS ${MNN_DIR}/lib REQUIRED)
endif()

# 链接库到静态库
target_link_directories(${PROJECT_NAME}_static PRIVATE ${MNN_DIR}/lib)
target_link_libraries(${PROJECT_NAME}_static PRIVATE MNN)

if(USE_GPU)
    target_link_libraries(${PROJECT_NAME}_static PRIVATE MNN_Cuda_Main)
    target_compile_definitions(${PROJECT_NAME}_static PRIVATE USE_GPU)
endif()

# 链接库到动态库
target_link_directories(${PROJECT_NAME}_shared PRIVATE ${MNN_DIR}/lib)
target_link_libraries(${PROJECT_NAME}_shared PRIVATE MNN)

if(USE_GPU)
    target_link_libraries(${PROJECT_NAME}_shared PRIVATE MNN_Cuda_Main)
    target_compile_definitions(${PROJECT_NAME}_shared PRIVATE USE_GPU)
endif()

# 链接库到可执行文件
target_link_directories(asr_demo PRIVATE ${MNN_DIR}/lib)
target_link_libraries(asr_demo PRIVATE MNN)

if(USE_GPU)
    target_link_libraries(asr_demo PRIVATE MNN_Cuda_Main)
    target_compile_definitions(asr_demo PRIVATE USE_GPU)
endif()

# 构建示例程序
add_executable(asr_sample sample/asr_sample.cpp)
target_link_libraries(asr_sample PRIVATE ${PROJECT_NAME}_static)
target_include_directories(asr_sample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 安装规则
install(TARGETS ${PROJECT_NAME}_static ${PROJECT_NAME}_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES src/mnn_asr_api.hpp
        DESTINATION include/mnn_asr)

install(FILES sample/README.md
        DESTINATION share/mnn_asr)