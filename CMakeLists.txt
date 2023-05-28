cmake_minimum_required(VERSION 3.1)

set(REDIS_LLM_VERSION "0.0.1")
message(STATUS "redis-llm version: ${REDIS_LLM_VERSION}")

project(redis-llm LANGUAGES CXX VERSION ${REDIS_LLM_VERSION})

set(REDIS_LLM_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE ${REDIS_LLM_DEFAULT_BUILD_TYPE} CACHE STRING "Set build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()
message(STATUS "redis-llm build type: ${CMAKE_BUILD_TYPE}")

#set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Werror -fPIC -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -fPIC -Wno-unused-parameter")

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/sw/redis-llm)

set(REDIS_LLM_HEADER_DIR src)

file(GLOB PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/*.cpp")

find_path(NLOHMANN_JSON_HEADER nlohmann)

find_package(OpenSSL REQUIRED)

set(SHARED_LIB shared)

add_library(${SHARED_LIB} SHARED ${PROJECT_SOURCE_FILES})

target_include_directories(${SHARED_LIB} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${REDIS_LLM_HEADER_DIR}>
        $<BUILD_INTERFACE:${NLOHMANN_JSON_HEADER}>
        $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(${SHARED_LIB} PUBLIC ${OPENSSL_LIBRARIES})

target_link_libraries(${SHARED_LIB} PUBLIC "-framework AppKit -framework Security")

set_target_properties(${SHARED_LIB} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

set_target_properties(${SHARED_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

include(GNUInstallDirs)

# Install shared lib.
install(TARGETS ${SHARED_LIB}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
