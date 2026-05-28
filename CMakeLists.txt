cmake_minimum_required(VERSION 3.16)
project(ClanDestiny LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Raylib Dependency (Prebuilt - everything is in ThirdParty/raylib)
# Follows the exact pattern used in repos/U7Revisited/CMakeLists.txt
# ============================================================================
set(RAYLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/include)
set(RAYLIB_EXTERNAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/external/include)

# Platform-specific library selection (everything lives under ThirdParty/raylib)
if(WIN32)
    set(RAYLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib/raylib.lib)
else()
    # Prefer static, fall back to shared (common on Linux distros / custom builds)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib/libraylib.a)
        set(RAYLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib/libraylib.a)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib/libraylib.so)
        set(RAYLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib/libraylib.so)
    else()
        set(RAYLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib/libraylib.a)
    endif()
endif()

# Create imported target for the prebuilt raylib (type depends on what we found)
if(RAYLIB_LIBRARY MATCHES "\\.(so|dylib|dll)$")
    add_library(raylib SHARED IMPORTED)
else()
    add_library(raylib STATIC IMPORTED)
endif()

set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION ${RAYLIB_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR};${RAYLIB_EXTERNAL_INCLUDE}"
)

# nlohmann/json is header-only and currently unused by the code,
# but the include path is kept for future-proofing / parity with the VS project.
set(NLOHMANN_JSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/nlohmann")

# -----------------------------------------------------------------------------
# Executable
# -----------------------------------------------------------------------------

add_executable(ClanDestiny
    Source/main.cpp
    Source/Clan.cpp
    Source/Map.cpp
    Source/Render.cpp
)

target_include_directories(ClanDestiny PRIVATE
    "${CMAKE_SOURCE_DIR}/Source"
    "${NLOHMANN_JSON_INCLUDE_DIR}"
)

target_link_libraries(ClanDestiny PRIVATE raylib)

# Windows system libraries required by the prebuilt raylib (matching original .vcxproj)
if(WIN32)
    target_link_libraries(ClanDestiny PRIVATE winmm opengl32 gdi32 user32 shell32)
endif()

# -----------------------------------------------------------------------------
# Assets
# Copy the asset folders next to the built executable so relative paths work:
#   Data/Fonts/softsquare.ttf
#   Images/tiles.png
# -----------------------------------------------------------------------------

set(ASSET_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Redist")
set(ASSET_DEST_DIR "$<TARGET_FILE_DIR:ClanDestiny>")

add_custom_command(TARGET ClanDestiny POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSET_SOURCE_DIR}/Data"   "${ASSET_DEST_DIR}/Data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSET_SOURCE_DIR}/Images" "${ASSET_DEST_DIR}/Images"
    COMMENT "Copying game assets (Data/ and Images/) next to the executable"
    VERBATIM
)

# Put the final binary in a predictable location (build/bin/)
set_target_properties(ClanDestiny PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# -----------------------------------------------------------------------------
# Compiler warnings (match spirit of the original Visual Studio project)
# -----------------------------------------------------------------------------
if(MSVC)
    target_compile_options(ClanDestiny PRIVATE /W3)
else()
    target_compile_options(ClanDestiny PRIVATE -Wall -Wextra)
endif()

# -----------------------------------------------------------------------------
# Usage notes (printed at configure time)
# -----------------------------------------------------------------------------
message(STATUS "ClanDestiny: To build, run:")
message(STATUS "  mkdir -p build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  cmake --build . --config Release")
message(STATUS "")
message(STATUS "Then run the game from the bin directory (assets are copied there):")
message(STATUS "  cd bin && ./ClanDestiny")
message(STATUS "")
message(STATUS "Using vendored raylib from ThirdParty/raylib (no git fetch, no system package required).")