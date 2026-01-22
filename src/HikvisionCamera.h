#pragma once

#include "HCNetSDK.h"
#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include "json.hpp"
#include "threadPool.hpp"

/**
 * @class SdkManager
 * @brief 使用单例模式管理全局唯一的SDK初始化和清理。
 * 确保 NET_DVR_Init() 和 NET_DVR_Cleanup() 在整个程序生命周期中只被调用一次。
 */
class SdkManager {
public:
    // 获取单例实例
    static SdkManager& getInstance() {
        static SdkManager instance;
        return instance;
    }

private:
    // 构造函数中初始化SDK
    SdkManager() {
        if (!NET_DVR_Init()) {
            throw std::runtime_error("Failed to initialize HCNetSDK. Error code: " + std::to_string(NET_DVR_GetLastError()));
        }
        // 设置连接和重连参数
        NET_DVR_SetConnectTime(2000, 1);
        NET_DVR_SetReconnect(10000, true);
        std::cout << "HCNetSDK initialized successfully." << std::endl;
    }

    // 析构函数中清理SDK
    ~SdkManager() {
        NET_DVR_Cleanup();
        std::cout << "HCNetSDK cleaned up." << std::endl;
    }

    // 删除拷贝构造和赋值操作，确保单例
    SdkManager(const SdkManager&) = delete;
    SdkManager& operator=(const SdkManager&) = delete;
};

/**
 * @class HikvisionCamera
 * @brief 封装海康威视摄像头设备的操作。
 * 每个实例代表一个与物理摄像头的连接。
 */
class HikvisionCamera {
public:
    /**
     * @brief 构造函数。
     * @param ip 设备IP地址。
     * @param port 设备端口号。
     * @param user 用户名。
     * @param password 密码。
     */
    HikvisionCamera(const std::string& ip, WORD port, const std::string& user, const std::string& password);

    /**
     * @brief 析构函数。
     * 自动处理注销和撤防。
     */
    ~HikvisionCamera();

    /**
     * @brief 登录设备。
     * @return 如果登录成功返回 true，否则返回 false。
     */
    bool login();

    /**
     * @brief 注销设备。
     */
    void logout();

    /**
     * @brief 启动报警监听（布防）。
     * @return 如果成功返回 true，否则返回 false。
     */
    bool setupAlarm();

    /**
     * @brief 停止报警监听（撤防）。
     */
    void closeAlarm();

    // 禁止拷贝和赋值
    HikvisionCamera(const HikvisionCamera&) = delete;
    HikvisionCamera& operator=(const HikvisionCamera&) = delete;

private:
    // SDK全局报警回调函数
    static void CALLBACK staticAlarmCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser);
    
    // 类的成员回调函数，用于处理具体的报警信息
    void handleAlarm(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen);

    // 设备登录信息
    std::string m_ip;
    WORD m_port;
    std::string m_user;
    std::string m_password;

    // SDK返回的用户ID和报警句柄
    LONG m_userId = -1;
    LONG m_alarmHandle = -1;

    ThreadPool pool_;
    nlohmann::json config_;
};