cmake_minimum_required(VERSION 3.24)
project(switch_auto_core)

set(CMAKE_CXX_STANDARD 23)

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
find_package(Boost CONFIG REQUIRED)

# ---------------------------------------------------------
# 2. 定义 Proto 编译逻辑
# ---------------------------------------------------------
# ---------------------------------------------------------
# 2. 全量定义 Proto 编译逻辑 (避免循环调用)
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

# 预先构建输出文件列表，让 CMake 知道要等这些文件生成
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
        ${PROTO_FILES}  # 这里传入整个文件列表
        DEPENDS ${PROTO_FILES}
        COMMENT "Generating ALL Proto/gRPC files at once to ensure table consistency"
        VERBATIM
)

# ---------------------------------------------------------
# 3. 准备源文件
# ---------------------------------------------------------
file(GLOB_RECURSE REPO_SOURCES "src/repo/*.cpp" "src/repo/*.h" "src/repo/base/*.cpp" "src/repo/base/*.h")
file(GLOB_RECURSE REPO_SOURCES "src/repo/combination/*.cpp" "src/repo/combination/*.h")
file(GLOB_RECURSE EXEC_SOURCES "src/exec/base/*.cpp" "src/exec/base/*.h")
file(GLOB_RECURSE EXEC_SOURCES "lib/*.cpp" "lib/*.h")

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
        src/service/BaseOperateService.cpp
        src/service/BaseOperateService.h
        src/service/CombinationGraphService.cpp
        src/service/CombinationGraphService.h
        src/service/mapper/BaseOperateMapper.h
        src/service/mapper/CombinationGraphMapper.h
        src/service/mapper/BaseOperateMapper.cpp
        src/service/mapper/CombinationGraphMapper.cpp
        src/repo/base/IdGenerate.h
        src/repo/base/IdGenerate.cpp
        src/exec/coroutine/CoroutineUtils.h
)
add_executable(switch_auto_core ${ALL_SOURCES})

# ---------------------------------------------------------
# 5. 配置 Include 路径和链接库
# ---------------------------------------------------------
target_include_directories(switch_auto_core PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(switch_auto_core PRIVATE src)
target_include_directories(switch_auto_core PRIVATE lib)
target_include_directories(switch_auto_core PRIVATE ${GEN_CPP_PATH})
target_include_directories(switch_auto_core PRIVATE ${GEN_GRPC_PATH})
target_include_directories(switch_auto_core PRIVATE ${bs_thread_pool_SOURCE_DIR}/include)
target_include_directories(switch_auto_core PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(switch_auto_core PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++_reflection
        gRPC::grpc++
        sqlite3
        sqlite_orm::sqlite_orm
)

set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GEN_BASE_PATH}")