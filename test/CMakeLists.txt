find_package(Qt6 REQUIRED COMPONENTS Test)

set(CORE_SRCS ${SOURCES} ${UIS} ${HEADERS})
list(REMOVE_ITEM CORE_SRCS main.cc)
list(TRANSFORM CORE_SRCS PREPEND "${CMAKE_SOURCE_DIR}/")

# 测试期间不跑 clang-tidy（全局变量，对本目录下所有目标生效）
set(CMAKE_CXX_CLANG_TIDY "")

qt_add_library(ytx_test_core STATIC ${CORE_SRCS})
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

# 关掉所有警告，减少编译输出噪音
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(ytx_test_core PUBLIC -w)
elseif(MSVC)
    target_compile_options(ytx_test_core PUBLIC /W0)
endif()

function(ytx_add_test name)
    qt_add_executable(tst_${name} tst_${name}.cc testutils.cc testutils.h)
    target_link_libraries(tst_${name} PRIVATE ytx_test_core Qt6::Test)
    add_test(NAME ${name} COMMAND tst_${name})
endfunction()

ytx_add_test(finance)
