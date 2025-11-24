cmake_minimum_required(VERSION 3.30.5)
project(YTX VERSION 0.2.9 LANGUAGES CXX)

# ------------------------
# Qt setup
# ------------------------
set(QT_MODULES Widgets Gui Concurrent PrintSupport WebSockets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS ${QT_MODULES})

qt_standard_project_setup(REQUIRES 6.9)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_FORCE_CMP0156_TO_VALUE NEW)

# Optionally enable debug build and flags
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(CMAKE_BUILD_TYPE MATCHES Debug AND ENABLE_ASAN)
    message(STATUS "Enabling AddressSanitizer for Debug build")
    add_compile_options(-O1 -g -fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# Include directories & external libs
add_subdirectory(external/yxlsx/)

# ------------------------
# Sources & resources
# ------------------------
set(BRC "${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.brc")

# Auto-generated file lists for CMake with python
set(SOURCES
    main.cc
    mainwindow.cc
    tree/node.cc
    tree/itemmodel.cc
    tree/widget/treewidgetp.cc
    tree/widget/treewidgeto.cc
    tree/widget/treewidgetf.cc
    tree/widget/treewidgetit.cc
    tree/model/treemodelf.cc
    tree/model/treemodelt.cc
    tree/model/treemodel.cc
    tree/model/treemodeli.cc
    tree/model/treemodelo.cc
    tree/model/treemodelp.cc
    entryhub/entryhubo.cc
    entryhub/entryhubp.cc
    entryhub/entryhubf.cc
    entryhub/entryhub.cc
    entryhub/entryhubi.cc
    entryhub/entryhubt.cc
    widget/datetimeedit.cc
    widget/datetimeeditautoselect.cc
    component/stringinitializer.cc
    delegate/status.cc
    delegate/plaintext.cc
    delegate/int.cc
    delegate/double.cc
    delegate/boolstring.cc
    delegate/document.cc
    delegate/line.cc
    delegate/styleditemdelegate.cc
    delegate/filterunit.cc
    delegate/readonly/nodenamer.cc
    delegate/readonly/issuedtimer.cc
    delegate/readonly/colorr.cc
    delegate/readonly/doublespinunitr.cc
    delegate/readonly/intstringr.cc
    delegate/readonly/doublespinnonezeror.cc
    delegate/readonly/doublespinr.cc
    delegate/readonly/boolstringr.cc
    delegate/readonly/statusr.cc
    delegate/readonly/nodepathr.cc
    delegate/readonly/sectionr.cc
    delegate/tree/treeissuedtime.cc
    delegate/tree/color.cc
    delegate/tree/finance/financeforeignr.cc
    delegate/search/searchpathtreer.cc
    delegate/search/searchpathtabler.cc
    delegate/table/tableissuedtime.cc
    delegate/table/tablecombofilter.cc
    utils/nodeutils.cc
    utils/mainwindowutils.cc
    global/printhub.cc
    search/entry/searchentrymodelo.cc
    search/entry/searchentrymodel.cc
    search/entry/searchentrymodelp.cc
    search/entry/searchentrymodeli.cc
    search/entry/searchentrymodelt.cc
    search/entry/searchentrymodelf.cc
    search/dialog/searchdialogo.cc
    search/dialog/searchdialogp.cc
    search/dialog/searchdialogf.cc
    search/dialog/searchdialogt.cc
    search/dialog/searchdialogi.cc
    search/dialog/searchdialog.cc
    search/node/searchnodemodeli.cc
    search/node/searchnodemodel.cc
    search/node/searchnodemodelt.cc
    search/node/searchnodemodelf.cc
    search/node/searchnodemodelp.cc
    search/node/searchnodemodelo.cc
    dialog/authdialog.cc
    dialog/editdocument.cc
    dialog/editnodename.cc
    dialog/editnodenameo.cc
    dialog/about.cc
    dialog/preferences.cc
    dialog/removenode/leafremovedialog.cc
    dialog/insertnode/insertnodefinance.cc
    dialog/insertnode/insertnodei.cc
    dialog/insertnode/insertnodep.cc
    dialog/insertnode/insertnodetask.cc
    dialog/insertnode/insertnodebranch.cc
    table/entry.cc
    table/entryshadow.cc
    table/widget/tablewidgetfipt.cc
    table/widget/tablewidgeto.cc
    table/model/tablemodeli.cc
    table/model/tablemodelt.cc
    table/model/tablemodelf.cc
    table/model/tablemodel.cc
    table/model/tablemodelo.cc
    table/model/tablemodelp.cc
    report/settlement.cc
    report/widget/refwidget.cc
    report/widget/statementwidget.cc
    report/widget/settlementwidget.cc
    report/model/statementprimarymodel.cc
    report/model/statementsecondarymodel.cc
    report/model/entryrefmodel.cc
    report/model/settlementmodel.cc
    report/model/settlementprimarymodel.cc
    report/model/statementmodel.cc
    global/logininfo.cc
    global/tablesstation.cc
    global/dailylogger.cc
    websocket/websocket.cc
    websocket/jsongen.cc
)

set(HEADERS
    mainwindow.h
    tree/includemultiplefiltermodel.h
    tree/excludeonefiltermodel.h
    tree/excludemultiplefiltermodel.h
    tree/node.h
    tree/itemmodel.h
    tree/widget/treewidgetp.h
    tree/widget/treewidget.h
    tree/widget/treewidgetf.h
    tree/widget/treewidgeto.h
    tree/widget/treewidgetit.h
    tree/model/treemodel.h
    tree/model/treemodeli.h
    tree/model/treemodelt.h
    tree/model/treemodelf.h
    tree/model/treemodelp.h
    tree/model/treemodelo.h
    entryhub/entryhubo.h
    entryhub/entryhubp.h
    entryhub/entryhubf.h
    entryhub/entryhub.h
    entryhub/entryhubt.h
    entryhub/entryhubi.h
    widget/plaintextedit.h
    widget/spinbox.h
    widget/combobox.h
    widget/doublespinbox.h
    widget/datetimeedit.h
    widget/datetimeeditautoselect.h
    widget/lineedit.h
    component/using.h
    component/signalblocker.h
    component/config.h
    component/stringinitializer.h
    component/constant.h
    component/enumclass.h
    component/sectioncontex.h
    component/info.h
    component/arg/tablemodelarg.h
    component/arg/orderwidgetarg.h
    component/arg/nodeinsertarg.h
    delegate/boolstring.h
    delegate/styleditemdelegate.h
    delegate/plaintext.h
    delegate/line.h
    delegate/int.h
    delegate/status.h
    delegate/double.h
    delegate/document.h
    delegate/filterunit.h
    delegate/readonly/boolstringr.h
    delegate/readonly/doublespinr.h
    delegate/readonly/doublespinunitr.h
    delegate/readonly/doublespinnonezeror.h
    delegate/readonly/statusr.h
    delegate/readonly/issuedtimer.h
    delegate/readonly/colorr.h
    delegate/readonly/intstringr.h
    delegate/readonly/nodenamer.h
    delegate/readonly/nodepathr.h
    delegate/readonly/sectionr.h
    delegate/tree/treeissuedtime.h
    delegate/tree/color.h
    delegate/tree/finance/financeforeignr.h
    delegate/search/searchpathtreer.h
    delegate/search/searchpathtabler.h
    delegate/table/tableissuedtime.h
    delegate/table/tablecombofilter.h
    websocket/jsongen.h
    websocket/websocket.h
    utils/nodeutils.h
    utils/templateutils.h
    utils/entryutils.h
    utils/mainwindowutils.h
    utils/castutils.h
    utils/mainutils.h
    global/printhub.h
    search/entry/searchentrymodel.h
    search/entry/searchentrymodeli.h
    search/entry/searchentrymodelt.h
    search/entry/searchentrymodelf.h
    search/entry/searchentrymodelp.h
    search/entry/searchentrymodelo.h
    search/dialog/searchdialog.h
    search/dialog/searchdialogf.h
    search/dialog/searchdialogt.h
    search/dialog/searchdialogo.h
    search/dialog/searchdialogp.h
    search/dialog/searchdialogi.h
    search/node/searchnodemodeli.h
    search/node/searchnodemodelo.h
    search/node/searchnodemodel.h
    search/node/searchnodemodelp.h
    search/node/searchnodemodelf.h
    search/node/searchnodemodelt.h
    dialog/about.h
    dialog/editnodename.h
    dialog/editnodenameo.h
    dialog/editdocument.h
    dialog/preferences.h
    dialog/authdialog.h
    dialog/removenode/leafremovedialog.h
    dialog/insertnode/insertnodep.h
    dialog/insertnode/insertnodebranch.h
    dialog/insertnode/insertnodei.h
    dialog/insertnode/insertnodetask.h
    dialog/insertnode/insertnodefinance.h
    table/entryshadow.h
    table/entry.h
    table/widget/tablewidgetfipt.h
    table/widget/tablewidgeto.h
    table/widget/tablewidget.h
    table/model/tablemodelp.h
    table/model/tablemodelo.h
    table/model/tablemodelt.h
    table/model/tablemodelf.h
    table/model/tablemodeli.h
    table/model/tablemodel.h
    report/entryref.h
    report/statement.h
    report/settlement.h
    report/widget/settlementwidget.h
    report/widget/reportwidget.h
    report/widget/statementwidget.h
    report/widget/refwidget.h
    report/model/statementprimarymodel.h
    report/model/settlementprimarymodel.h
    report/model/statementsecondarymodel.h
    report/model/settlementmodel.h
    report/model/statementmodel.h
    report/model/entryrefmodel.h
    global/dailylogger.h
    global/entrypool.h
    global/resourcepool.h
    global/entryshadowpool.h
    global/logininfo.h
    global/nodepool.h
    global/concepts.h
    global/tablesstation.h
    global/collator.h
)

set(UIS
    mainwindow.ui
    tree/widget/treewidgetit.ui
    tree/widget/treewidgetf.ui
    tree/widget/treewidgeto.ui
    tree/widget/treewidgetp.ui
    search/dialog/searchdialog.ui
    dialog/editnodename.ui
    dialog/editnodenameo.ui
    dialog/about.ui
    dialog/preferences.ui
    dialog/editdocument.ui
    dialog/authdialog.ui
    dialog/removenode/leafremovedialog.ui
    dialog/insertnode/insertnodebranch.ui
    dialog/insertnode/insertnodep.ui
    dialog/insertnode/insertnodetask.ui
    dialog/insertnode/insertnodei.ui
    dialog/insertnode/insertnodefinance.ui
    table/widget/tablewidgetfipt.ui
    table/widget/tablewidgeto.ui
    report/widget/statementwidget.ui
    report/widget/settlementwidget.ui
    report/widget/refwidget.ui
)

# ------------------------
# Platform-specific setup
# ------------------------
if(APPLE)
    set(ICON_MAC "${CMAKE_CURRENT_SOURCE_DIR}/resource/logo/logo.icns")
    set_source_files_properties(${ICON_MAC} ${BRC} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    qt6_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${ICON_MAC})

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        @ONLY
    )

    # Bundle properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
    )

    # Post-build: copy "print" folder into .app bundle's Resources
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/print_template"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/print_template"
            "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/print_template"
    )

elseif(WIN32)
    set(WINRC "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x86/rc.exe")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.rc"
        "${CMAKE_CURRENT_BINARY_DIR}/Info.rc"
        @ONLY
    )

    set(INFO_RC "${CMAKE_CURRENT_BINARY_DIR}/Info.rc")
    qt6_add_executable(${PROJECT_NAME} WIN32 ${INFO_RC})

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/print_template"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/print_template"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/print_template"
    )

# Linux or other platforms
else()
    qt6_add_executable(${PROJECT_NAME})
endif()

# ------------------------
# Qt Translations
# ------------------------
# Note:
#   - Only one TS file can be listed here, otherwise CMake will not work correctly.
#   - Replace YTX_zh_CN.ts with the language you need, for example:
#       YTX_ja.ts   # Japanese
#       YTX_de.ts   # German
#
# Workflow:
#   1. In Qt Creator:
#        Projects -> Build & Run -> Build -> Build Steps -> Details -> Targets
#        Enable "update_translations".
#   2. Build the project.
#        -> This will extract all translatable text into the TS file.
#   3. Open the TS file in Qt Linguist and add translations.
#   4. In Qt Linguist, choose:
#        File -> Release
#        -> This will generate the corresponding .qm file.
#   5. Use the .qm file in your application (via QTranslator).
qt6_add_translations(${PROJECT_NAME} TS_FILES resource/I18N/ytx_zh_CN.ts)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${PROJECT_NAME} PRIVATE
    ${SOURCES}
    ${HEADERS}
    ${UIS}
    ${BRC}
)

# ------------------------
# Define compile-time macros
# ------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_USE_QSTRINGBUILDER
    QT_USE_FAST_OPERATOR_PLUS
    QT_DISABLE_DEPRECATED_BEFORE=0x060900
)

# ------------------------
# Compiler options
# ------------------------

# GCC/Clang specific warning options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wconversion
        -Wno-shorten-64-to-32
    )
endif()

# MSVC specific warning options (optional, to elevate warning level on Windows)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4  # Warning level 4 (equivalent to high warnings like -Wall)
        /wd4244  # Disable double to int conversion warning (corresponding to part of -Wconversion)
        /wd4267  # Disable size_t to int conversion warning (corresponding to -Wno-shorten-64-to-32)
    )
endif()

# Link required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Concurrent
    Qt6::PrintSupport
    Qt6::WebSockets
    YXlsx::YXlsx
)
