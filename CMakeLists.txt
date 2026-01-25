cmake_minimum_required(VERSION 3.24)
project(switch_auto_core)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DDB_FILE_PATH="/Users/churunfa/Desktop/code/switch_auto.db")

include(FetchContent)

FetchContent_Declare(
        sqlite_orm
        GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
        GIT_TAG        v1.9.1
)

FetchContent_MakeAvailable(sqlite_orm)

# ---------------------------------------------------------
# 1. 查找依赖 (Config 模式)
# ---------------------------------------------------------
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# ---------------------------------------------------------
# 2. 定义 Proto 编译逻辑 (最关键的部分)
# ---------------------------------------------------------
# 找到 protoc 编译器和 grpc_cpp_plugin 插件的可执行路径
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# 自动扫描 proto 目录下的所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES "proto/*.proto")

# 创建一个列表来存放即将生成的 .pb.cc 和 .grpc.pb.cc 文件路径
set(PROTO_GEN_SRCS "")
set(PROTO_GEN_HDRS ""
        src/repo/DatabaseManager.h
)

foreach(FIL ${PROTO_FILES})
    # 获取文件名（不带路径和后缀），例如 "bot_core"
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    # 定义生成文件的完整路径 (生成到构建目录 build/ 里面，保持源码目录干净)
    set(_PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    set(_PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")
    set(_GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.cc")
    set(_GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.h")

    # 添加编译命令
    add_custom_command(
            OUTPUT ${_PROTO_SRCS} ${_PROTO_HDRS} ${_GRPC_SRCS} ${_GRPC_HDRS}
            COMMAND ${_PROTOBUF_PROTOC}
            ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
            --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
            -I ${CMAKE_CURRENT_SOURCE_DIR}/proto  # <-- 关键：指定 proto 文件的根目录
            --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
            ${FIL}
            DEPENDS ${FIL}
            COMMENT "Running C++ protocol buffer compiler on ${FIL}"
            VERBATIM
    )

    # 将生成的文件加入列表
    list(APPEND PROTO_GEN_SRCS ${_PROTO_SRCS} ${_GRPC_SRCS})
    list(APPEND PROTO_GEN_HDRS ${_PROTO_HDRS} ${_GRPC_HDRS})
endforeach()

# ---------------------------------------------------------
# 3. 准备源文件
# ---------------------------------------------------------
file(GLOB_RECURSE REPO_SOURCES "src/repo/*.cpp" "src/repo/*.h" "src/repo/base/*.cpp" "src/repo/base/*.h")
file(GLOB_RECURSE REPO_SOURCES "src/repo/combination/*.cpp" "src/repo/combination/*.h")
file(GLOB_RECURSE EXEC_SOURCES "src/exec/base/*.cpp" "src/exec/base/*.h")

# ---------------------------------------------------------
# 4. 创建可执行文件
# ---------------------------------------------------------
set(ALL_SOURCES
        main.cpp
        ${PROTO_GEN_SRCS}
        ${PROTO_GEN_HDRS}
        ${REPO_SOURCES}
        ${EXEC_SOURCES}
        src/exec/base/StickOperate.h
        src/exec/base/ImuOperator.h
        src/exec/base/ResetAllOperate.h
        src/exec/base/BaseOperationProcess.h
        src/repo/combination/combination.h
        src/repo/base/BaseOperate.cpp
        src/repo/combination/CombinationGraph.cpp
        src/repo/combination/CombinationGraph.h
        src/exec/base/EmptyOperate.h
)
add_executable(switch_auto_core ${ALL_SOURCES})

# ---------------------------------------------------------
# 5. 配置 Include 路径和链接库
# ---------------------------------------------------------
target_include_directories(switch_auto_core PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(switch_auto_core PRIVATE src)
target_include_directories(switch_auto_core PRIVATE lib)

target_link_libraries(switch_auto_core PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++_reflection
        gRPC::grpc++
        sqlite3
        sqlite_orm::sqlite_orm
)