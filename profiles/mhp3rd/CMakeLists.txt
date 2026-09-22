set(MHP3RD_PROFILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# The AOT corpus is derived from the user's executable and is not committed.
# Until scripts/generate.sh has produced it, link an empty registry instead.
file(GLOB MHP3RD_GENERATED CONFIGURE_DEPENDS "${MHP3RD_PROFILE_DIR}/generated/*.cpp")
if(MHP3RD_GENERATED)
    list(LENGTH MHP3RD_GENERATED MHP3RD_GENERATED_COUNT)
    message(STATUS "mhp3rd: ${MHP3RD_GENERATED_COUNT} generated AOT units")
    if(MSVC)
        if(PSPRECOMP_GENERATED_OPT_LEVEL STREQUAL "0")
            set(MHP3RD_GENERATED_OPTIONS "/Od;/bigobj")
        else()
            set(MHP3RD_GENERATED_OPTIONS "/O${PSPRECOMP_GENERATED_OPT_LEVEL};/Ob0;/bigobj")
        endif()
    else()
        set(MHP3RD_GENERATED_OPTIONS "-O${PSPRECOMP_GENERATED_OPT_LEVEL};-g0")
    endif()
    set_source_files_properties(${MHP3RD_GENERATED} PROPERTIES COMPILE_OPTIONS "${MHP3RD_GENERATED_OPTIONS}")

    # The analog camera wraps the game's camera rotation helper, and main.cpp
    # has to hand it the generated unit that helper was compiled into. Which
    # unit that is depends on how the generator partitions the code, so read it
    # from the corpus instead of naming it: a new partition then finds its own
    # unit, and a corpus without the helper stops the build here rather than
    # letting the camera call the wrong code in game.
    set(MHP3RD_CAMERA_HELPER_UNIT "")
    foreach(unit IN LISTS MHP3RD_GENERATED)
        file(STRINGS "${unit}" hit REGEX "register_function\\(0x08878B70u, &recomp_unit_[0-9]+," LIMIT_COUNT 1)
        if(hit)
            string(REGEX MATCH "recomp_unit_[0-9]+" MHP3RD_CAMERA_HELPER_UNIT "${hit}")
            break()
        endif()
    endforeach()
    if(NOT MHP3RD_CAMERA_HELPER_UNIT)
        message(FATAL_ERROR "mhp3rd: no generated unit registers the camera rotation helper 0x08878B70; "
                            "regenerate the corpus from the supported executable, or update "
                            "host/camera/game_camera.cpp if the helper has moved")
    endif()
    message(STATUS "mhp3rd: camera rotation helper in ${MHP3RD_CAMERA_HELPER_UNIT}")
    # A regenerated corpus with the same file names must be read again.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MHP3RD_GENERATED})
else()
    message(STATUS "mhp3rd: no generated AOT units; run profiles/mhp3rd/scripts/generate.sh")
    set(MHP3RD_GENERATED host/generated_stub.cpp)
endif()

# Renderer: SDL3 for the window and Vulkan for drawing. Both are optional so the
# profile still builds headless on a machine without them.
option(MHP3RD_RENDERER "Build the Vulkan renderer" ON)
set(MHP3RD_RENDERER_SOURCES)
if(MHP3RD_RENDERER)
    find_package(SDL3 QUIET)
    find_package(Vulkan QUIET)
    find_program(MHP3RD_GLSLANG NAMES glslangValidator glslang)
    if(SDL3_FOUND AND Vulkan_FOUND AND MHP3RD_GLSLANG)
        set(MHP3RD_SHADER_INC "${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/ge_shaders.inc")
        add_custom_command(
            OUTPUT "${MHP3RD_SHADER_INC}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated_shaders"
            COMMAND python3 "${MHP3RD_PROFILE_DIR}/tools/embed_shaders.py" "${MHP3RD_GLSLANG}" "${MHP3RD_SHADER_INC}"
                    "kGeVertexShader=${MHP3RD_PROFILE_DIR}/host/gpu/shaders/ge.vert"
                    "kGeFragmentShader=${MHP3RD_PROFILE_DIR}/host/gpu/shaders/ge.frag"
            DEPENDS "${MHP3RD_PROFILE_DIR}/host/gpu/shaders/ge.vert"
                    "${MHP3RD_PROFILE_DIR}/host/gpu/shaders/ge.frag"
                    "${MHP3RD_PROFILE_DIR}/tools/embed_shaders.py"
            COMMENT "Compiling GE shaders to SPIR-V")
        add_custom_target(mhp3rd_shaders DEPENDS "${MHP3RD_SHADER_INC}")
        # Dear ImGui (third_party/imgui, unmodified) draws the port's own
        # interface over the game.
        set(MHP3RD_RENDERER_SOURCES
            host/gpu/vulkan_renderer.cpp
            host/ui/file_browser.cpp
            host/ui/font_menu.cpp
            host/ui/input_script.cpp
            host/ui/layer.cpp
            host/ui/menu.cpp
            host/ui/save_screen.cpp
            host/ui/setup_screens.cpp
            host/ui/text_input.cpp
            host/ui/widgets.cpp
            third_party/imgui/imgui.cpp
            third_party/imgui/imgui_draw.cpp
            third_party/imgui/imgui_tables.cpp
            third_party/imgui/imgui_widgets.cpp
            third_party/imgui/backends/imgui_impl_sdl3.cpp
            third_party/imgui/backends/imgui_impl_vulkan.cpp)
        message(STATUS "mhp3rd: Vulkan renderer enabled")
    else()
        message(STATUS "mhp3rd: renderer disabled (SDL3, Vulkan or glslang not found)")
        set(MHP3RD_RENDERER OFF)
    endif()
endif()

# The build the in-game menu shows: `git describe` of the checkout, refreshed
# on every build but rewritten only when it changes, so only the menu
# recompiles after a commit.
set(MHP3RD_VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated_version/yakumo_version.hpp")
add_custom_target(mhp3rd_version
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DOUTPUT=${MHP3RD_VERSION_HEADER}
            -P "${MHP3RD_PROFILE_DIR}/tools/write_version.cmake"
    BYPRODUCTS "${MHP3RD_VERSION_HEADER}"
    COMMENT "Checking the build version")

# FFmpeg decodes the game's music and its movies; by default it is built with
# the project (see cmake/FFmpeg.cmake).
include("${MHP3RD_PROFILE_DIR}/cmake/FFmpeg.cmake")

# GE parsing, texture decoding and the audio mixer are backend independent and
# always built; only the device the sink opens depends on SDL.
set(MHP3RD_HOST_SOURCES
    host/app_paths.cpp
    host/audio/atrac_decoder.cpp
    host/movie/avc_decoder.cpp
    host/movie/psmf_demuxer.cpp
    host/audio/audio_sink.cpp
    host/audio/sas_core.cpp
    host/camera_probe.cpp
    host/camera/camera_input.cpp
    host/camera/game_aspect.cpp
    host/camera/game_camera.cpp
    host/gpu/ge_state.cpp
    host/gpu/texture_decode.cpp
    host/input/bindings.cpp
    host/perf/frame_stats.cpp
    host/perf/perf_overlay.cpp
    host/overlays.cpp
    host/mhp3rd_profile.cpp
    host/kernel/kernel.cpp
    host/kernel/iso_image.cpp
    host/hle/hle_common.cpp
    host/hle/hle_threadman.cpp
    host/hle/hle_sysmem.cpp
    host/hle/hle_io.cpp
    host/hle/hle_system.cpp
    host/hle/hle_media.cpp
    host/hle/hle_atrac.cpp
    host/hle/hle_mpeg.cpp
    host/fonts/game_font.cpp
    host/hle/hle_font.cpp
    host/hle/hle_utility.cpp
    host/hle/hle_savedata.cpp
    host/hle/hle_adhoc.cpp
    host/adhoc/client.cpp
    host/adhoc/discovery.cpp
    host/adhoc/host.cpp
    host/adhoc/server.cpp
    host/save_data/aes128.cpp
    host/save_data/param_sfo.cpp
    host/save_data/savedata_crypto.cpp
    host/save_data/savedata_store.cpp
    host/save_data/save_transfer.cpp
    host/install/executable_preparation.cpp
    host/install/front_end.cpp
    host/install/installer.cpp
    host/install/installer_dialogs.cpp
    host/install/user_data.cpp
    host/settings/settings.cpp
    third_party/tiny_aes/aes.c
)
# tiny-AES-c is C; the project only enables C++, which compiles it unchanged.
set_source_files_properties(third_party/tiny_aes/aes.c PROPERTIES LANGUAGE CXX)

# The recompiled game code gets a target of its own. It needs only the
# psprecomp headers, so definitions and include paths the host adds to
# Yakumo (a renderer switch, a new library's headers) do not change its
# compile commands and rebuild all of it. Its objects still link straight into
# the executable, which exports their symbols to the overlay libraries.
add_library(mhp3rd_generated OBJECT ${MHP3RD_GENERATED})
target_include_directories(mhp3rd_generated PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${MHP3RD_PROFILE_DIR}/generated")
set_target_properties(mhp3rd_generated PROPERTIES JOB_POOL_COMPILE psprecomp_generated)

add_executable(Yakumo
    host/main.cpp
    ${MHP3RD_HOST_SOURCES}
    ${MHP3RD_RENDERER_SOURCES}
    $<TARGET_OBJECTS:mhp3rd_generated>
)
if(WIN32)
    set(MHP3RD_WINDOWS_ICON "${MHP3RD_PROFILE_DIR}/packaging/windows/emblem.ico")
    configure_file(packaging/windows/Yakumo.rc.in Yakumo.rc @ONLY)
    target_sources(Yakumo PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Yakumo.rc")
endif()
if(MHP3RD_RENDERER)
    add_dependencies(Yakumo mhp3rd_shaders mhp3rd_version)
    # The sink opens its own SDL audio subsystem, so it only needs SDL to exist.
    target_compile_definitions(Yakumo PRIVATE MHP3RD_HAS_RENDERER=1 MHP3RD_HAS_SDL_AUDIO=1 MHP3RD_HAS_SDL=1)
    target_include_directories(Yakumo PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated_shaders"
        "${CMAKE_CURRENT_BINARY_DIR}/generated_version"
        "${MHP3RD_PROFILE_DIR}/third_party/imgui")
    target_link_libraries(Yakumo PRIVATE SDL3::SDL3 Vulkan::Vulkan)
endif()
# The host only: the generated code never sees FFmpeg.
mhp3rd_use_ffmpeg(Yakumo)
target_include_directories(Yakumo PRIVATE
    "${MHP3RD_PROFILE_DIR}/host"
    "${MHP3RD_PROFILE_DIR}/third_party"
    "${MHP3RD_PROFILE_DIR}/generated")
target_link_libraries(Yakumo PRIVATE psprecomp_core ${CMAKE_DL_LIBS})
if(WIN32)
    # The ad hoc client's and server's sockets, and the list of interfaces.
    target_link_libraries(Yakumo PRIVATE ws2_32 iphlpapi)
endif()
find_package(Threads REQUIRED)
target_link_libraries(Yakumo PRIVATE Threads::Threads)
# The overlay libraries resolve psprecomp and runtime symbols against the
# executable that loads them, so it has to export them.
set_target_properties(Yakumo PROPERTIES
    ENABLE_EXPORTS TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
# Only main.cpp needs the camera helper's unit; keeping it off the target keeps
# it off every other unit's command line.
if(MHP3RD_CAMERA_HELPER_UNIT)
    set_property(SOURCE host/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS
        MHP3RD_CAMERA_HELPER_UNIT=${MHP3RD_CAMERA_HELPER_UNIT})
endif()
# The NID table is compiled in, so the program reads nothing from the checkout
# it was built in.
set(MHP3RD_NIDS_CSV "${PROJECT_SOURCE_DIR}/configs/nids.csv")
set(MHP3RD_NIDS_INC "${CMAKE_CURRENT_BINARY_DIR}/generated_nids/nid_table.inc")
add_custom_command(
    OUTPUT "${MHP3RD_NIDS_INC}"
    COMMAND ${CMAKE_COMMAND} -DINPUT=${MHP3RD_NIDS_CSV} -DOUTPUT=${MHP3RD_NIDS_INC}
            -P "${MHP3RD_PROFILE_DIR}/tools/embed_nids.cmake"
    DEPENDS "${MHP3RD_NIDS_CSV}" "${MHP3RD_PROFILE_DIR}/tools/embed_nids.cmake"
    COMMENT "Embedding the NID table")
target_sources(Yakumo PRIVATE "${MHP3RD_NIDS_INC}")
target_include_directories(Yakumo PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated_nids")

# A release build runs from wherever it is unpacked: it has no fallback to
# this checkout's game directory, and on Linux it finds the libraries it ships
# in lib/ next to the executable and nothing else.
option(MHP3RD_RELEASE "Build for distribution: no checkout paths, libraries from lib/ next to the executable" OFF)
if(NOT MHP3RD_RELEASE)
    # The checkout's profiles/mhp3rd/game, a developer fallback after the
    # per-user data directory. Only main.cpp reads it; on the target it would be
    # on every generated unit's command line, so moving the checkout would
    # rebuild all of them and no other checkout could reuse their cache entries.
    set_property(SOURCE host/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS
        MHP3RD_DEFAULT_GAME_DIR="${MHP3RD_PROFILE_DIR}/game")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(Yakumo PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN/lib")
    # The C++ runtime is linked in statically, so a build made against an old
    # glibc runs wherever that glibc does, and its symbols stay private: the
    # executable exports its own symbols to the overlay libraries, and a
    # second, exported libstdc++ would clash with the system's one that the
    # graphics driver loads.
    set(MHP3RD_RELEASE_LINK_OPTIONS -static-libstdc++ "LINKER:--exclude-libs,libstdc++.a:libsupc++.a")
    target_link_options(Yakumo PRIVATE ${MHP3RD_RELEASE_LINK_OPTIONS})
endif()
set_target_properties(Yakumo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    JOB_POOL_COMPILE psprecomp_generated)

# Save-data checks that need no game data: crypto test vectors and round trips.
add_executable(mhp3rd_savedata_tests
    tests/savedata_tests.cpp
    host/save_data/aes128.cpp
    host/save_data/param_sfo.cpp
    host/save_data/savedata_crypto.cpp
    host/save_data/savedata_store.cpp
    host/save_data/save_transfer.cpp
    third_party/tiny_aes/aes.c)
target_include_directories(mhp3rd_savedata_tests PRIVATE
    "${MHP3RD_PROFILE_DIR}/host"
    "${MHP3RD_PROFILE_DIR}/third_party")
set_target_properties(mhp3rd_savedata_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(PSPRECOMP_BUILD_TESTS)
    add_test(NAME mhp3rd_savedata_tests COMMAND mhp3rd_savedata_tests)
endif()

# Camera dispatch, ownership and integration checks without game data.
add_executable(mhp3rd_camera_tests
    tests/analog_camera_tests.cpp
    host/camera/camera_input.cpp
    host/camera/game_camera.cpp
    host/input/bindings.cpp)
target_include_directories(mhp3rd_camera_tests PRIVATE "${MHP3RD_PROFILE_DIR}/host")
target_link_libraries(mhp3rd_camera_tests PRIVATE psprecomp_core)
set_target_properties(mhp3rd_camera_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(PSPRECOMP_BUILD_TESTS)
    add_test(NAME mhp3rd_camera_tests COMMAND mhp3rd_camera_tests)
endif()

# The game's view shape: what is written to guest memory, and what is left.
add_executable(mhp3rd_aspect_tests
    tests/game_aspect_tests.cpp
    host/camera/game_aspect.cpp)
target_include_directories(mhp3rd_aspect_tests PRIVATE "${MHP3RD_PROFILE_DIR}/host")
target_link_libraries(mhp3rd_aspect_tests PRIVATE psprecomp_core)
set_target_properties(mhp3rd_aspect_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(PSPRECOMP_BUILD_TESTS)
    add_test(NAME mhp3rd_aspect_tests COMMAND mhp3rd_aspect_tests)
endif()

# Keyboard and mouse bindings: names, settings.ini spelling, the menu's
# assignment rules and what held keys press. No SDL or game data.
add_executable(mhp3rd_input_tests
    tests/input_tests.cpp
    host/input/bindings.cpp)
target_include_directories(mhp3rd_input_tests PRIVATE "${MHP3RD_PROFILE_DIR}/host")
set_target_properties(mhp3rd_input_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(PSPRECOMP_BUILD_TESTS)
    add_test(NAME mhp3rd_input_tests COMMAND mhp3rd_input_tests)
endif()

# The built-in ad hoc server against the client and a raw protocol peer, and
# local network discovery, all over the loopback address.
add_executable(mhp3rd_adhoc_tests
    tests/adhoc_tests.cpp
    host/adhoc/client.cpp
    host/adhoc/discovery.cpp
    host/adhoc/server.cpp)
target_include_directories(mhp3rd_adhoc_tests PRIVATE "${MHP3RD_PROFILE_DIR}/host")
target_link_libraries(mhp3rd_adhoc_tests PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(mhp3rd_adhoc_tests PRIVATE ws2_32 iphlpapi)
endif()
set_target_properties(mhp3rd_adhoc_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(PSPRECOMP_BUILD_TESTS)
    add_test(NAME mhp3rd_adhoc_tests COMMAND mhp3rd_adhoc_tests)
endif()

# Chained AOT calls nest native frames; give the guest-executing main thread
# the same 64 MiB stack the Windows profiles use.
if(MSVC)
    target_link_options(Yakumo PRIVATE /STACK:67108864)
elseif(APPLE)
    target_link_options(Yakumo PRIVATE "LINKER:-stack_size,0x4000000")
endif()

# Recompiled overlay corpora, produced by tools/add_overlay.py. Each directory
# under overlays/ holds one corpus and becomes one shared module that the host
# loads at run time, so a new overlay never relinks the executable.
set(MHP3RD_OVERLAY_DIR "${CMAKE_BINARY_DIR}/bin/overlays")

function(mhp3rd_add_overlay meta_path)
    get_filename_component(source_dir "${meta_path}" DIRECTORY)
    get_filename_component(MHP3RD_OVERLAY_PREFIX "${source_dir}" NAME)
    file(STRINGS "${meta_path}" meta_lines REGEX "^(base|size|code_size|hash|name)=")
    foreach(line IN LISTS meta_lines)
        string(REGEX MATCH "^([a-z_]+)=(.+)$" matched "${line}")
        if(NOT matched)
            continue()
        endif()
        string(TOUPPER "${CMAKE_MATCH_1}" key)
        set(MHP3RD_OVERLAY_${key} "${CMAKE_MATCH_2}")
    endforeach()
    if(NOT MHP3RD_OVERLAY_BASE OR NOT MHP3RD_OVERLAY_SIZE OR NOT MHP3RD_OVERLAY_CODE_SIZE
       OR NOT MHP3RD_OVERLAY_HASH OR NOT MHP3RD_OVERLAY_NAME)
        message(WARNING "mhp3rd: incomplete overlay metadata in ${meta_path}")
        return()
    endif()

    file(GLOB sources CONFIGURE_DEPENDS "${source_dir}/*.cpp")
    if(NOT sources)
        message(WARNING "mhp3rd: no sources for overlay ${MHP3RD_OVERLAY_PREFIX}")
        return()
    endif()
    set(entry_point "${CMAKE_CURRENT_BINARY_DIR}/overlays/${MHP3RD_OVERLAY_PREFIX}_module.cpp")
    configure_file("${MHP3RD_PROFILE_DIR}/host/overlay_module.cpp.in" "${entry_point}" @ONLY)

    set(target "overlay_${MHP3RD_OVERLAY_PREFIX}")
    add_library(${target} MODULE ${sources} "${entry_point}")
    # Headers only: linking psprecomp_core would give the module its own copy of
    # the runtime state the host already owns.
    target_include_directories(${target} PRIVATE
        "${source_dir}" "${MHP3RD_PROFILE_DIR}/host" "${PROJECT_SOURCE_DIR}/include")
    if(WIN32 OR APPLE)
        # Both linkers want the host binary while linking the module: MSVC for
        # its import library, ld64 as the bundle loader. ELF leaves the host
        # symbols undefined and resolves them when the module is loaded.
        target_link_libraries(${target} PRIVATE Yakumo)
    else()
        add_dependencies(${target} Yakumo)
    endif()
    if(MSVC)
        target_compile_definitions(${target} PRIVATE PSPRECOMP_IMPORT_HOST_SYMBOLS=1)
        if(PSPRECOMP_GENERATED_OPT_LEVEL STREQUAL "0")
            target_compile_options(${target} PRIVATE /Od /bigobj)
        else()
            target_compile_options(${target} PRIVATE /O${PSPRECOMP_GENERATED_OPT_LEVEL} /Ob0 /bigobj)
        endif()
    else()
        target_compile_options(${target} PRIVATE -O${PSPRECOMP_GENERATED_OPT_LEVEL} -g0)
    endif()
    if(MHP3RD_RELEASE_LINK_OPTIONS)
        target_link_options(${target} PRIVATE ${MHP3RD_RELEASE_LINK_OPTIONS})
    endif()
    set_target_properties(${target} PROPERTIES
        JOB_POOL_COMPILE psprecomp_generated
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        PREFIX ""
        SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}"
        OUTPUT_NAME "${MHP3RD_OVERLAY_PREFIX}"
        LIBRARY_OUTPUT_DIRECTORY "${MHP3RD_OVERLAY_DIR}")
endfunction()

file(GLOB MHP3RD_OVERLAY_METADATA CONFIGURE_DEPENDS "${MHP3RD_PROFILE_DIR}/overlays/*/meta.txt")
foreach(meta IN LISTS MHP3RD_OVERLAY_METADATA)
    mhp3rd_add_overlay("${meta}")
endforeach()
list(LENGTH MHP3RD_OVERLAY_METADATA MHP3RD_OVERLAY_COUNT)
message(STATUS "mhp3rd: ${MHP3RD_OVERLAY_COUNT} overlay librar(ies) in ${MHP3RD_OVERLAY_DIR}")
