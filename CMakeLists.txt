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

# Auto-generated file lists

set(HEADERS
    billing/settlement/settlement.h
    billing/settlement/tablemodelsettlement.h
    billing/settlement/tablewidgetsettlement.h
    billing/settlement/treemodelsettlement.h
    billing/settlement/treewidgetsettlement.h
    billing/statement/statement.h
    billing/statement/statemententrymodel.h
    billing/statement/statemententrywidget.h
    billing/statement/statementmodel.h
    billing/statement/statementnodemodel.h
    billing/statement/statementnodewidget.h
    billing/statement/statementwidget.h
    component/arg/nodeinsertarg.h
    component/arg/orderwidgetarg.h
    component/arg/tablemodelarg.h
    component/config.h
    component/constant.h
    component/constantbool.h
    component/constantdouble.h
    component/constantint.h
    component/constantstring.h
    component/constantwebsocket.h
    component/info.h
    component/sectioncontex.h
    component/signalblocker.h
    component/stringinitializer.h
    component/using.h
    component/widgetcontex.h
    delegate/bool.h
    delegate/boolstring.h
    delegate/color.h
    delegate/document.h
    delegate/double.h
    delegate/filterunit.h
    delegate/int.h
    delegate/issuedtime.h
    delegate/line.h
    delegate/readonly/amountorderreferencer.h
    delegate/readonly/amountr.h
    delegate/readonly/boolcolorstringr.h
    delegate/readonly/boolstringr.h
    delegate/readonly/colorr.h
    delegate/readonly/documentr.h
    delegate/readonly/doublenonezeror.h
    delegate/readonly/doubler.h
    delegate/readonly/financeforeignr.h
    delegate/readonly/intstringr.h
    delegate/readonly/issuedtimer.h
    delegate/readonly/nodenamer.h
    delegate/readonly/nodepathr.h
    delegate/readonly/statusr.h
    delegate/rhsnode.h
    delegate/search/searchpathtabler.h
    delegate/search/searchpathtreer.h
    delegate/statusdelegate.h
    delegate/styleditemdelegate.h
    delegate/tagdelegate.h
    delegate/workspaceroledelegate.h
    dialog/about.h
    dialog/authdialog.h
    dialog/deletenode/exactmatchconfirmdialog.h
    dialog/deletenode/leafdeletedialog.h
    dialog/editdocument.h
    dialog/editnodename.h
    dialog/editnodenameo.h
    dialog/insertnode/insertnodebranch.h
    dialog/insertnode/insertnodefinance.h
    dialog/insertnode/insertnodei.h
    dialog/insertnode/insertnodep.h
    dialog/insertnode/insertnodetask.h
    dialog/preferences.h
    dialog/tagmanagerdlg.h
    dialog/userprofiledialog.h
    entryhub/entryhub.h
    entryhub/entryhubf.h
    entryhub/entryhubi.h
    entryhub/entryhubo.h
    entryhub/entryhubp.h
    entryhub/entryhubt.h
    enum/authenum.h
    enum/entryenum.h
    enum/nodeenum.h
    enum/reference.h
    enum/section.h
    enum/settlementenum.h
    enum/statementenum.h
    enum/statusenum.h
    enum/syncenum.h
    enum/tagenum.h
    global/collator.h
    global/concepts.h
    global/dailylogger.h
    global/entrypool.h
    global/exportexcel.h
    global/logininfo.h
    global/nodepool.h
    global/password_encryption.h
    global/printhub.h
    global/resourcepool.h
    global/tablesstation.h
    global/userprofile.h
    mainwindow/mainwindow.h
    reference/orderreference.h
    reference/orderreferencemodel.h
    reference/orderreferencemodelp.h
    reference/orderreferencewidget.h
    reference/salereferencemodeli.h
    search/dialog/searchdialog.h
    search/dialog/searchdialogf.h
    search/dialog/searchdialogi.h
    search/dialog/searchdialogo.h
    search/dialog/searchdialogp.h
    search/dialog/searchdialogt.h
    search/entry/searchentrymodel.h
    search/entry/searchentrymodelf.h
    search/entry/searchentrymodeli.h
    search/entry/searchentrymodelo.h
    search/entry/searchentrymodelp.h
    search/entry/searchentrymodelt.h
    search/node/searchnodemodel.h
    search/node/searchnodemodelf.h
    search/node/searchnodemodeli.h
    search/node/searchnodemodelo.h
    search/node/searchnodemodelp.h
    search/node/searchnodemodelt.h
    table/entry.h
    table/entryshadow.h
    table/model/tablemodel.h
    table/model/tablemodelf.h
    table/model/tablemodeli.h
    table/model/tablemodelo.h
    table/model/tablemodelp.h
    table/model/tablemodelt.h
    table/widget/tablewidget.h
    table/widget/tablewidgetfipt.h
    table/widget/tablewidgeto.h
    tag/tag.h
    tag/tagicon.h
    tag/tagmodel.h
    tree/excludeidfiltermodel.h
    tree/excludeidunitfiltermodel.h
    tree/includeunitfiltermodel.h
    tree/itemmodel.h
    tree/model/treemodel.h
    tree/model/treemodelf.h
    tree/model/treemodeli.h
    tree/model/treemodelo.h
    tree/model/treemodelp.h
    tree/model/treemodelt.h
    tree/node.h
    tree/replaceselffiltermodel.h
    tree/widget/treewidget.h
    tree/widget/treewidgetf.h
    tree/widget/treewidgetit.h
    tree/widget/treewidgeto.h
    tree/widget/treewidgetp.h
    utils/entryutils.h
    utils/mainutils.h
    utils/mainwindowutils.h
    utils/nodeutils.h
    utils/tagutils.h
    utils/templateutils.h
    websocket/jsongen.h
    websocket/websocket.h
    widget/combobox.h
    widget/datetimeedit.h
    widget/doublespinbox.h
    widget/lineedit.h
    widget/spinbox.h
    widget/treeview.h
    workspace_member/workspacemember.h
    workspace_member/workspacememberdialog.h
    workspace_member/workspacememberenum.h
    workspace_member/workspacemembermodel.h
    workspace_member/workspacerole.h
    delegate/databaseroledelegate.h
    workspace_member/workspaceinfo.h
)

set(SOURCES
    billing/settlement/settlement.cc
    billing/settlement/tablemodelsettlement.cc
    billing/settlement/tablewidgetsettlement.cc
    billing/settlement/treemodelsettlement.cc
    billing/settlement/treewidgetsettlement.cc
    billing/statement/statemententrymodel.cc
    billing/statement/statemententrywidget.cc
    billing/statement/statementmodel.cc
    billing/statement/statementnodemodel.cc
    billing/statement/statementnodewidget.cc
    billing/statement/statementwidget.cc
    component/stringinitializer.cc
    delegate/bool.cc
    delegate/boolstring.cc
    delegate/color.cc
    delegate/document.cc
    delegate/double.cc
    delegate/filterunit.cc
    delegate/int.cc
    delegate/issuedtime.cc
    delegate/line.cc
    delegate/readonly/amountorderreferencer.cc
    delegate/readonly/amountr.cc
    delegate/readonly/boolcolorstringr.cc
    delegate/readonly/boolstringr.cc
    delegate/readonly/colorr.cc
    delegate/readonly/documentr.cc
    delegate/readonly/doublenonezeror.cc
    delegate/readonly/doubler.cc
    delegate/readonly/financeforeignr.cc
    delegate/readonly/intstringr.cc
    delegate/readonly/issuedtimer.cc
    delegate/readonly/nodenamer.cc
    delegate/readonly/nodepathr.cc
    delegate/readonly/statusr.cc
    delegate/rhsnode.cc
    delegate/search/searchpathtabler.cc
    delegate/search/searchpathtreer.cc
    delegate/statusdelegate.cc
    delegate/styleditemdelegate.cc
    delegate/tagdelegate.cc
    delegate/workspaceroledelegate.cc
    delegate/databaseroledelegate.cc
    dialog/about.cc
    dialog/authdialog.cc
    dialog/deletenode/exactmatchconfirmdialog.cc
    dialog/deletenode/leafdeletedialog.cc
    dialog/editdocument.cc
    dialog/editnodename.cc
    dialog/editnodenameo.cc
    dialog/insertnode/insertnodebranch.cc
    dialog/insertnode/insertnodefinance.cc
    dialog/insertnode/insertnodei.cc
    dialog/insertnode/insertnodep.cc
    dialog/insertnode/insertnodetask.cc
    dialog/preferences.cc
    dialog/tagmanagerdlg.cc
    dialog/userprofiledialog.cc
    entryhub/entryhub.cc
    entryhub/entryhubf.cc
    entryhub/entryhubi.cc
    entryhub/entryhubo.cc
    entryhub/entryhubp.cc
    entryhub/entryhubt.cc
    global/dailylogger.cc
    global/exportexcel.cc
    global/logininfo.cc
    global/password_encryption.cc
    global/printhub.cc
    global/tablesstation.cc
    main.cc
    mainwindow/mainwindow.cc
    mainwindow/mainwindow_account.cc
    mainwindow/mainwindow_auth.cc
    mainwindow/mainwindow_charts.cc
    mainwindow/mainwindow_connect.cc
    mainwindow/mainwindow_context.cc
    mainwindow/mainwindow_delegate.cc
    mainwindow/mainwindow_entry.cc
    mainwindow/mainwindow_global.cc
    mainwindow/mainwindow_local.cc
    mainwindow/mainwindow_node.cc
    mainwindow/mainwindow_node_remove.cc
    mainwindow/mainwindow_order.cc
    mainwindow/mainwindow_order_reference.cc
    mainwindow/mainwindow_search.cc
    mainwindow/mainwindow_settlement.cc
    mainwindow/mainwindow_statement.cc
    mainwindow/mainwindow_tab_widget.cc
    mainwindow/mainwindow_tag.cc
    mainwindow/mainwindow_view.cc
    reference/orderreferencemodel.cc
    reference/orderreferencemodelp.cc
    reference/orderreferencewidget.cc
    reference/salereferencemodeli.cc
    search/dialog/searchdialog.cc
    search/dialog/searchdialogf.cc
    search/dialog/searchdialogi.cc
    search/dialog/searchdialogo.cc
    search/dialog/searchdialogp.cc
    search/dialog/searchdialogt.cc
    search/entry/searchentrymodel.cc
    search/entry/searchentrymodelf.cc
    search/entry/searchentrymodeli.cc
    search/entry/searchentrymodelo.cc
    search/entry/searchentrymodelp.cc
    search/entry/searchentrymodelt.cc
    search/node/searchnodemodel.cc
    search/node/searchnodemodelf.cc
    search/node/searchnodemodeli.cc
    search/node/searchnodemodelo.cc
    search/node/searchnodemodelp.cc
    search/node/searchnodemodelt.cc
    table/entry.cc
    table/entryshadow.cc
    table/model/tablemodel.cc
    table/model/tablemodelf.cc
    table/model/tablemodeli.cc
    table/model/tablemodelo.cc
    table/model/tablemodelp.cc
    table/model/tablemodelt.cc
    table/widget/tablewidgetfipt.cc
    table/widget/tablewidgeto.cc
    tag/tagmodel.cc
    tree/itemmodel.cc
    tree/model/treemodel.cc
    tree/model/treemodelf.cc
    tree/model/treemodeli.cc
    tree/model/treemodelo.cc
    tree/model/treemodelp.cc
    tree/model/treemodelt.cc
    tree/node.cc
    tree/widget/treewidgetf.cc
    tree/widget/treewidgetit.cc
    tree/widget/treewidgeto.cc
    tree/widget/treewidgetp.cc
    utils/mainwindowutils.cc
    utils/nodeutils.cc
    utils/tagutils.cc
    websocket/jsongen.cc
    websocket/websocket.cc
    widget/datetimeedit.cc
    widget/treeview.cc
    workspace_member/workspacememberdialog.cc
    workspace_member/workspacemembermodel.cc
)

set(UIS
    billing/settlement/tablewidgetsettlement.ui
    billing/settlement/treewidgetsettlement.ui
    billing/statement/statemententrywidget.ui
    billing/statement/statementnodewidget.ui
    billing/statement/statementwidget.ui
    dialog/about.ui
    dialog/authdialog.ui
    dialog/deletenode/exactmatchconfirmdialog.ui
    dialog/deletenode/leafdeletedialog.ui
    dialog/editdocument.ui
    dialog/editnodename.ui
    dialog/editnodenameo.ui
    dialog/insertnode/insertnodebranch.ui
    dialog/insertnode/insertnodefinance.ui
    dialog/insertnode/insertnodei.ui
    dialog/insertnode/insertnodep.ui
    dialog/insertnode/insertnodetask.ui
    dialog/preferences.ui
    dialog/tagmanagerdlg.ui
    dialog/userprofiledialog.ui
    mainwindow/mainwindow.ui
    reference/orderreferencewidget.ui
    search/dialog/searchdialog.ui
    table/widget/tablewidgetfipt.ui
    table/widget/tablewidgeto.ui
    tree/widget/treewidgetf.ui
    tree/widget/treewidgetit.ui
    tree/widget/treewidgeto.ui
    tree/widget/treewidgetp.ui
    workspace_member/workspacememberdialog.ui
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
