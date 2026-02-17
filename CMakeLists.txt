cmake_minimum_required(VERSION 3.24)
project(switch_auto_core)

# Fusion 是 C 语言库，这里保持 CXX 标准的同时，CMake 会自动处理 C 语言编译
set(CMAKE_CXX_STANDARD 23)

add_definitions(-DDB_FILE_PATH="/Users/churunfa/Desktop/code/switch_auto.db")

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

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


# ---------------------------------------------------------
# 1. 查找依赖 (Config 模式)
# ---------------------------------------------------------
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
# SDL3 查找
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

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
        sqlite3
        sqlite_orm::sqlite_orm
        serialport
        SDL3::SDL3
)

set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GEN_BASE_PATH}")