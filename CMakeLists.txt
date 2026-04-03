cmake_minimum_required(VERSION 3.30.5)
project(
    YTX
    VERSION 0.3.4
    LANGUAGES CXX
)

# ------------------------
# Qt setup
# ------------------------
set(QT_MODULES
    Widgets
    Gui
    Concurrent
    PrintSupport
    WebSockets
    LinguistTools
)
find_package(Qt6 REQUIRED COMPONENTS ${QT_MODULES})

qt_standard_project_setup(REQUIRES 6.9)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_FORCE_CMP0156_TO_VALUE NEW)

if(CLANG_TIDY_EXE)
    # Will automatically use .clang-tidy config
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
endif()

# Optionally enable debug build and flags set(CMAKE_BUILD_TYPE Debug) set(CMAKE_CXX_FLAGS_DEBUG
# "${CMAKE_CXX_FLAGS_DEBUG} -g")

# How to install OpenSSL for Qt / LLVM-MinGW on Windows
#
# Download and install MSYS2 from: https://www.msys2.org/
#
# Open the MSYS2 shell and update the system: pacman -Syu (Close the shell when prompted, then
# reopen and run again) pacman -Syu
#
# Launch "MSYS2 MinGW64" (important: NOT MSYS, NOT UCRT)
#
# Install OpenSSL for MinGW-w64: pacman -S mingw-w64-x86_64-openssl
#
# After installation, OpenSSL will be located at: D:/msys64/mingw64
#
# Use this path as OPENSSL_ROOT_DIR when building with Qt + LLVM-MinGW

if(APPLE)
    set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3")
    set(zstd_ROOT "/opt/homebrew/opt/zstd")
elseif(WIN32)
    set(OPENSSL_ROOT_DIR "D:/msys64/mingw64")
    set(zstd_ROOT "D:/msys64/mingw64")
endif()

find_package(zstd REQUIRED)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message(STATUS "✅ OpenSSL ${OPENSSL_VERSION} found")
    message(STATUS "   Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "   SSL Library: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "   Crypto Library: ${OPENSSL_CRYPTO_LIBRARY}")
endif()

# Include directories & external libs
add_subdirectory(external/yxlsx/)

# ------------------------
# Sources & resources
# ------------------------
set(BRC "${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.brc")

file(
    GLOB_RECURSE
    SOURCES
    CONFIGURE_DEPENDS
    "*.cc"
)

file(
    GLOB_RECURSE
    HEADERS
    CONFIGURE_DEPENDS
    "*.h"
)

file(
    GLOB_RECURSE
    UIS
    CONFIGURE_DEPENDS
    "*.ui"
)

list(
    FILTER
    SOURCES
    EXCLUDE
    REGEX
    ".*/external/.*"
)

list(
    FILTER
    HEADERS
    EXCLUDE
    REGEX
    ".*/external/.*"
)

list(
    FILTER
    UIS
    EXCLUDE
    REGEX
    ".*/external/.*"
)

# ------------------------
# Platform-specific setup
# ------------------------
if(APPLE)
    set(ICON_MAC "${CMAKE_CURRENT_SOURCE_DIR}/resource/logo/logo.icns")
    set_source_files_properties(${ICON_MAC} ${BRC} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    qt6_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${ICON_MAC})

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY
    )

    # Bundle properties
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE MACOSX_BUNDLE_INFO_PLIST
                                                      ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
    )

    # Post-build: copy "print" folder into .app bundle's Resources
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/print_template"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/print_template"
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/print_template"
    )

elseif(WIN32)
    set(WINRC "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x86/rc.exe")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.rc" "${CMAKE_CURRENT_BINARY_DIR}/Info.rc" @ONLY
    )

    set(INFO_RC "${CMAKE_CURRENT_BINARY_DIR}/Info.rc")
    qt6_add_executable(${PROJECT_NAME} WIN32 ${INFO_RC})

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${PROJECT_NAME}>/print_template"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/print_template"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/print_template"
    )

    # Linux or other platforms
else()
    qt6_add_executable(${PROJECT_NAME})
endif()

# ------------------------
# Qt Translations
# ------------------------
# Note: - Only one TS file can be listed here, otherwise CMake will not work correctly. - Replace
# YTX_zh_CN.ts with the language you need, for example: YTX_ja.ts   # Japanese YTX_de.ts   # German
#
# Workflow: 1. In Qt Creator: Projects -> Build & Run -> Build -> Build Steps -> Details -> Targets
# Enable "update_translations". 2. Build the project. -> This will extract all translatable text
# into the TS file. 3. Open the TS file in Qt Linguist and add translations. 4. In Qt Linguist,
# choose: File -> Release -> This will generate the corresponding .qm file. 5. Use the .qm file in
# your application (via QTranslator).
qt6_add_translations(${PROJECT_NAME} TS_FILES resource/I18N/ytx_zh_CN.ts)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(
    ${PROJECT_NAME}
    PRIVATE ${SOURCES}
            ${HEADERS}
            ${UIS}
            ${BRC}
)

# ------------------------
# Define compile-time macros
# ------------------------
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE QT_USE_QSTRINGBUILDER
            QT_USE_FAST_OPERATOR_PLUS
            QT_DISABLE_DEPRECATED_BEFORE=0x060900
            PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# ------------------------
# Compiler options
# ------------------------

# GCC/Clang specific warning options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE -Wall
                -Wextra
                -Wconversion
                -Wunused
                -Wunused-function
                -Wunused-parameter
                -Wunused-variable
                -Wunreachable-code
                -Wno-shorten-64-to-32
                -Wswitch
                -Wswitch-enum
    )
endif()

# MSVC specific warning options (optional, to elevate warning level on Windows)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE /W4 # Warning level 4 (equivalent to high warnings like -Wall)
                /wd4244 # Disable double to int conversion warning (corresponding to part of
                        # -Wconversion)
                /wd4267 # Disable size_t to int conversion warning (corresponding to
                        # -Wno-shorten-64-to-32)
    )
endif()

# Link required libraries
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt6::Widgets
            Qt6::Gui
            Qt6::Concurrent
            Qt6::PrintSupport
            Qt6::WebSockets
            YXlsx::YXlsx
            OpenSSL::SSL
            OpenSSL::Crypto
            zstd::libzstd_static
)

# -----------------------------
# Option to enable AddressSanitizer
# -----------------------------
# ENABLE_ASAN: ON  -> Enable ASan for Debug builds OFF -> Disable ASan
option(ENABLE_ASAN "Enable AddressSanitizer" ON)

# -----------------------------
# Configure AddressSanitizer for Debug builds
# -----------------------------
# Notes: 1. We use generator expressions $<$<CONFIG:Debug>:...> so that ASan is only enabled for
# Debug configuration. 2. This works for both single-config generators (Makefile/Ninja) and
# multi-config generators (Xcode, Visual Studio). 3. ASan runtime must be linked; we add
# -fsanitize=address to both compile and link. 4. The -fno-omit-frame-pointer flag ensures proper
# stack traces in ASan reports. 5. -g adds debug symbols; optimization level defaults to Debug (-O0
# or -O1).
if(ENABLE_ASAN AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU"))
    message(STATUS "✅ Enabling AddressSanitizer for Debug targets")

    # Add compile flags for Debug builds
    target_compile_options(
        ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer -g>
    )

    # Add link flags for Debug builds
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
endif()
