cmake_minimum_required(VERSION 3.22)

set(CPM_DOWNLOAD_VERSION 0.35.5)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

project(circle)

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:SRombauts/SQLiteCpp#3.3.0")
CPMAddPackage("gh:nlohmann/json#v3.11.2")
CPMAddPackage("gh:gabime/spdlog#v1.12.0")
CPMAddPackage("gh:robinlinden/libsodium-cmake#master")
CPMAddPackage("gh:effolkronium/random#v1.5.0")

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options regex)

set(SHARED_INCLUDE_DIRS
        include
        ${nlohmann_json_SOURCE_DIR}/single_include
        ${Boost_INCLUDE_DIRS}
        ${spdlog_SOURCE_DIR}/include
        ${fmt_SOURCE_DIR}/include
)

set(CIRCLE_INCLUDE_DIRS
        ${SHARED_INCLUDE_DIRS}
        ${eff_random_SOURCE_DIR}/include
        ${SQLiteCpp_SOURCE_DIR}/include
        ${sodium_SOURCE_DIR}/libsodium/src/libsodium/include
        ${effolkronium_random_SOURCE_DIR}/include
        ${EnTT_SOURCE_DIR}/single_include
        )

set(PORTAL_INCLUDE_DIRS
        ${SHARED_INCLUDE_DIRS}
        )



file(GLOB_RECURSE CIRCLE_INCLUDE include/dbat/*.h include/shared/*.h)
file(GLOB_RECURSE CIRCLE_SRC src/dbat/*.cpp)

file(GLOB_RECURSE PORTAL_SRC src/portal/*.cpp)
file(GLOB_RECURSE PORTAL_INCLUDE include/portal/*.h include/shared/*.h)

file(GLOB_RECURSE SHARED_SRC src/shared/*.cpp)
file(GLOB_RECURSE SHARED_INCLUDE include/shared/*.h)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_FLAGS "-fpermissive -w")

set(BUILD_TESTING OFF)

set(CIRCLE_LINK_LIBRARIES
        fmt::fmt ${SQLite3_LIBRARIES} ${Boost_LIBRARIES}
        SQLiteCpp sqlite3 bsd
        #Luau.VM Luau.Common Luau.Compiler
        sodium
        effolkronium_random
        )

set(PORTAL_LINK_LIBRARIES
        fmt::fmt ${Boost_LIBRARIES}
        ssl
        crypto
)

add_library(shared_lib ${SHARED_INCLUDE} ${SHARED_SRC})
target_include_directories(shared_lib PUBLIC ${SHARED_INCLUDE_DIRS})

# this is the core library we're making.
add_library(circlemud ${CIRCLE_INCLUDE} ${CIRCLE_SRC})
# everything we make will link to it.
target_link_libraries(circlemud shared_lib ${CIRCLE_LINK_LIBRARIES})
target_include_directories(circlemud PUBLIC ${CIRCLE_INCLUDE_DIRS})


SET(circle_link ${CMAKE_INSTALL_PREFIX}/bin/)