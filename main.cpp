#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <csignal>
#include <atomic>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif
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
    ControllerMonitor::getInstance().isRunning = false;
}

void RunServer() {
    std::string server_address = "0.0.0.0:";
    if (const char* port_env = std::getenv("SWITCH_AUTO_CORE_PORT")) {
        server_address += port_env;
    } else {
        server_address += "50051";
    }

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
    // 禁用 SDL 断言弹窗
    #ifdef _WIN32
    SetEnvironmentVariableA("SDL_ASSERT", "always_ignore");
    SDL_SetAssertionHandler(nullptr, nullptr);
    // 强制设置控制台输出为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // 强制设置控制台输入为 UTF-8 (可选)
    SetConsoleCP(CP_UTF8);
    #else
    #endif
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    std::cout << "按 Ctrl+C 退出程序" << std::endl;

    DatabaseManager::getInstance();
    SwitchControlLibrary::getInstance();
    CacheLoader::getInstance();
    
    std::thread server_thread(RunServer);
    server_thread.detach();

    ControllerMonitor::getInstance().start();
    std::cout << "程序已退出" << std::endl;
    
    return 0;
}