cmake_minimum_required(VERSION 3.24)
project(switch_auto_core)

# Fusion 是 C 语言库，这里保持 CXX 标准的同时，CMake 会自动处理 C 语言编译
set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    # 这一行是全域设置，最为稳妥
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    add_compile_options("/utf-8")
    add_compile_options("/await:strict")
    add_compile_definitions(_WIN32_WINNT=0x0A00 WINVER=0x0A00)
else()
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

if (MSVC)
    add_compile_options("/utf-8")
endif()

include(FetchContent)

FetchContent_Declare(
        sqlite_orm
        GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
        GIT_TAG        v1.9.1
)


FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG        v7.0.2
)


FetchContent_MakeAvailable(sqlite_orm glaze)

set(SIMPLEBLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SIMPLEBLE_TESTS OFF CACHE BOOL "" FORCE)
set(SIMPLEBLE_LIBRARY_TYPE STATIC CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(
        simpleble
        GIT_REPOSITORY https://github.com/OpenBluetoothToolbox/SimpleBLE.git
        GIT_TAG v0.12.1
)

FetchContent_GetProperties(simpleble)
if(NOT simpleble_POPULATED)
    FetchContent_Populate(simpleble)
    # 核心修正：指向包含 CMakeLists.txt 的子目录
    add_subdirectory(${simpleble_SOURCE_DIR}/simpleble ${simpleble_BINARY_DIR})
endif()

# ---------------------------------------------------------
# 1. 查找依赖 (Config 模式)
# ---------------------------------------------------------
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
# SDL3 查找
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

# SQLite3 查找 (兼容 vcpkg)
find_package(unofficial-sqlite3 CONFIG QUIET)
if(NOT unofficial-sqlite3_FOUND)
    find_package(SQLite3 REQUIRED)
endif()

# ---------------------------------------------------------
# 2. 定义 Proto 编译逻辑
# ---------------------------------------------------------
set(GEN_BASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(GEN_CPP_PATH  "${GEN_BASE_PATH}/cpp")
set(GEN_GRPC_PATH "${GEN_BASE_PATH}/grpc")

file(MAKE_DIRECTORY ${GEN_CPP_PATH})
file(MAKE_DIRECTORY ${GEN_GRPC_PATH})

set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# 扫描所有 proto 文件
file(GLOB_RECURSE PROTO_FILES "proto/*.proto")

set(PROTO_GEN_SRCS "")
set(PROTO_GEN_HDRS "")

foreach(FIL ${PROTO_FILES})
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    list(APPEND PROTO_GEN_SRCS "${GEN_CPP_PATH}/${FIL_WE}.pb.cc" "${GEN_GRPC_PATH}/${FIL_WE}.grpc.pb.cc")
    list(APPEND PROTO_GEN_HDRS "${GEN_CPP_PATH}/${FIL_WE}.pb.h" "${GEN_GRPC_PATH}/${FIL_WE}.grpc.pb.h")
endforeach()

add_custom_command(
        OUTPUT ${PROTO_GEN_SRCS} ${PROTO_GEN_HDRS}
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --cpp_out=${GEN_CPP_PATH}
        --grpc_out=${GEN_GRPC_PATH}
        -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
        --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
        ${PROTO_FILES}
        DEPENDS ${PROTO_FILES}
        COMMENT "Generating Proto/gRPC files"
        VERBATIM
)

# ---------------------------------------------------------
# 3. 准备源文件 (自动扫描)
# ---------------------------------------------------------
# [修复] 扫描 Fusion 的 .c 文件 (Fusion 是纯 C 库)
file(GLOB FUSION_SOURCES "Fusion/*.c")

# 扫描其他 C++ 文件
file(GLOB_RECURSE REPO_SOURCES "src/repo/*.cpp" "src/repo/*.h")
file(GLOB_RECURSE CACHE_SOURCES "src/cache/*.cpp" "src/cache/*.h")
file(GLOB_RECURSE EXEC_SOURCES "src/exec/*.cpp" "src/exec/*.h" "src/controller/*.cpp" "src/controller/*.h")
file(GLOB_RECURSE SERVICE_SOURCES "src/service/*.cpp" "src/service/*.h")
file(GLOB_RECURSE LIB_SOURCES "lib/*.cpp" "lib/*.h")

# ---------------------------------------------------------
# 4. 创建可执行文件
# ---------------------------------------------------------
# [注意] 我删除了下面手动列出的具体 .cpp 文件，因为上面的 GLOB_RECURSE 已经包含它们了。
# 如果同时保留，链接器会报错 "multiple definition" 或 CMake 会警告。
set(ALL_SOURCES
        main.cpp
        ${PROTO_GEN_SRCS}
        ${PROTO_GEN_HDRS}
        ${FUSION_SOURCES}   # 添加 Fusion 源码
        ${REPO_SOURCES}
        ${CACHE_SOURCES}
        ${EXEC_SOURCES}
        ${SERVICE_SOURCES}
        ${LIB_SOURCES}
        src/cache/CombinationGraphCache.h
        src/cache/CacheLoader.h
        src/cache/CacheServiceInterface.h
        src/cache/BaseOperateCache.h
        src/repo/base/ButtonBinding.cpp
        src/repo/base/ButtonBinding.h
        src/cache/ButtonBindingCache.h
        src/serial/GraphSerial.h
        lib/transport/ITransport.h
        lib/transport/UsbTransport.h
)

add_executable(switch_auto_core ${ALL_SOURCES})

# ---------------------------------------------------------
# 5. 配置 Include 路径和链接库
# ---------------------------------------------------------
target_include_directories(switch_auto_core PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        src
        lib
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GEN_CPP_PATH}
        ${GEN_GRPC_PATH}
        ${bs_thread_pool_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        ${glaze_SOURCE_DIR}/include
)

target_link_libraries(switch_auto_core PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++_reflection
        gRPC::grpc++
        $<$<TARGET_EXISTS:unofficial::sqlite3::sqlite3>:unofficial::sqlite3::sqlite3>
        $<$<NOT:$<TARGET_EXISTS:unofficial::sqlite3::sqlite3>>:SQLite::SQLite3>
        sqlite_orm::sqlite_orm
        SDL3::SDL3
        simpleble
)

if(APPLE)
    target_link_libraries(switch_auto_core PRIVATE
            "-framework Foundation"
            "-framework CoreBluetooth"
            "-framework IOBluetooth"
            "-framework AppKit"
    )
endif()

set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GEN_BASE_PATH}")

# B. 定义安装目标
install(TARGETS switch_auto_core RUNTIME DESTINATION .)

# C. 核心：自动扫描并拷贝第三方 DLL (仅限 Windows)
if(WIN32)
    install(CODE [[
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES $<TARGET_FILE:switch_auto_core>
            RESOLVED_DEPENDENCIES_VAR _deps
            UNRESOLVED_DEPENDENCIES_VAR _notfound
            # 排除 Windows 系统核心库，只抓取 gRPC, SDL3, SQLite 等第三方库
            PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*" "kernel32.dll" "user32.dll" "gdi32.dll" "msvcrt.dll" "advapi32.dll"
        )
        foreach(_file ${_deps})
            file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE SHARED_LIBRARY FILES "${_file}")
        endforeach()

        if(_notfound)
            message(WARNING "警告：部分依赖未找到，可能需要手动拷贝: ${_notfound}")
        endif()
    ]])
endif()

# D. 配置 CPack 生成 ZIP 压缩包
set(CPACK_PACKAGE_NAME "switch_auto_core")
set(CPACK_GENERATOR "ZIP") # 生成 zip 即为绿色版
set(CPACK_STRIP_FILES TRUE) # 移除调试符号减小体积
include(CPack)