# Serialize builds of one build directory: install cmake/ninja_locked.py.in as
# CMAKE_MAKE_PROGRAM, so every `cmake --build` waits for a Ninja that is already
# running here instead of racing it (see docs/BUILD_SYSTEM.md).
#
# On Unix the wrapper is a shebang script that flock(2)s and execs Ninja. On
# Windows CMAKE_MAKE_PROGRAM must be a .cmd (or other executable), so this file
# writes ninja-locked.cmd plus ninja-locked.py and points CMake at the .cmd.
# Generators other than Ninja leave CMAKE_MAKE_PROGRAM alone.

option(PSPRECOMP_BUILD_LOCK "Allow only one Ninja at a time per build directory" ON)

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
    return()
endif()

if(CMAKE_HOST_WIN32)
    find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)
    set(_psprecomp_wrapper "${CMAKE_BINARY_DIR}/ninja-locked.cmd")
    set(_psprecomp_wrapper_py "${CMAKE_BINARY_DIR}/ninja-locked.py")
else()
    set(_psprecomp_wrapper "${CMAKE_BINARY_DIR}/ninja-locked")
endif()

# Remember the real Ninja: CMake's own on the first configure, or whatever the
# user points CMAKE_MAKE_PROGRAM at later. Do not treat the wrapper as Ninja.
if(NOT CMAKE_MAKE_PROGRAM STREQUAL _psprecomp_wrapper)
    set(PSPRECOMP_REAL_NINJA "${CMAKE_MAKE_PROGRAM}" CACHE FILEPATH
        "Ninja executable run by the build lock wrapper" FORCE)
endif()

if(PSPRECOMP_BUILD_LOCK)
    if(CMAKE_HOST_WIN32)
        # The .cmd must stay next to the .py: %~dp0ninja-locked.py.
        configure_file("${CMAKE_CURRENT_LIST_DIR}/ninja_locked.py.in"
            "${_psprecomp_wrapper_py}" @ONLY)
        configure_file("${CMAKE_CURRENT_LIST_DIR}/ninja_locked.cmd.in"
            "${_psprecomp_wrapper}" @ONLY)
    else()
        configure_file("${CMAKE_CURRENT_LIST_DIR}/ninja_locked.py.in" "${_psprecomp_wrapper}" @ONLY
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
                             WORLD_READ WORLD_EXECUTE)
    endif()
    set(CMAKE_MAKE_PROGRAM "${_psprecomp_wrapper}" CACHE FILEPATH "" FORCE)
else()
    set(CMAKE_MAKE_PROGRAM "${PSPRECOMP_REAL_NINJA}" CACHE FILEPATH "" FORCE)
endif()

# Ninja 1.13.2 keeps a damaged .ninja_deps record when it recovers, so one
# corruption rebuilds everything on every later build (ninja-build/ninja#2703).
# The wrapper repairs the log before each build; say so once per build tree.
execute_process(COMMAND "${PSPRECOMP_REAL_NINJA}" --version
    OUTPUT_VARIABLE _psprecomp_ninja_version OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(_psprecomp_ninja_version VERSION_EQUAL "1.13.2"
   AND NOT PSPRECOMP_NINJA_WARNED STREQUAL _psprecomp_ninja_version)
    message(WARNING
        "Ninja 1.13.2 does not recover from a damaged .ninja_deps (ninja-build/ninja#2703) "
        "and rebuilds everything on every build once it is damaged. "
        "Use Ninja 1.14 or later, or Ninja built from master. Until then, build only through "
        "`cmake --build`, which repairs the log before each build; "
        "docs/BUILD_SYSTEM.md explains the manual repair.")
    set(PSPRECOMP_NINJA_WARNED "${_psprecomp_ninja_version}" CACHE INTERNAL "")
endif()
