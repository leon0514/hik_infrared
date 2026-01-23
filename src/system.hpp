
#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

struct AiTaskVo {
    std::string fwqCode;
    int id = 0;
    std::string alarmTaskName;
    int deviceId = 0;
    std::string deviceName;
    std::string deviceIp;
    std::string deviceAlgorithmIp;
    std::string cameraType;
    std::string deviceChannel;
    std::string videoName;
    std::string videoPassword;
    std::string algorithmCode;
    std::string algorithmName;
    std::string beforeDrawPhoto;
    std::string electricFence;
    std::string playbackAddress;
    std::string deviceCode;
    int sleepJudgeTime = 0;
    int personLimitNum = 0;
};

std::optional<json> post_json_data(const std::string& path, const json& request_data);
std::optional<std::vector<AiTaskVo>> get_task_list(const json& filters);

int get_channel_by_ip(const std::string& device_ip);
void upload_image_to_server(const std::string& filename, const std::string& imageData, int channelId, const std::string& videoTime);


#endif