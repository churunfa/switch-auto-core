#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "SwitchControlLibrary.h"
#include "cache/CacheLoader.h"
#include "controller/ControllerMonitor.h"
#include "service/BaseOperateService.h"
#include "service/CombinationGraphService.h"
#include "src/repo/DatabaseManager.h"


void RunServer() {
    constexpr std::string server_address("0.0.0.0:50051");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // 注册服务
    builder.RegisterService(new service::BaseOperateServiceImpl());
    builder.RegisterService(new service::CombinationGraphServiceImpl());

    const std::unique_ptr server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // 阻塞等待，直到服务器关闭
    server->Wait();
}

int main(int argc, char** argv) {
    DatabaseManager::getInstance();
    SwitchControlLibrary::getInstance();
    CacheLoader::getInstance();
    ControllerMonitor::getInstance().start();
    RunServer();
    return 0;
}