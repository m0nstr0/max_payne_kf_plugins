cmake_minimum_required(VERSION 3.26)

set(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(THIRD_PARTY_3D_MAX_SDK "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/maxsdk" CACHE PATH "Path to 3D MAX 2024 SDK root directory")

set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin" CACHE PATH "Path to runtime output root directory")

set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin" CACHE PATH "Path to library output root directory")

set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin" CACHE PATH "Path to archive output root directory")

list(APPEND CMAKE_PREFIX_PATH "${THIRD_PARTY_3D_MAX_SDK}")

# First for the generic no-config case (e.g. with mingw)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY} )

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY} )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ARCHIVE_OUTPUT_DIRECTORY} )

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )

    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RUNTIME_OUTPUT_DIRECTORY} )

    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBRARY_OUTPUT_DIRECTORY} )

    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${ARCHIVE_OUTPUT_DIRECTORY} )

endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

project("MaxPayneKFPlugins")

find_package(MaxSDK REQUIRED)

add_subdirectory(MaxPayneCore)

add_subdirectory(MaxPayneKFMaterial)

add_subdirectory(MaxPayneKFExporter)

add_subdirectory(MaxPayneKFImporter)
