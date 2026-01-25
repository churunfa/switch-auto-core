#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

// 引入由 protoc 编译器生成的头文件
#include "helloworld.grpc.pb.h"
#include "src/repo/DatabaseManager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloRequest;
using helloworld::HelloReply;

// 1. 实现服务逻辑
class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override {
        // 获取 Java 发来的名字
        std::string prefix("Hello from C++, ");

        // 构造返回消息
        reply->set_message(prefix + request->name());

        std::cout << "Received request: " << request->name() << std::endl;

        return Status::OK;
    }
};

void RunServer() {
    constexpr std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    ServerBuilder builder;
    // 监听端口，不使用 SSL/TLS
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // 注册服务
    builder.RegisterService(&service);

    const std::unique_ptr server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // 阻塞等待，直到服务器关闭
    server->Wait();
}

int main(int argc, char** argv) {
    DatabaseManager::getInstance();
    RunServer();
    return 0;
}