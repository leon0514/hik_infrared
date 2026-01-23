#include "HikvisionCamera.h"
#include "system.hpp"
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

// 时间解析宏定义
#define GET_YEAR(_time_) (((_time_) >> 26) + 2000)
#define GET_MONTH(_time_) (((_time_) >> 22) & 15)
#define GET_DAY(_time_) (((_time_) >> 17) & 31)
#define GET_HOUR(_time_) (((_time_) >> 12) & 31)
#define GET_MINUTE(_time_) (((_time_) >> 6) & 63)
#define GET_SECOND(_time_) (((_time_) >> 0) & 63)


static std::unordered_map<std::string, int> ip_channel = {};

HikvisionCamera::HikvisionCamera(const std::string& ip, WORD port, const std::string& user, const std::string& password)
    : m_ip(ip), m_port(port), m_user(user), m_password(password), pool_(1) {
    SdkManager::getInstance();
    // 读取配置文件
    std::ifstream configFile("config.json");
    if (configFile.is_open()) {
        configFile >> config_;
        configFile.close();
    } 
    else {
        std::cerr << "Failed to open config.json" << std::endl;
    }
}

HikvisionCamera::~HikvisionCamera() {
    closeAlarm();
    logout();
}

bool HikvisionCamera::login() {
    if (m_userId >= 0) {
        std::cout << "Already logged in." << std::endl;
        return true;
    }

    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0;
    strncpy(struLoginInfo.sDeviceAddress, m_ip.c_str(), sizeof(struLoginInfo.sDeviceAddress) - 1);
    struLoginInfo.wPort = m_port;
    strncpy(struLoginInfo.sUserName, m_user.c_str(), sizeof(struLoginInfo.sUserName) - 1);
    strncpy(struLoginInfo.sPassword, m_password.c_str(), sizeof(struLoginInfo.sPassword) - 1);

    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    m_userId = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);

    if (m_userId < 0) {
        std::cerr << "Login failed for device " << m_ip << ". Error code: " << NET_DVR_GetLastError() << std::endl;
        return false;
    }

    std::cout << "Login success for device " << m_ip << ". UserID: " << m_userId << std::endl;
    return true;
}

void HikvisionCamera::logout() {
    if (m_userId >= 0) {
        NET_DVR_Logout(m_userId);
        m_userId = -1;
        std::cout << "Logged out from device " << m_ip << std::endl;
    }
}

bool HikvisionCamera::setupAlarm() {
    if (m_userId < 0) {
        std::cerr << "Cannot setup alarm: not logged in." << std::endl;
        return false;
    }

    // 设置报警回调函数，将 this 指针作为用户数据传递
    NET_DVR_SetDVRMessageCallBack_V50(0, staticAlarmCallback, this);

    NET_DVR_SETUPALARM_PARAM struAlarmParam = {0};
    struAlarmParam.dwSize = sizeof(struAlarmParam);
    // 可根据需要设置布防等级、布防类型等
    // struAlarmParam.byLevel = 1;

    m_alarmHandle = NET_DVR_SetupAlarmChan_V41(m_userId, &struAlarmParam);
    if (m_alarmHandle < 0) {
        std::cerr << "NET_DVR_SetupAlarmChan_V41 failed. Error code: " << NET_DVR_GetLastError() << std::endl;
        return false;
    }

    std::cout << "Alarm channel setup success. Handle: " << m_alarmHandle << std::endl;
    return true;
}

void HikvisionCamera::closeAlarm() {
    if (m_alarmHandle >= 0) {
        if (!NET_DVR_CloseAlarmChan_V30(m_alarmHandle)) {
            std::cerr << "NET_DVR_CloseAlarmChan_V30 failed. Error code: " << NET_DVR_GetLastError() << std::endl;
        }
        m_alarmHandle = -1;
        std::cout << "Alarm channel closed." << std::endl;
    }
}

// 静态回调函数，作为SDK的入口点
void CALLBACK HikvisionCamera::staticAlarmCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser) {
    // 将 pUser 转换回 HikvisionCamera 对象指针，并调用成员函数处理
    if (pUser != nullptr) {
        static_cast<HikvisionCamera*>(pUser)->handleAlarm(lCommand, pAlarmer, pAlarmInfo, dwBufLen);
    }
}

// 成员函数，处理具体的报警逻辑
// 开启线程将违章图片上传到系统服务器
void HikvisionCamera::handleAlarm(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen) {
    switch (lCommand) {
        case COMM_FIREDETECTION_ALARM: {
            NET_DVR_FIREDETECTION_ALARM struFireDetection = {0};
            memcpy(&struFireDetection, pAlarmInfo, sizeof(NET_DVR_FIREDETECTION_ALARM));

            // 打印报警信息
            printf("Fire Detection Alarm from %s: MaxTemp:%u, Distance:%u\n",
                   struFireDetection.struDevInfo.struDevIP.sIpV4,
                   struFireDetection.wFireMaxTemperature,
                   struFireDetection.wTargetDistance);
                   
            // 获取当前时间
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            char timeBuffer[20];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &tm);
            std::string timeStr(timeBuffer);
            printf("Alarm Time: %s\n", timeStr.c_str());
            
            // 将可见光图片直接上传到系统服务器
            if (struFireDetection.dwVisiblePicLen > 0 && struFireDetection.pVisiblePicBuf != NULL)
            {
                char cVisFilename[256] = {0};
                sprintf(cVisFilename, "VisibleFirePic_%s_%s.jpg",
                        struFireDetection.struDevInfo.struDevIP.sIpV4,
                        timeStr.c_str());

                std::string imageData(reinterpret_cast<const char*>(struFireDetection.pVisiblePicBuf),
                                      static_cast<size_t>(struFireDetection.dwVisiblePicLen));
                std::string ipAddr = std::string(struFireDetection.struDevInfo.struDevIP.sIpV4);
                int channelId = -1;
                if (ip_channel.find(ipAddr) != ip_channel.end()) {
                    channelId = ip_channel[ipAddr];
                }
                else
                {
                    channelId = get_channel_by_ip(
                        ipAddr);
                    ip_channel[ipAddr] = channelId;
                }
                if (channelId == -1) {
                    std::cerr << "No channelId found for IP: " << ipAddr << std::endl;
                    break;;
                }
                std::cout << "Uploading image for IP: " << ipAddr << ", Channel: " << channelId << std::endl;
                pool_.submitTask(upload_image_to_server, 
                                 std::string(cVisFilename), 
                                 imageData, 
                                 channelId,
                                 timeStr);
            }
            break;
        }
        default:
            printf("Other alarm type: 0x%X\n", lCommand);
            break;
    }
}