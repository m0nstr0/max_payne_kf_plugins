project("MaxPayneKFImporter")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE MAX_ENGINE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.rc ${CMAKE_CURRENT_SOURCE_DIR}/*.def)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MAX_ENGINE_SOURCE})

add_library(MaxPayneKFImporter SHARED ${MAX_ENGINE_SOURCE})

target_link_libraries(MaxPayneKFImporter 
    MaxSDK::MaxSDK
    MaxSDK::GEOM
    MaxSDK::GFX
    MaxSDK::GUP
    MaxSDK::MAXSCRPT
    MaxSDK::ASSETMANAGEMENT
    MaxSDK::BMM
    MaxSDK::PARAMBLK2
    MaxSDK::MAXUTIL
    MaxSDK::MESH
)

set_target_properties(MaxPayneKFImporter PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(MaxPayneKFImporter PROPERTIES OUTPUT_NAME "MaxPayneKFImporter" SUFFIX ".dlt")

if (MSVC)
    target_compile_definitions(MaxPayneKFImporter PUBLIC UNICODE _UNICODE)
    set_target_properties(MaxPayneKFImporter PROPERTIES LINK_FLAGS "/DEF:MaxPayneKFImporter.def")
endif()


target_include_directories(MaxPayneKFImporter
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )