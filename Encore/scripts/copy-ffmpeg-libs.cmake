set(FFMPEG_LIB_DIR "$ENV{FFMPEG_LIB_DIR}")
set(TARGET_DIR "$ENV{TARGET_DIR}")

if(FFMPEG_LIB_DIR AND TARGET_DIR)

    message(STATUS "Checking FFmpeg lib directory: ${FFMPEG_LIB_DIR}")
    if(EXISTS ${FFMPEG_LIB_DIR})
        file(GLOB ALL_LIB_FILES "${FFMPEG_LIB_DIR}/*")
        message(STATUS "Available files in FFmpeg lib directory:")
        foreach(FILE ${ALL_LIB_FILES})
            get_filename_component(FILENAME ${FILE} NAME)
            message(STATUS "  - ${FILENAME}")
        endforeach()
        
        set(FFMPEG_LIBS "avcodec" "avformat" "avutil" "swscale" "swresample" "avfilter" "avdevice")
        set(COPIED_COUNT 0)
        
        foreach(LIB_BASE ${FFMPEG_LIBS})
            file(GLOB VERSIONED_FILES "${FFMPEG_LIB_DIR}/lib${LIB_BASE}.so.[0-9]*")
            foreach(VERSIONED_FILE ${VERSIONED_FILES})
                get_filename_component(LIB_NAME ${VERSIONED_FILE} NAME)
                message(STATUS "  Copying versioned file: ${LIB_NAME}")
                configure_file(${VERSIONED_FILE} ${TARGET_DIR}/${LIB_NAME} COPYONLY)
                
                if(EXISTS "${TARGET_DIR}/${LIB_NAME}")
                    message(STATUS "    ✓ Successfully copied to ${TARGET_DIR}/${LIB_NAME}")
                    math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
                else()
                    message(STATUS "    ✗ Failed to copy ${LIB_NAME}")
                endif()
            endforeach()
            
            set(BASE_FILE "${FFMPEG_LIB_DIR}/lib${LIB_BASE}.so")
            if(EXISTS ${BASE_FILE})
                message(STATUS "  Copying base file: lib${LIB_BASE}.so")
                configure_file(${BASE_FILE} ${TARGET_DIR}/lib${LIB_BASE}.so COPYONLY)
                
                if(EXISTS "${TARGET_DIR}/lib${LIB_BASE}.so")
                    message(STATUS "    ✓ Successfully copied to ${TARGET_DIR}/lib${LIB_BASE}.so")
                    math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
                else()
                    message(STATUS "    ✗ Failed to copy lib${LIB_BASE}.so")
                endif()
            endif()
        endforeach()
    else()
        message(STATUS "FFmpeg lib directory does not exist!")
        # Try fallback to local FFmpeg
        set(SOURCE_DIR "$ENV{SOURCE_DIR}")
        set(LOCAL_FFMPEG_LIB_DIR "${SOURCE_DIR}/lib/ffmpeg/linux/lib")
        message(STATUS "Trying fallback local FFmpeg libraries from: ${LOCAL_FFMPEG_LIB_DIR}")
        
        if(EXISTS ${LOCAL_FFMPEG_LIB_DIR})
            file(GLOB LOCAL_SO_FILES "${LOCAL_FFMPEG_LIB_DIR}/*.so*")
            message(STATUS "Local FFmpeg files found:")
            foreach(FILE ${LOCAL_SO_FILES})
                get_filename_component(FILENAME ${FILE} NAME)
                message(STATUS "  - ${FILENAME}")
            endforeach()
            
            set(COPIED_COUNT 0)
            foreach(LIB_FILE ${LOCAL_SO_FILES})
                get_filename_component(LIB_NAME ${LIB_FILE} NAME)
                
                if(LIB_NAME MATCHES "^lib.*\\.so.*$")
                    message(STATUS "  Copying local file: ${LIB_NAME}")
                    configure_file(${LIB_FILE} ${TARGET_DIR}/${LIB_NAME} COPYONLY)
                    
                    if(EXISTS "${TARGET_DIR}/${LIB_NAME}")
                        message(STATUS "    ✓ Successfully copied to ${TARGET_DIR}/${LIB_NAME}")
                        math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
                    else()
                        message(STATUS "    ✗ Failed to copy ${LIB_NAME}")
                    endif()
                else()
                    message(STATUS "  Skipping: ${LIB_NAME} (not a library file)")
                endif()
            endforeach()
        else()
            message(FATAL_ERROR "Neither downloaded nor local FFmpeg lib directory found")
        endif()
    endif()
    
    set(FFMPEG_LIBS_TO_LINK
        "avcodec"
        "avformat"
        "avutil"
        "swscale"
        "avfilter"
        "avdevice"
        "postproc"
    )
    
    foreach(LIB_BASE ${FFMPEG_LIBS_TO_LINK})
        file(GLOB MAJOR_VERSION_FILE "${TARGET_DIR}/lib${LIB_BASE}.so.[0-9]*")
        if(MAJOR_VERSION_FILE)
            list(GET MAJOR_VERSION_FILE 0 VERSION_FILE)
            get_filename_component(VERSION_FILENAME ${VERSION_FILE} NAME)
            
            set(BASE_SYMLINK "${TARGET_DIR}/lib${LIB_BASE}.so")
            
            if(NOT EXISTS ${BASE_SYMLINK})
                message(STATUS "  Creating symlink: lib${LIB_BASE}.so -> ${VERSION_FILENAME}")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${VERSION_FILENAME} lib${LIB_BASE}.so
                    WORKING_DIRECTORY ${TARGET_DIR}
                    RESULT_VARIABLE SYMLINK_RESULT
                )
                if(SYMLINK_RESULT EQUAL 0)
                    message(STATUS "    ✓ Symlink created successfully")
                else()
                    message(STATUS "    ✗ Failed to create symlink (result: ${SYMLINK_RESULT})")
                endif()
            else()
                message(STATUS "  Symlink already exists: lib${LIB_BASE}.so")
            endif()
        else()
            message(STATUS "  No major version file found for lib${LIB_BASE}")
        endif()
    endforeach()
    
    message(STATUS "Final files in target directory:")
    file(GLOB TARGET_FILES "${TARGET_DIR}/lib*.so*")
    foreach(FILE ${TARGET_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        if(IS_SYMLINK ${FILE})
            message(STATUS "  - ${FILENAME} (symlink)")
        else()
            message(STATUS "  - ${FILENAME}")
        endif()
    endforeach()
    



    
    find_program(PATCHELF_PROGRAM patchelf)
    find_program(CHRPATH_PROGRAM chrpath)
    
    if(PATCHELF_PROGRAM)
        message(STATUS "Fixing RPATH and dependencies on copied libraries...")
        file(GLOB COPIED_LIBS "${TARGET_DIR}/lib*.so*")
        foreach(LIB_FILE ${COPIED_LIBS})
            get_filename_component(LIB_NAME ${LIB_FILE} NAME)
            

            if(NOT IS_SYMLINK ${LIB_FILE})
                message(STATUS "  Processing ${LIB_NAME}")
                

                execute_process(
                    COMMAND ${PATCHELF_PROGRAM} --set-rpath '$ORIGIN' ${LIB_FILE}
                    RESULT_VARIABLE RPATH_RESULT
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                
                if(RPATH_RESULT EQUAL 0)
                    message(STATUS "    ✓ RPATH set successfully")
                else()
                    message(STATUS "    Warning: Failed to set RPATH")
                endif()
                


            endif()
        endforeach()
    elseif(CHRPATH_PROGRAM)
        message(STATUS "Using chrpath to fix RPATH...")
        file(GLOB COPIED_LIBS "${TARGET_DIR}/lib*.so.[0-9]*")
        foreach(LIB_FILE ${COPIED_LIBS})
            get_filename_component(LIB_NAME ${LIB_FILE} NAME)
            message(STATUS "  Setting RPATH on ${LIB_NAME}")
            
            execute_process(
                COMMAND ${CHRPATH_PROGRAM} -r '$ORIGIN' ${LIB_FILE}
                RESULT_VARIABLE RPATH_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )
            
            if(RPATH_RESULT EQUAL 0)
                message(STATUS "    ✓ RPATH set successfully")
            else()
                message(STATUS "    Warning: Failed to set RPATH")
            endif()
        endforeach()
    else()
        message(STATUS "Neither patchelf nor chrpath found")
        message(STATUS "Install patchelf for better library compatibility")
        

        message(STATUS "Creating LD_LIBRARY_PATH wrapper script...")
        set(WRAPPER_SCRIPT "${TARGET_DIR}/run_encore.sh")
        file(WRITE ${WRAPPER_SCRIPT} "#!/bin/bash\n")
        file(APPEND ${WRAPPER_SCRIPT} "export LD_LIBRARY_PATH=\"$(dirname \"$0\"):$LD_LIBRARY_PATH\"\n")
        file(APPEND ${WRAPPER_SCRIPT} "exec \"$(dirname \"$0\")/Encore\" \"$@\"\n")
        
        execute_process(
            COMMAND chmod +x ${WRAPPER_SCRIPT}
            RESULT_VARIABLE CHMOD_RESULT
        )
        
        if(CHMOD_RESULT EQUAL 0)
            message(STATUS "✓ Created wrapper script: run_encore.sh")
            message(STATUS "  Use ./run_encore.sh instead of ./Encore to run the application")
        endif()
    endif()
    
    message(STATUS "Creating symlinks for dependency resolution...")
    set(FFMPEG_LIBS "avcodec" "avformat" "avutil" "swscale" "swresample" "avfilter" "avdevice")
    
    foreach(LIB_BASE ${FFMPEG_LIBS})
        file(GLOB VERSIONED_FILE "${TARGET_DIR}/lib${LIB_BASE}.so.[0-9]*")
        if(VERSIONED_FILE)
            list(GET VERSIONED_FILE 0 VERSION_FILE)
            get_filename_component(VERSION_FILENAME ${VERSION_FILE} NAME)
            
            set(SYMLINK_PATH "${TARGET_DIR}/lib${LIB_BASE}.so")
            
            if(NOT EXISTS ${SYMLINK_PATH})
                message(STATUS "  Creating symlink: lib${LIB_BASE}.so -> ${VERSION_FILENAME}")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${VERSION_FILENAME} lib${LIB_BASE}.so
                    WORKING_DIRECTORY ${TARGET_DIR}
                    RESULT_VARIABLE SYMLINK_RESULT
                )
                if(SYMLINK_RESULT EQUAL 0)
                    message(STATUS "    ✓ Symlink created successfully")
                else()
                    message(STATUS "    ✗ Failed to create symlink")
                endif()
            else()
                message(STATUS "  Base .so already exists: lib${LIB_BASE}.so")
            endif()
        else()
            message(STATUS "  No versioned file found for lib${LIB_BASE}")
        endif()
    endforeach()
    
    message(STATUS "Final files in target directory:")
    file(GLOB TARGET_SO_FILES "${TARGET_DIR}/lib*.so*")
    foreach(FILE ${TARGET_SO_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        if(IS_SYMLINK ${FILE})
            message(STATUS "  - ${FILENAME} (symlink)")
        else()
            message(STATUS "  - ${FILENAME}")
        endif()
    endforeach()
    
    message(STATUS "Copied ${COPIED_COUNT} FFmpeg .so files and created symlinks")
else()
    message(STATUS "FFMPEG_LIB_DIR or TARGET_DIR not set")
endif()