# FFmpeg decodes the game's music (ATRAC3, ATRAC3plus) and its movies (H.264).
# Only libavcodec and libavutil are used, always as shared libraries.
#
# MHFU_FFMPEG selects where they come from:
#   bundled  (default) On macOS and Linux, download the pinned FFmpeg release
#            below, check its SHA-256 and build it at configure time with the
#            LGPL-only configuration below. On Windows, whose documented MSVC
#            toolchain cannot run FFmpeg's configure, download the pinned
#            prebuilt LGPL shared build below instead. Either way the libraries
#            end up next to the executable, so it needs no FFmpeg on the system.
#   system   The libavcodec and libavutil that pkg-config finds.
#   OFF      No FFmpeg: the game has no music and skips its movies.
#
# Sets MHFU_HAS_FFMPEG and defines mhfu_use_ffmpeg(<target>).

# FFmpeg 7.1.5 (libavcodec 61, libavutil 59), the release the Linux packages use.
set(MHFU_FFMPEG_VERSION 7.1.5)
set(MHFU_FFMPEG_URL "https://ffmpeg.org/releases/ffmpeg-${MHFU_FFMPEG_VERSION}.tar.xz")
set(MHFU_FFMPEG_SHA256 de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f)
set(MHFU_FFMPEG_SOVERSIONS avcodec 61 avutil 59)

# Only what the game needs: the ATRAC3, ATRAC3plus and H.264 decoders. The host
# splits the movie streams into access units and converts pixels itself, so no
# demuxer, parser, scaler or resampler is built. No GPL or non-free parts: the
# result is LGPL-2.1-or-later, which the build checks.
set(MHFU_FFMPEG_CONFIGURE_FLAGS
    --enable-shared
    --disable-static
    --disable-programs
    --disable-doc
    --disable-avdevice
    --disable-avformat
    --disable-avfilter
    --disable-swscale
    --disable-swresample
    --disable-network
    --disable-autodetect
    --disable-everything
    --enable-decoder=atrac3,atrac3p,h264
    --disable-x86asm
    --disable-debug)

# Windows x64: BtbN/FFmpeg-Builds, the LGPL shared build of the 7.1.5 release
# branch from the monthly build of June 2026, which that project keeps for two
# years. It is built with --enable-version3 and without --enable-gpl or
# --enable-nonfree, so it is LGPL-3.0-or-later. It depends on libswresample.
set(MHFU_FFMPEG_WINDOWS_URL
    "https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2026-06-30-13-34/ffmpeg-n7.1.5-1-g7d0e842004-win64-lgpl-shared-7.1.zip")
set(MHFU_FFMPEG_WINDOWS_SHA256 03a8003e245c08df4277d7b0adc50b93a97ddd4a3aaafea21943c4384df59895)
set(MHFU_FFMPEG_WINDOWS_DLLS avcodec-61 avutil-59 swresample-5)

set(MHFU_FFMPEG "bundled" CACHE STRING
    "Where FFmpeg comes from: bundled (built with the project), system (pkg-config) or OFF (no music, no movies)")
set_property(CACHE MHFU_FFMPEG PROPERTY STRINGS bundled system OFF)
# MHFU_FFMPEG used to be ON/OFF: ON (the old default) becomes bundled.
string(TOUPPER "${MHFU_FFMPEG}" _mhfu_ffmpeg_mode)
if(_mhfu_ffmpeg_mode MATCHES "^(ON|TRUE|YES|Y|1|BUNDLED)$")
    set(_mhfu_ffmpeg_mode bundled)
elseif(_mhfu_ffmpeg_mode MATCHES "^(OFF|FALSE|NO|N|0)$")
    set(_mhfu_ffmpeg_mode OFF)
elseif(_mhfu_ffmpeg_mode STREQUAL "SYSTEM")
    set(_mhfu_ffmpeg_mode system)
else()
    message(FATAL_ERROR "MHFU_FFMPEG must be bundled, system or OFF, not '${MHFU_FFMPEG}'")
endif()
if(NOT MHFU_FFMPEG STREQUAL _mhfu_ffmpeg_mode)
    set_property(CACHE MHFU_FFMPEG PROPERTY TYPE STRING)
    set_property(CACHE MHFU_FFMPEG PROPERTY VALUE "${_mhfu_ffmpeg_mode}")
endif()

set(MHFU_FFMPEG_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/_deps/downloads" CACHE PATH
    "Where the bundled FFmpeg archive is downloaded to, or found without downloading")

set(MHFU_HAS_FFMPEG OFF)
set(_mhfu_ffmpeg_root "${CMAKE_BINARY_DIR}/_deps/ffmpeg")
set(_mhfu_ffmpeg_bin "${CMAKE_BINARY_DIR}/bin")

# Downloads <url> to <file name> in MHFU_FFMPEG_DOWNLOAD_DIR unless a copy
# with the pinned SHA-256 is already there; sets <out> to its path.
function(_mhfu_ffmpeg_fetch out name url sha256)
    set(archive "${MHFU_FFMPEG_DOWNLOAD_DIR}/${name}")
    if(EXISTS "${archive}")
        file(SHA256 "${archive}" existing)
        if(existing STREQUAL sha256)
            set(${out} "${archive}" PARENT_SCOPE)
            return()
        endif()
    endif()
    message(STATUS "mhfu: downloading ${url}")
    file(DOWNLOAD "${url}" "${archive}.part" STATUS status TLS_VERIFY ON)
    list(GET status 0 code)
    if(NOT code EQUAL 0)
        file(REMOVE "${archive}.part")
        message(FATAL_ERROR "mhfu: could not download ${url}: ${status}\n"
            "Put the file in ${MHFU_FFMPEG_DOWNLOAD_DIR} by hand, or configure with "
            "-DMHFU_FFMPEG=system to use an installed FFmpeg.")
    endif()
    file(SHA256 "${archive}.part" actual)
    if(NOT actual STREQUAL sha256)
        file(REMOVE "${archive}.part")
        message(FATAL_ERROR "mhfu: ${url} has SHA-256 ${actual}, expected ${sha256}")
    endif()
    file(RENAME "${archive}.part" "${archive}")
    set(${out} "${archive}" PARENT_SCOPE)
endfunction()

# Runs a step of the FFmpeg build with its output in <log>; stops with the end
# of the log on failure.
function(_mhfu_ffmpeg_run what log)
    execute_process(COMMAND ${ARGN}
        WORKING_DIRECTORY "${_mhfu_ffmpeg_root}/build"
        OUTPUT_FILE "${log}" ERROR_FILE "${log}.err"
        RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        file(READ "${log}.err" errors)
        message(FATAL_ERROR "mhfu: FFmpeg ${what} failed (${result}); see ${log} and ${log}.err\n${errors}")
    endif()
endfunction()

# A short note next to the libraries: what they are and where their source is.
function(_mhfu_ffmpeg_write_notice dir)
    string(CONCAT text ${ARGN})
    file(WRITE "${dir}/FFmpeg-SOURCE.txt" "${text}")
endfunction()

if(_mhfu_ffmpeg_mode STREQUAL "OFF")
    message(WARNING
        "MHFU_FFMPEG=OFF: the game will have NO MUSIC and will SKIP ITS MOVIES. "
        "Configure with -DMHFU_FFMPEG=bundled (the default) to build FFmpeg with the project.")

elseif(_mhfu_ffmpeg_mode STREQUAL "system")
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(MHFU_LIBAV QUIET IMPORTED_TARGET libavcodec libavutil)
    endif()
    if(NOT MHFU_LIBAV_FOUND)
        message(FATAL_ERROR
            "MHFU_FFMPEG=system, but pkg-config finds no libavcodec and libavutil. Install "
            "FFmpeg's development files, or configure with -DMHFU_FFMPEG=bundled (the default).")
    endif()
    add_library(mhfu_ffmpeg INTERFACE)
    target_link_libraries(mhfu_ffmpeg INTERFACE PkgConfig::MHFU_LIBAV)
    set(MHFU_HAS_FFMPEG ON)
    message(STATUS "mhfu: FFmpeg libavcodec ${MHFU_LIBAV_libavcodec_VERSION} from the system; music and movies enabled")

elseif(WIN32)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8 OR CMAKE_SYSTEM_PROCESSOR MATCHES "ARM|arm|aarch")
        message(FATAL_ERROR "mhfu: the bundled FFmpeg for Windows is x64 only. "
            "Configure with -DMHFU_FFMPEG=system and an FFmpeg of your own.")
    endif()
    string(REGEX MATCH "[^/]+\\.zip$" _mhfu_ffmpeg_zip "${MHFU_FFMPEG_WINDOWS_URL}")
    string(REGEX REPLACE "\\.zip$" "" _mhfu_ffmpeg_name "${_mhfu_ffmpeg_zip}")
    set(_mhfu_ffmpeg_prefix "${_mhfu_ffmpeg_root}/${_mhfu_ffmpeg_name}")
    if(NOT EXISTS "${_mhfu_ffmpeg_prefix}/pokee.stamp")
        _mhfu_ffmpeg_fetch(_mhfu_ffmpeg_archive "${_mhfu_ffmpeg_zip}"
            "${MHFU_FFMPEG_WINDOWS_URL}" "${MHFU_FFMPEG_WINDOWS_SHA256}")
        file(REMOVE_RECURSE "${_mhfu_ffmpeg_prefix}")
        file(ARCHIVE_EXTRACT INPUT "${_mhfu_ffmpeg_archive}" DESTINATION "${_mhfu_ffmpeg_root}")
        file(WRITE "${_mhfu_ffmpeg_prefix}/pokee.stamp" "${MHFU_FFMPEG_WINDOWS_SHA256}\n")
    endif()
    set(_mhfu_ffmpeg_includes "${_mhfu_ffmpeg_prefix}/include")
    foreach(dll IN LISTS MHFU_FFMPEG_WINDOWS_DLLS)
        configure_file("${_mhfu_ffmpeg_prefix}/bin/${dll}.dll" "${_mhfu_ffmpeg_bin}/${dll}.dll" COPYONLY)
    endforeach()
    configure_file("${_mhfu_ffmpeg_prefix}/LICENSE.txt" "${_mhfu_ffmpeg_bin}/FFmpeg-LICENSE.txt" COPYONLY)
    _mhfu_ffmpeg_write_notice("${_mhfu_ffmpeg_bin}"
        "The FFmpeg libraries here (avcodec, avutil, swresample) are an unmodified prebuilt\n"
        "LGPL shared build of FFmpeg ${MHFU_FFMPEG_VERSION} from BtbN/FFmpeg-Builds, licensed under the\n"
        "GNU LGPL version 3 or later (FFmpeg-LICENSE.txt). They are dynamically linked.\n\n"
        "Build:  ${MHFU_FFMPEG_WINDOWS_URL}\n"
        "Source: https://github.com/FFmpeg/FFmpeg/tree/release/7.1 (the commit is in the file name)\n"
        "Build scripts: https://github.com/BtbN/FFmpeg-Builds\n")
    add_library(mhfu_ffmpeg INTERFACE)
    foreach(lib avcodec avutil)
        add_library(mhfu_ffmpeg_${lib} SHARED IMPORTED)
        foreach(dll IN LISTS MHFU_FFMPEG_WINDOWS_DLLS)
            if(dll MATCHES "^${lib}-")
                set(_mhfu_ffmpeg_dll "${dll}")
            endif()
        endforeach()
        set_target_properties(mhfu_ffmpeg_${lib} PROPERTIES
            IMPORTED_LOCATION "${_mhfu_ffmpeg_bin}/${_mhfu_ffmpeg_dll}.dll"
            IMPORTED_IMPLIB "${_mhfu_ffmpeg_prefix}/lib/${lib}.lib")
        target_link_libraries(mhfu_ffmpeg INTERFACE mhfu_ffmpeg_${lib})
    endforeach()
    target_include_directories(mhfu_ffmpeg INTERFACE "${_mhfu_ffmpeg_includes}")
    set(MHFU_HAS_FFMPEG ON)
    message(STATUS "mhfu: bundled FFmpeg ${MHFU_FFMPEG_VERSION} (prebuilt, LGPL); music and movies enabled")

else()
    find_program(MHFU_MAKE NAMES gmake make)
    if(NOT MHFU_MAKE)
        message(FATAL_ERROR "mhfu: building the bundled FFmpeg needs make. Install it, or "
            "configure with -DMHFU_FFMPEG=system to use an installed FFmpeg.")
    endif()
    set(_mhfu_ffmpeg_src "${_mhfu_ffmpeg_root}/ffmpeg-${MHFU_FFMPEG_VERSION}")
    set(_mhfu_ffmpeg_prefix "${_mhfu_ffmpeg_root}/install")
    # FFmpeg keeps its configure line, prefix included, as a string in the
    # libraries. It is configured for a neutral prefix and installed through
    # DESTDIR, so no path of the build machine (a home directory, a user name)
    # ends up in a shipped library.
    set(_mhfu_ffmpeg_neutral_prefix /ffmpeg)
    # Platform settings that do not change what is built: where macOS finds the
    # libraries (the executable's rpath) and the compiler to use.
    set(_mhfu_ffmpeg_flags ${MHFU_FFMPEG_CONFIGURE_FLAGS})
    if(APPLE)
        list(APPEND _mhfu_ffmpeg_flags --install-name-dir=@rpath)
        if(CMAKE_OSX_DEPLOYMENT_TARGET)
            list(APPEND _mhfu_ffmpeg_flags
                "--extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}"
                "--extra-ldflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        endif()
    endif()
    if(ANDROID)
        # Cross-compiled with the NDK's clang for the ABI and API level of the
        # rest of the build. FFmpeg's android target names the libraries
        # libavcodec.so and libavutil.so, without a version, which is also the
        # only form an APK's lib/ directory accepts.
        if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
            message(FATAL_ERROR "mhfu: the bundled FFmpeg is set up for arm64-v8a only, not ${ANDROID_ABI}")
        endif()
        set(_mhfu_ndk_bin "${ANDROID_TOOLCHAIN_ROOT}/bin")
        list(APPEND _mhfu_ffmpeg_flags
            --enable-cross-compile --target-os=android --arch=aarch64
            "--cc=${_mhfu_ndk_bin}/aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang"
            "--cxx=${_mhfu_ndk_bin}/aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang++"
            "--ar=${_mhfu_ndk_bin}/llvm-ar" "--nm=${_mhfu_ndk_bin}/llvm-nm"
            "--ranlib=${_mhfu_ndk_bin}/llvm-ranlib" "--strip=${_mhfu_ndk_bin}/llvm-strip"
            # Pages may be 16 KiB on current devices; the NDK's own flag for it.
            "--extra-ldflags=-Wl,-z,max-page-size=16384")
    elseif(DEFINED ENV{CC})
        list(APPEND _mhfu_ffmpeg_flags "--cc=$ENV{CC}")
    endif()
    # Built once per build directory; again only when the pin or the flags change.
    string(REPLACE ";" " " _mhfu_ffmpeg_stamp
        "${MHFU_FFMPEG_VERSION} ${MHFU_FFMPEG_SHA256} --prefix=${_mhfu_ffmpeg_neutral_prefix} ${_mhfu_ffmpeg_flags}")
    set(_mhfu_ffmpeg_stamp_file "${_mhfu_ffmpeg_prefix}/pokee.stamp")
    set(_mhfu_ffmpeg_built "")
    if(EXISTS "${_mhfu_ffmpeg_stamp_file}")
        file(READ "${_mhfu_ffmpeg_stamp_file}" _mhfu_ffmpeg_built)
    endif()
    if(NOT _mhfu_ffmpeg_built STREQUAL _mhfu_ffmpeg_stamp)
        _mhfu_ffmpeg_fetch(_mhfu_ffmpeg_archive "ffmpeg-${MHFU_FFMPEG_VERSION}.tar.xz"
            "${MHFU_FFMPEG_URL}" "${MHFU_FFMPEG_SHA256}")
        message(STATUS "mhfu: building FFmpeg ${MHFU_FFMPEG_VERSION} "
            "(${PSPRECOMP_GENERATED_JOBS} jobs, once per build directory)")
        file(REMOVE_RECURSE "${_mhfu_ffmpeg_src}" "${_mhfu_ffmpeg_root}/build" "${_mhfu_ffmpeg_prefix}")
        file(ARCHIVE_EXTRACT INPUT "${_mhfu_ffmpeg_archive}" DESTINATION "${_mhfu_ffmpeg_root}")
        file(MAKE_DIRECTORY "${_mhfu_ffmpeg_root}/build")
        set(_log "${_mhfu_ffmpeg_root}/build")
        _mhfu_ffmpeg_run(configure "${_log}/configure.log"
            sh "${_mhfu_ffmpeg_src}/configure" "--prefix=${_mhfu_ffmpeg_neutral_prefix}" ${_mhfu_ffmpeg_flags})
        # The licensing in docs/SOURCE_PROVENANCE.md assumes exactly this.
        file(READ "${_log}/configure.log" _mhfu_ffmpeg_configure)
        file(READ "${_log}/config.h" _mhfu_ffmpeg_config)
        if(NOT _mhfu_ffmpeg_configure MATCHES "(^|\n)License: LGPL version 2\\.1 or later\n"
           OR NOT _mhfu_ffmpeg_config MATCHES "\n#define CONFIG_GPL 0\n"
           OR NOT _mhfu_ffmpeg_config MATCHES "\n#define CONFIG_NONFREE 0\n")
            message(FATAL_ERROR "mhfu: the FFmpeg configuration is not LGPL-2.1-or-later only; "
                "see ${_log}/configure.log")
        endif()
        _mhfu_ffmpeg_run(build "${_log}/build.log" "${MHFU_MAKE}" -j${PSPRECOMP_GENERATED_JOBS})
        file(REMOVE_RECURSE "${_mhfu_ffmpeg_root}/destdir")
        _mhfu_ffmpeg_run(install "${_log}/install.log"
            "${MHFU_MAKE}" install "DESTDIR=${_mhfu_ffmpeg_root}/destdir")
        file(RENAME "${_mhfu_ffmpeg_root}/destdir${_mhfu_ffmpeg_neutral_prefix}" "${_mhfu_ffmpeg_prefix}")
        file(REMOVE_RECURSE "${_mhfu_ffmpeg_root}/destdir")
        file(WRITE "${_mhfu_ffmpeg_stamp_file}" "${_mhfu_ffmpeg_stamp}")
    endif()

    # The libraries go to lib/ next to the executable, under the names the
    # loader looks for; the executable's rpath points there.
    set(_mhfu_ffmpeg_lib "${_mhfu_ffmpeg_bin}/lib")
    add_library(mhfu_ffmpeg INTERFACE)
    set(_versions ${MHFU_FFMPEG_SOVERSIONS})
    while(_versions)
        list(POP_FRONT _versions lib soversion)
        if(APPLE)
            set(runtime_name "lib${lib}.${soversion}.dylib")
        elseif(ANDROID)
            set(runtime_name "lib${lib}.so")
        else()
            set(runtime_name "lib${lib}.so.${soversion}")
        endif()
        configure_file("${_mhfu_ffmpeg_prefix}/lib/${runtime_name}" "${_mhfu_ffmpeg_lib}/${runtime_name}" COPYONLY)
        add_library(mhfu_ffmpeg_${lib} SHARED IMPORTED)
        set_target_properties(mhfu_ffmpeg_${lib} PROPERTIES
            IMPORTED_LOCATION "${_mhfu_ffmpeg_lib}/${runtime_name}")
        if(NOT APPLE)
            set_target_properties(mhfu_ffmpeg_${lib} PROPERTIES IMPORTED_SONAME "${runtime_name}")
        endif()
        target_link_libraries(mhfu_ffmpeg INTERFACE mhfu_ffmpeg_${lib})
    endwhile()
    target_include_directories(mhfu_ffmpeg INTERFACE "${_mhfu_ffmpeg_prefix}/include")
    configure_file("${_mhfu_ffmpeg_src}/COPYING.LGPLv2.1" "${_mhfu_ffmpeg_lib}/FFmpeg-COPYING.LGPLv2.1.txt" COPYONLY)
    string(REPLACE ";" " " _mhfu_ffmpeg_flag_text "${MHFU_FFMPEG_CONFIGURE_FLAGS}")
    _mhfu_ffmpeg_write_notice("${_mhfu_ffmpeg_lib}"
        "The FFmpeg libraries here (libavcodec, libavutil) are FFmpeg ${MHFU_FFMPEG_VERSION},\n"
        "unmodified, licensed under the GNU LGPL version 2.1 or later\n"
        "(FFmpeg-COPYING.LGPLv2.1.txt). They are dynamically linked.\n\n"
        "Source: ${MHFU_FFMPEG_URL}\n"
        "SHA-256: ${MHFU_FFMPEG_SHA256}\n"
        "Configured with: ${_mhfu_ffmpeg_flag_text}\n")
    set(MHFU_FFMPEG_RPATH "$ORIGIN/lib")
    if(APPLE)
        set(MHFU_FFMPEG_RPATH "@loader_path/lib")
    endif()
    set(MHFU_HAS_FFMPEG ON)
    message(STATUS "mhfu: bundled FFmpeg ${MHFU_FFMPEG_VERSION} (LGPL, atrac3 atrac3p h264); music and movies enabled")
endif()

# Links <target> against FFmpeg and lets it find the bundled libraries.
function(mhfu_use_ffmpeg target)
    if(NOT MHFU_HAS_FFMPEG)
        return()
    endif()
    target_compile_definitions(${target} PRIVATE MHFU_HAS_FFMPEG=1)
    target_link_libraries(${target} PRIVATE mhfu_ffmpeg)
    if(MHFU_FFMPEG_RPATH)
        set_property(TARGET ${target} APPEND PROPERTY BUILD_RPATH "${MHFU_FFMPEG_RPATH}")
        set_property(TARGET ${target} APPEND PROPERTY INSTALL_RPATH "${MHFU_FFMPEG_RPATH}")
    endif()
endfunction()
