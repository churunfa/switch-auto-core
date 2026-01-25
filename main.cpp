#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

// 引入由 protoc 编译器生成的头文件
#include "SwitchControlLibrary.h"
#include "service/BaseOperateService.h"
#include "src/repo/DatabaseManager.h"


void RunServer() {
    constexpr std::string server_address("0.0.0.0:50051");
    service::BaseOperateServiceImpl base_operate_service_impl;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // 注册服务
    builder.RegisterService(&base_operate_service_impl);

    const std::unique_ptr server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // 阻塞等待，直到服务器关闭
    server->Wait();
}

int main(int argc, char** argv) {
    DatabaseManager::getInstance();
    SwitchControlLibrary::getInstance();
    RunServer();
    return 0;
}