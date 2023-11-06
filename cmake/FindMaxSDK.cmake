set(_Max_sdk_library_name geom gfx gup maxscrpt assetmanagement bmm paramblk2 maxutil mesh igame)
set(_Max_skd_library_path lib/x64/Release)
set(_Max_skd_include_path include)

set(MaxSDK_FOUND FALSE)

find_path(MaxSDK_INCLUDE_DIR 
    NAMES max.h
    PATH_SUFFIXES
        ${_Max_skd_include_path}
    )

find_library(MaxSDK_LIBRARY
    NAMES core
    PATH_SUFFIXES
        ${_Max_skd_library_path}
    REQUIRED
    )

set(_Max_sdk_components)

foreach(MaxSDK_LIB ${_Max_sdk_library_name})
    string(TOUPPER ${MaxSDK_LIB} _MaxSDK_LIB_)
    find_library(MaxSDK_${_MaxSDK_LIB_}_LIBRARY
        NAMES ${MaxSDK_LIB}
        PATH_SUFFIXES
            ${_Max_skd_library_path}
        )
    if(MaxSDK_${_MaxSDK_LIB_}_LIBRARY)
        set(MaxSDK_${_MaxSDK_LIB_}_FOUND TRUE)
    else()
        message(FATAL_ERROR "${MaxSDK_LIB} library not found")
        set(MaxSDK_${_MaxSDK_LIB_}_FOUND FALSE)
    endif()
endforeach()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MaxSDK
  REQUIRED_VARS
    MaxSDK_INCLUDE_DIR
    MaxSDK_LIBRARY
  HANDLE_COMPONENTS
)

if(MaxSDK_FOUND)
    mark_as_advanced(MaxSDK_INCLUDE_DIR)
    mark_as_advanced(MaxSDK_LIBRARY)
    set(MaxSDK_LIBRARIES ${MaxSDK_LIBRARY})
    set(MaxSDK_INCLUDE_DIRS ${MaxSDK_INCLUDE_DIR})

    if(NOT TARGET MaxSDK::MaxSDK)
        add_library(MaxSDK::MaxSDK UNKNOWN IMPORTED) #UNKNOWN - ?
        set_target_properties(MaxSDK::MaxSDK PROPERTIES
            IMPORTED_LOCATION "${MaxSDK_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${MaxSDK_INCLUDE_DIRS}")
    endif()

    foreach(MaxSDK_LIB ${_Max_sdk_library_name})
        string(TOUPPER ${MaxSDK_LIB} _MaxSDK_LIB_)
        if (MaxSDK_${_MaxSDK_LIB_}_FOUND AND NOT TARGET MaxSDK::${_MaxSDK_LIB_})
            mark_as_advanced(MaxSDK_${_MaxSDK_LIB_}_LIBRARY)
            add_library(MaxSDK::${_MaxSDK_LIB_} UNKNOWN IMPORTED)
            set_target_properties(MaxSDK::${_MaxSDK_LIB_} PROPERTIES
                IMPORTED_LOCATION "${MaxSDK_${_MaxSDK_LIB_}_LIBRARY}")
            target_link_libraries(MaxSDK::${_MaxSDK_LIB_} INTERFACE MaxSDK::MaxSDK)
            #INTERFACE_INCLUDE_DIRECTORIES "${MaxSDK_INCLUDE_DIRS}")
            #target_link_libraries(MaxSDK::${_MaxSDK_LIB_} INTERFACE MaxSDK::MaxSDK)
        endif()
    endforeach()
endif()