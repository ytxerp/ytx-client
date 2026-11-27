find_package(Qt6 REQUIRED COMPONENTS Test)

# ---------- 核心库：复用主程序源码（排除 main.cc） ----------
set(CORE_SRCS ${SOURCES} ${UIS} ${HEADERS})
list(REMOVE_ITEM CORE_SRCS main.cc)
list(TRANSFORM CORE_SRCS PREPEND "${CMAKE_SOURCE_DIR}/")

# 测试期间不跑 clang-tidy（对本目录下所有目标生效）
set(CMAKE_CXX_CLANG_TIDY "")

qt_add_library(ytx_test_core STATIC ${CORE_SRCS})
set_target_properties(ytx_test_core PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
target_include_directories(ytx_test_core PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_definitions(ytx_test_core PUBLIC
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    APP_VERSION="${PROJECT_VERSION}"
    QT_USE_QSTRINGBUILDER
    QT_USE_FAST_OPERATOR_PLUS
)
target_link_libraries(ytx_test_core PUBLIC
    Qt6::Widgets Qt6::Concurrent Qt6::PrintSupport Qt6::WebSockets
    YXlsx::YXlsx OpenSSL::SSL OpenSSL::Crypto zstd::libzstd_static
)

# ---------- 测试工具库 ----------
qt_add_library(ytx_test_utils STATIC testutils.cc testutils.h)
set_target_properties(ytx_test_utils PROPERTIES AUTOMOC ON)
target_include_directories(ytx_test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ytx_test_utils PUBLIC
    ytx_test_core
    Qt6::Widgets
    Qt6::Test
)

# ---------- 关闭警告（测试代码不关心） ----------
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(ytx_test_core PRIVATE -w)
    target_compile_options(ytx_test_utils PRIVATE -w)
elseif(MSVC)
    target_compile_options(ytx_test_core PRIVATE /W0)
    target_compile_options(ytx_test_utils PRIVATE /W0)
endif()

# ---------- 添加测试的辅助函数 ----------
function(ytx_add_test target_base)
    qt_add_executable(tst_${target_base} tst_${target_base}.cc)
    set_target_properties(tst_${target_base} PROPERTIES AUTOMOC ON)
    target_link_libraries(tst_${target_base} PRIVATE ytx_test_utils Qt6::Test)

    add_test(NAME ${target_base} COMMAND tst_${target_base})

    # 无头环境（CI）使用 offscreen 平台；需要真实窗口时可去掉
    #set_tests_properties(${target_base} PROPERTIES
    #    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    #)
endfunction()

# ---------- 测试列表 ----------
ytx_add_test(double_node)
ytx_add_test(double_entry)