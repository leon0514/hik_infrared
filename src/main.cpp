#include "HikvisionCamera.h"
#include <iostream>
#include <future>
#include <csignal>

// 创建一个全局的 promise
std::promise<void> exit_promise;

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
    exit_promise.set_value();
}

// 从 env 中获取 NVR 连接信息，若未设置则使用默认值 
static const std::string NVR_IP = getenv("NVR_IP") ? getenv("NVR_IP") : "172.16.22.16"; 
static const WORD NVR_PORT = getenv("NVR_PORT") ? static_cast<WORD>(std::stoi(getenv("NVR_PORT"))) : 8000;  
static const std::string NVR_USER = getenv("NVR_USER") ? getenv("NVR_USER") : "admin";
static const std::string NVR_PASSWORD = getenv("NVR_PASSWORD") ? getenv("NVR_PASSWORD") : "lww123456";

int main() 
{
    printf("HikVision NVR INFO\n");
    printf("===============================\n");
    printf("IP   : %s\n", NVR_IP.c_str());
    printf("Port : %d\n", NVR_PORT);
    printf("User : %s\n", NVR_USER.c_str());
    printf("===============================\n");
    signal(SIGINT, signalHandler);
    try 
    {
        // 创建摄像头对象，自动初始化SDK
        HikvisionCamera camera(NVR_IP, NVR_PORT, NVR_USER, NVR_PASSWORD);

        // 登录
        if (!camera.login()) {
            return -1;
        }

        // 布防
        if (!camera.setupAlarm()) {
            return -1;
        }

        std::cout << "Press Ctrl+C to exit..." << std::endl;
        // 从 promise 获取 future
        std::future<void> exit_future = exit_promise.get_future();
        // 主线程将在此处阻塞，直到 exit_promise.set_value() 被调用
        exit_future.wait();

    } 
    catch (const std::runtime_error& e) 
    {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        return -1;
    }
    std::cout << "Exiting application." << std::endl;
    return 0;
}