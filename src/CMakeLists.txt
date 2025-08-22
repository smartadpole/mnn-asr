cmake_minimum_required(VERSION 3.5)
set(PROJECT_NAME ASL)
set(LIBRARY_VERSION 1.0.0)
project(${PROJECT_NAME})
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../)

option(USE_GPU "Enable GPU(CUDA) support" OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# include dir
include_directories(src/include/)

# source files
set(SRC_FILES ${ROOT_DIR})
include(${ROOT_DIR}/cmake/find_lib_files.cmake)
list(APPEND SRC_DIR ${sub_dirs})

include_directories(${SRC_FILES})
find_source_file(SRC_FILES "*.h" "*.cpp" "*.c" "*.cc" "*.hpp")
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
SET_TARGET_PROPERTIES(${LIBRARY_NAME} PROPERTIES VERSION ${LIBRARY_VERSION} SOVERSION 1)


# CNN lib
include(${ROOT_DIR}/cmake/find_device.cmake)
target_link_libraries(${PROJECT_NAME}  ${CNN_LIBRARY})

# make install
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(LIB_GENERAET "lib/x64")
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(LIB_GENERAET "lib/arm64")
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "armv7-a" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm")
    set(LIB_GENERAET "lib/arm")
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "i686" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86")
    set(LIB_GENERAET "lib/x86")
else()
    set(LIB_GENERAET "lib/${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

install(TARGETS ${LIBRARY_NAME}
        RUNTIME DESTINATION bin ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${LIB_GENERAET} ${CMAKE_INSTALL_LIBDIR}
        #        PUBLIC_HEADER DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR}
        ARCHIVE DESTINATION lib ${CMAKE_INSTALL_LIBDIR}
)
#set(INSTALL_HEAD_FILES ${LIBRARY_NAME}.h)
#install(FILES ${INSTALL_HEAD_FILES} DESTINATION  ${CMAKE_INSTALL_PREFIX}/include)
#install(FILES ${ROOT_DIR}/src/perception.h DESTINATION  ${CMAKE_INSTALL_PREFIX}/include)
#install(DIRECTORY ${ROOT_DIR}/include DESTINATION  ${CMAKE_INSTALL_PREFIX}/)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_SYNC} DESTINATION  ../)