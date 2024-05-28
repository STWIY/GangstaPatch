cmake_minimum_required(VERSION 3.14)
project(GangstaPatch LANGUAGES CXX)

# Set the Windows target platform version
set(CMAKE_SYSTEM_VERSION 10.0)

# Specify the output directories for the build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Define the target
add_library(GangstaPatch SHARED
    GangstaPatch/Core/Hooks/GameObject/ControllerInput.cc
    GangstaPatch/Core/Hooks/Game/Registry.cc
    GangstaPatch/Core/Hooks/P3D/OceanRenderer.cc
    GangstaPatch/Core/Hooks/P3D/VehicleShader.cc
    GangstaPatch/Core/Hooks/Script/ListScreenResolutionEntries.cc
    GangstaPatch/Core/Patcher.cc
    GangstaPatch/dllmain.cc
)

# Include directories
target_include_directories(GangstaPatch PRIVATE
    ${CMAKE_SOURCE_DIR}/3rdParty
    ${CMAKE_SOURCE_DIR}/3rdParty/CRT-STL
    ${CMAKE_SOURCE_DIR}/3rdParty/CRT-STL/STL
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/..
)

# Preprocessor definitions
target_compile_definitions(GangstaPatch PRIVATE
    WIN32
    NDEBUG
    GANGSTAPATCH_EXPORTS
    _WINDOWS
    _USRDLL
    _WINDLL
    _MBCS
)

string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")


# Set the compiler options based on the compiler (Clang)
target_compile_options(GangstaPatch PRIVATE
    /Zc:threadSafeInit- 
    -Wno-microsoft-cast
    -nostdlib++
    -nostdlib
    /Zi
    /MT
    /W3
    /GR-
    /Gy
    /Oi
    /GF
    /GS-
    /fp:except-
    -fno-exceptions
    -O2
    -Os  
    /O2 
    /fp:precise 
    /WX-
    /Gd
    /nologo
    /Os
    /diagnostics:column
)

# Set the standard to C++17
set_target_properties(GangstaPatch PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Linker options for lld-link
target_link_options(GangstaPatch PRIVATE
    /NOLOGO
    /MANIFEST:NO
    /NXCOMPAT
    /DYNAMICBASE
    /DEBUG:FASTLINK
    /DLL
    /ENTRY:DllMain
    /OPT:REF
    /INCREMENTAL:NO
    /OPT:ICF
    /NODEFAULTLIB
    /SUBSYSTEM:WINDOWS
    /MANIFESTUAC:NO
)

# Specify output paths
set_target_properties(GangstaPatch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Release
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Release
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Release
    PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Release
    OUTPUT_NAME "GangstaPatch"
)

# Additional libraries to link against
target_link_libraries(GangstaPatch PRIVATE
    "${CMAKE_SOURCE_DIR}/GangstaPatch/3rdParty/MinHook.lib"
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    comdlg32.lib
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib
)

# Post-build command to rename the output
add_custom_command(TARGET GangstaPatch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:GangstaPatch> $<TARGET_FILE_DIR:GangstaPatch>/$<TARGET_NAME:GangstaPatch>.asi
)
