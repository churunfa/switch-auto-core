#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <csignal>
#include <atomic>
#include "SwitchControlLibrary.h"
#include "cache/CacheLoader.h"
#include "controller/ControllerMonitor.h"
#include "service/BaseOperateService.h"
#include "service/CombinationGraphService.h"
#include "service/ButtonBindingService.h"
#include "src/repo/DatabaseManager.h"


static std::atomic shutdown_requested{false};
static std::unique_ptr<grpc::Server> global_server;

void signal_handler(const int signal) {
    std::cout << "\n接收到信号 " << signal << "，正在关闭..." << std::endl;
    shutdown_requested = true;
    if (global_server) {
        global_server->Shutdown();
    }
}

void RunServer() {
    constexpr std::string server_address("0.0.0.0:50051");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(new service::BaseOperateServiceImpl());
    builder.RegisterService(new service::CombinationGraphServiceImpl());
    builder.RegisterService(new service::ButtonBindingServiceImpl());

    global_server = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    global_server->Shutdown();
    std::cout << "服务器已关闭" << std::endl;
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    std::cout << "按 Ctrl+C 退出程序" << std::endl;

    DatabaseManager::getInstance();
    SwitchControlLibrary::getInstance();
    CacheLoader::getInstance();
    ControllerMonitor::getInstance().start();
    
    RunServer();
    
    ControllerMonitor::getInstance().stop();
    std::cout << "程序已退出" << std::endl;
    
    return 0;
}