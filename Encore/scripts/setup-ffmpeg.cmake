function(setup_ffmpeg)
    set(FFMPEG_VERSION "6.0")

    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(FFMPEG_BASE_URL "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest")
            set(FFMPEG_PACKAGE "ffmpeg-master-latest-win64-gpl-shared")
            set(PLATFORM_NAME "Windows x64")
            set(ARCHIVE_EXT "zip")
            set(USE_DOWNLOAD TRUE)
        else()
            set(FFMPEG_BASE_URL "https://github.com/yt-dlp/FFmpeg-Builds/releases/download/latest")
            set(FFMPEG_PACKAGE "ffmpeg-master-latest-win32-gpl-shared")
            set(PLATFORM_NAME "Windows x86")
            set(ARCHIVE_EXT "zip")
            set(USE_DOWNLOAD TRUE)
        endif()
        set(LIB_PREFIX "")
        set(LIB_SUFFIX ".lib")
        set(SHARED_SUFFIX ".dll")
    elseif(APPLE)
        set(PLATFORM_NAME "macOS")
        set(LIB_PREFIX "lib")
        set(LIB_SUFFIX ".dylib")
        set(SHARED_SUFFIX ".dylib")
        set(USE_DOWNLOAD FALSE)
        set(BUILD_FROM_SOURCE TRUE)
    elseif(UNIX)
        set(PLATFORM_NAME "Linux x64")
        set(LIB_PREFIX "lib")
        set(LIB_SUFFIX ".so")
        set(SHARED_SUFFIX ".so")
        set(USE_DOWNLOAD FALSE)
        set(USE_LOCAL TRUE)
    endif()
    
    message(STATUS "Setting up FFmpeg for ${PLATFORM_NAME}")

    if(USE_DOWNLOAD)
        set(FFMPEG_URL "${FFMPEG_BASE_URL}/${FFMPEG_PACKAGE}.${ARCHIVE_EXT}")
        set(FFMPEG_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg" PARENT_SCOPE)

        include(FetchContent)
        FetchContent_Declare(
            ffmpeg_prebuilt
            URL ${FFMPEG_URL}
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
        )
        
        FetchContent_GetProperties(ffmpeg_prebuilt)
        if(NOT ffmpeg_prebuilt_POPULATED)
            message(STATUS "Downloading FFmpeg prebuilt binaries...")
            FetchContent_MakeAvailable(ffmpeg_prebuilt)

            file(COPY ${ffmpeg_prebuilt_SOURCE_DIR}/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg)
            message(STATUS "FFmpeg setup complete for ${PLATFORM_NAME}")
        endif()

        set(FFMPEG_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/include" PARENT_SCOPE)
        set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib" PARENT_SCOPE)
        set(FFMPEG_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/bin" PARENT_SCOPE)
    elseif(BUILD_FROM_SOURCE)
        message(STATUS "Building FFmpeg from source for ${PLATFORM_NAME}")
        build_ffmpeg_macos_from_source()
        

        set(FFMPEG_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg")
        set(FFMPEG_INCLUDE_DIR "${FFMPEG_INSTALL_PREFIX}/include" PARENT_SCOPE)
        set(FFMPEG_LIB_DIR "${FFMPEG_INSTALL_PREFIX}/lib" PARENT_SCOPE)
        set(FFMPEG_BIN_DIR "${FFMPEG_INSTALL_PREFIX}/bin" PARENT_SCOPE)
    elseif(USE_LOCAL)
        set(FFMPEG_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg/linux")
        
        # Check if local FFmpeg exists
        if(NOT EXISTS "${FFMPEG_LOCAL_DIR}")
            message(FATAL_ERROR "FFmpeg not found at ${FFMPEG_LOCAL_DIR}. Please ensure FFmpeg is installed in the lib folder with the expected structure.")
        endif()
        
        message(STATUS "Using local FFmpeg from: ${FFMPEG_LOCAL_DIR}")
        
        # Set the output variables to point to local FFmpeg
        set(FFMPEG_INCLUDE_DIR "${FFMPEG_LOCAL_DIR}/include" PARENT_SCOPE)
        set(FFMPEG_LIB_DIR "${FFMPEG_LOCAL_DIR}/lib" PARENT_SCOPE)
        set(FFMPEG_BIN_DIR "${FFMPEG_LOCAL_DIR}/bin" PARENT_SCOPE)
    else()
        # Use local FFmpeg from lib folder (fallback)
        set(FFMPEG_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/${FFMPEG_FOLDER}")
        
        # Check if local FFmpeg exists
        if(NOT EXISTS "${FFMPEG_LOCAL_DIR}")
            message(FATAL_ERROR "FFmpeg not found at ${FFMPEG_LOCAL_DIR}. Please ensure FFmpeg is installed in the lib folder with the expected structure.")
        endif()
        
        message(STATUS "Using local FFmpeg from: ${FFMPEG_LOCAL_DIR}")
        
        # Set the output variables to point to local FFmpeg
        set(FFMPEG_INCLUDE_DIR "${FFMPEG_LOCAL_DIR}/include" PARENT_SCOPE)
        set(FFMPEG_LIB_DIR "${FFMPEG_LOCAL_DIR}/lib" PARENT_SCOPE)
        set(FFMPEG_BIN_DIR "${FFMPEG_LOCAL_DIR}/bin" PARENT_SCOPE)
    endif()
    set(FFMPEG_LIB_PREFIX "${LIB_PREFIX}" PARENT_SCOPE)
    set(FFMPEG_LIB_SUFFIX "${LIB_SUFFIX}" PARENT_SCOPE)
    set(FFMPEG_SHARED_SUFFIX "${SHARED_SUFFIX}" PARENT_SCOPE)
    
endfunction()

function(find_ffmpeg_libraries)
    if(APPLE)
        # For macOS, search in the specified paths first, then allow default paths
        find_library(AVFORMAT_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avformat${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(AVCODEC_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avcodec${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(AVUTIL_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avutil${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(SWSCALE_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}swscale${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(SWRESAMPLE_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}swresample${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(AVFILTER_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avfilter${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(AVDEVICE_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avdevice${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
        find_library(POSTPROC_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}postproc${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR}
            PATH_SUFFIXES lib
        )
    else()
        find_library(AVFORMAT_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avformat${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(AVCODEC_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avcodec${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(AVUTIL_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avutil${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(SWSCALE_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}swscale${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(SWRESAMPLE_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}swresample${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(AVFILTER_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avfilter${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(AVDEVICE_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}avdevice${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
        find_library(POSTPROC_LIB 
            NAMES ${FFMPEG_LIB_PREFIX}postproc${FFMPEG_LIB_SUFFIX}
            PATHS ${FFMPEG_LIB_DIR} 
            NO_DEFAULT_PATH
        )
    endif()

    # Debug output for macOS
    if(APPLE)
        message(STATUS "FFmpeg library search results:")
        message(STATUS "  AVFORMAT_LIB: ${AVFORMAT_LIB}")
        message(STATUS "  AVCODEC_LIB: ${AVCODEC_LIB}")
        message(STATUS "  AVUTIL_LIB: ${AVUTIL_LIB}")
        message(STATUS "  SWSCALE_LIB: ${SWSCALE_LIB}")
        message(STATUS "  FFMPEG_LIB_DIR: ${FFMPEG_LIB_DIR}")
        message(STATUS "  FFMPEG_LIB_PREFIX: ${FFMPEG_LIB_PREFIX}")
        message(STATUS "  FFMPEG_LIB_SUFFIX: ${FFMPEG_LIB_SUFFIX}")
        
        # Debug: List what's actually in the lib directory
        if(EXISTS ${FFMPEG_LIB_DIR})
            file(GLOB LIB_FILES "${FFMPEG_LIB_DIR}/*")
            message(STATUS "Files in FFMPEG_LIB_DIR:")
            foreach(FILE ${LIB_FILES})
                message(STATUS "    ${FILE}")
            endforeach()
        else()
            message(STATUS "FFMPEG_LIB_DIR does not exist: ${FFMPEG_LIB_DIR}")
        endif()
    endif()

    if(NOT AVFORMAT_LIB OR NOT AVCODEC_LIB OR NOT AVUTIL_LIB OR NOT SWSCALE_LIB)
        if(APPLE)
            # Try using pkg-config as fallback for macOS
            find_package(PkgConfig QUIET)
            if(PKG_CONFIG_FOUND)
                message(STATUS "Trying pkg-config as fallback...")
                pkg_check_modules(PC_AVFORMAT QUIET libavformat)
                pkg_check_modules(PC_AVCODEC QUIET libavcodec)
                pkg_check_modules(PC_AVUTIL QUIET libavutil)
                pkg_check_modules(PC_SWSCALE QUIET libswscale)
                pkg_check_modules(PC_SWRESAMPLE QUIET libswresample)
                pkg_check_modules(PC_AVFILTER QUIET libavfilter)
                pkg_check_modules(PC_AVDEVICE QUIET libavdevice)
                
                if(PC_AVFORMAT_FOUND AND PC_AVCODEC_FOUND AND PC_AVUTIL_FOUND AND PC_SWSCALE_FOUND)
                    set(AVFORMAT_LIB ${PC_AVFORMAT_LIBRARIES})
                    set(AVCODEC_LIB ${PC_AVCODEC_LIBRARIES})
                    set(AVUTIL_LIB ${PC_AVUTIL_LIBRARIES})
                    set(SWSCALE_LIB ${PC_SWSCALE_LIBRARIES})
                    if(PC_SWRESAMPLE_FOUND)
                        set(SWRESAMPLE_LIB ${PC_SWRESAMPLE_LIBRARIES})
                    endif()
                    if(PC_AVFILTER_FOUND)
                        set(AVFILTER_LIB ${PC_AVFILTER_LIBRARIES})
                    endif()
                    if(PC_AVDEVICE_FOUND)
                        set(AVDEVICE_LIB ${PC_AVDEVICE_LIBRARIES})
                    endif()
                    message(STATUS "Found FFmpeg libraries via pkg-config")
                endif()
            endif()
        endif()
        
        if(NOT AVFORMAT_LIB OR NOT AVCODEC_LIB OR NOT AVUTIL_LIB OR NOT SWSCALE_LIB)
            message(STATUS "Final FFmpeg library search results:")
            message(STATUS "  AVFORMAT_LIB: ${AVFORMAT_LIB}")
            message(STATUS "  AVCODEC_LIB: ${AVCODEC_LIB}")
            message(STATUS "  AVUTIL_LIB: ${AVUTIL_LIB}")
            message(STATUS "  SWSCALE_LIB: ${SWSCALE_LIB}")
            message(STATUS "  FFMPEG_LIB_DIR: ${FFMPEG_LIB_DIR}")
            message(STATUS "  FFMPEG_LIB_PREFIX: ${FFMPEG_LIB_PREFIX}")
            message(STATUS "  FFMPEG_LIB_SUFFIX: ${FFMPEG_LIB_SUFFIX}")
            message(FATAL_ERROR "Required FFmpeg libraries not found!")
        endif()
    endif()

    set(FFMPEG_LIBRARIES_LIST)

    if(AVDEVICE_LIB)
        list(APPEND FFMPEG_LIBRARIES_LIST ${AVDEVICE_LIB})
    endif()
    if(AVFILTER_LIB)
        list(APPEND FFMPEG_LIBRARIES_LIST ${AVFILTER_LIB})
    endif()
    list(APPEND FFMPEG_LIBRARIES_LIST ${AVFORMAT_LIB})
    list(APPEND FFMPEG_LIBRARIES_LIST ${AVCODEC_LIB})
    if(POSTPROC_LIB)
        list(APPEND FFMPEG_LIBRARIES_LIST ${POSTPROC_LIB})
    endif()
    if(SWRESAMPLE_LIB)
        list(APPEND FFMPEG_LIBRARIES_LIST ${SWRESAMPLE_LIB})
    endif()
    list(APPEND FFMPEG_LIBRARIES_LIST ${SWSCALE_LIB})
    list(APPEND FFMPEG_LIBRARIES_LIST ${AVUTIL_LIB})

    set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES_LIST} PARENT_SCOPE)
    
    message(STATUS "Found FFmpeg libraries:")
    message(STATUS "  AVFORMAT: ${AVFORMAT_LIB}")
    message(STATUS "  AVCODEC: ${AVCODEC_LIB}")
    message(STATUS "  AVUTIL: ${AVUTIL_LIB}")
    message(STATUS "  SWSCALE: ${SWSCALE_LIB}")
    if(SWRESAMPLE_LIB)
        message(STATUS "  SWRESAMPLE: ${SWRESAMPLE_LIB}")
    endif()
    if(AVFILTER_LIB)
        message(STATUS "  AVFILTER: ${AVFILTER_LIB}")
    endif()
    if(AVDEVICE_LIB)
        message(STATUS "  AVDEVICE: ${AVDEVICE_LIB}")
    endif()
    if(POSTPROC_LIB)
        message(STATUS "  POSTPROC: ${POSTPROC_LIB}")
    endif()
endfunction()

function(copy_ffmpeg_binaries TARGET_DIR)
    if(WIN32)
        file(GLOB FFMPEG_BINARIES "${FFMPEG_BIN_DIR}/*${FFMPEG_SHARED_SUFFIX}")
        if(FFMPEG_BINARIES)
            file(COPY ${FFMPEG_BINARIES} DESTINATION ${TARGET_DIR})
            list(LENGTH FFMPEG_BINARIES DLL_COUNT)
            message(STATUS "Copied ${DLL_COUNT} FFmpeg DLLs to ${TARGET_DIR}")

            foreach(DLL ${FFMPEG_BINARIES})
                get_filename_component(DLL_NAME ${DLL} NAME)
                message(STATUS "  - ${DLL_NAME}")
            endforeach()
        else()
            message(WARNING "No FFmpeg DLLs found in ${FFMPEG_BIN_DIR}")
        endif()
    elseif(UNIX)
        file(GLOB FFMPEG_BINARIES "${FFMPEG_LIB_DIR}/*${FFMPEG_SHARED_SUFFIX}*")
        if(FFMPEG_BINARIES)
            file(COPY ${FFMPEG_BINARIES} DESTINATION ${TARGET_DIR})
            message(STATUS "Copied FFmpeg shared libraries to ${TARGET_DIR}")
        endif()
    endif()
endfunction()

function(setup_all_post_build TARGET_NAME TARGET_DIR)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying assets and dependencies..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_DIR}"
        
        # Copy assets (Songs and Assets folders)
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Songs"
            "${TARGET_DIR}/Songs"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
            "${TARGET_DIR}/Assets"
    )
    
    if(WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            # Copy Discord RPC, BASS, and BASSOPUS DLLs based on architecture
            COMMAND ${CMAKE_COMMAND} -E echo "Copying Windows-specific libraries..."
        )
        

        
        # Copy FFmpeg DLLs from local lib folder
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${FFMPEG_BIN_DIR}"
                "${TARGET_DIR}"
            
            COMMENT "Copying Windows dependencies to output directory"
        )
    elseif(UNIX AND NOT APPLE)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            # Copy FFmpeg .so files using file(GLOB) in a custom script
            COMMAND ${CMAKE_COMMAND} -E echo "Copying FFmpeg .so files..."
            COMMAND ${CMAKE_COMMAND} -E env FFMPEG_LIB_DIR=${FFMPEG_LIB_DIR} TARGET_DIR=${TARGET_DIR} SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/scripts/copy-ffmpeg-libs.cmake
            
            COMMENT "Copying Linux dependencies to output directory"
        )
    elseif(APPLE)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMENT "Copying macOS dependencies to output directory"
        )
        
        if(FFMPEG_LIB_DIR)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Copying FFmpeg libraries from ${FFMPEG_LIB_DIR}..."
                COMMAND ${CMAKE_COMMAND} -E env FFMPEG_LIB_DIR=${FFMPEG_LIB_DIR} TARGET_DIR=${TARGET_DIR} ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/scripts/copy-ffmpeg-libs-macos.cmake
                COMMENT "Copying FFmpeg dylibs to output directory"
            )
        endif()
    endif()
endfunction()

# Legacy function for backward compatibility
function(setup_ffmpeg_post_build TARGET_NAME TARGET_DIR)
    setup_all_post_build(${TARGET_NAME} ${TARGET_DIR})
endfunction()