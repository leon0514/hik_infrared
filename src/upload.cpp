#include "upload.hpp"
#include "httplib.h"
#include "json.hpp" 
#include <fstream>
#include <iostream>
#include <cstdlib>

static const std::string SERVER_IP = getenv("SERVER_IP") ? getenv("SERVER_IP") : "36.7.84.146";
static const int SERVER_PORT = getenv("SERVER_PORT") ? std::stoi(getenv("SERVER_PORT")) : 28801;
static const std::string SERVER_UPLOAD_PATH = "/open/api/operate/upload";
static const int CLASS_INDEX = getenv("CLASS_INDEX") ? std::stoi(getenv("CLASS_INDEX")) : 16;
static const std::string ALGM_IP = getenv("ALGM_IP") ? getenv("ALGM_IP") : "192.168.100.72";

static httplib::Client cli(SERVER_IP, SERVER_PORT);

/**
 * @brief 将图片和相关数据上传到服务器
 *
 * @param imagePath 要上传的本地图片文件路径
 * @param channelId 通道 ID
 * @param classIndex 类别索引
 * @param ipAddress 相机 IP 地址
 * @param videoTime 视频时间戳
 */
void uploadImageToServer(const std::string& filename, const std::string& imageData, int channelId, const std::string& videoTime)
{
    // httplib::Client cli(SERVER_IP.c_str()); // 在实际使用时，请确保cli已正确初始化
    cli.set_connection_timeout(5, 0);

    // 1. 构建 UploadFormDataItems 列表
    //    直接使用内存中的 imageData
    httplib::UploadFormDataItems items = {
        // 文件部分
        { "file", imageData, filename, "image/jpeg" },
        // 其他元数据
        { "channel", std::to_string(channelId), "", "" },
        { "classIndex", std::to_string(CLASS_INDEX), "", "" },
        { "ip", ALGM_IP, "", "" },
        { "videoTime", videoTime, "", "" }
    };

    printf("Uploading image %s (%zu bytes) from memory to %s%s...\n", filename.c_str(), imageData.length(), SERVER_IP.c_str(), SERVER_UPLOAD_PATH.c_str());

    // 2. 发送 POST 请求
    auto res = cli.Post(SERVER_UPLOAD_PATH.c_str(), items);

    // 3. 处理响应
    if (res) {
        if (res->status == 200) {
            try {
                auto jsonResponse = nlohmann::json::parse(res->body);
                if (jsonResponse.contains("code") && jsonResponse["code"] == 0) {
                    printf("Image uploaded successfully: %s\n", filename.c_str());
                } else {
                    printf("Image upload failed (server logic error): %s, Response: %s\n", filename.c_str(), res->body.c_str());
                }
            } catch (const nlohmann::json::parse_error& e) {
                printf("JSON parsing error for response: %s, Error: %s\n", res->body.c_str(), e.what());
            }
        } else {
            printf("HTTP request failed for image: %s, Status code: %d, Body: %s\n",
                   filename.c_str(), res->status, res->body.c_str());
        }
    } else {
        auto err = res.error();
        printf("HTTP request failed for image: %s, Error: %s\n", filename.c_str(), httplib::to_string(err).c_str());
    }
}



// int main() {
//     // 示例调用
//     uploadImageToServer(std::string("test.jpg"), 10, CLASS_INDEX, "172.16.22.16", "2026-04-05 12:34:56");
//     return 0;
// }