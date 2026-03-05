#include "system.hpp"
#include <iomanip>
#include <sstream>

static const std::string SERVER_IP = getenv("SERVER_IP") ? getenv("SERVER_IP") : "36.7.84.146";
static const int SERVER_PORT = getenv("SERVER_PORT") ? std::stoi(getenv("SERVER_PORT")) : 28801;
static const std::string SERVER_UPLOAD_PATH = "/open/api/operate/upload";
static const std::string SERVER_TASK_PATH = "/open/api/operate/taskList";
static const int CLASS_INDEX = getenv("CLASS_INDEX") ? std::stoi(getenv("CLASS_INDEX")) : 0;
static const std::string ALGM_IP = getenv("ALGM_IP") ? getenv("ALGM_IP") : "192.168.100.72";

static httplib::Client cli(SERVER_IP, SERVER_PORT);

/**
 * @brief 从 JSON 对象中安全地获取一个值。
 *
 * 这个函数可以同时处理 "键不存在" 和 "键存在但值为 null" 的情况。
 * @tparam T 期望获取的值的类型 (例如 std::string, int)。
 * @param j The nlohmann::json object to read from.
 * @param key 要查找的键。
 * @param default_value 如果键不存在或其值为 null，则返回此默认值。
 * @return 获取到的值或默认值。
 */
template <typename T>
T get_value_or_default(const json &j, const std::string &key, const T &default_value)
{
    // 1. 检查键是否存在
    if (!j.contains(key))
    {
        return default_value;
    }
    // 2. 检查键对应的值是否为 null
    if (j.at(key).is_null())
    {
        return default_value;
    }
    // 3. 如果键存在且值不为 null，则返回值
    return j.at(key).get<T>();
}

// URL 编码辅助函数
std::string url_encode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
        }
        else
        {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}

// 将 JSON 转换为 key1=val1&key2=val2 字符串
std::string json_to_form_body(const json &j)
{
    std::string res;
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        if (!res.empty())
            res += "&";
        res += it.key() + "=";

        // 确保值被正确转为字符串并编码
        std::string val;
        if (it.value().is_string())
        {
            val = it.value().get<std::string>();
        }
        else
        {
            val = it.value().dump(); // 数字或布尔值转为字符串
        }
        res += url_encode(val);
    }
    return res;
}

std::optional<json> post_form_data(const std::string &path, const json &request_data)
{
    try
    {
        cli.set_connection_timeout(5, 0);
        cli.set_read_timeout(5, 0);

        // 1. 将 JSON 转换为表单字符串 (key1=val1&key2=val2)
        std::string request_body = json_to_form_body(request_data);

        // 2. 构建 Headers (完全模拟你的 curl 参数)
        httplib::Headers headers = {
            {"Accept", "*/*"},
            {"Request-Origion", "Knife4j"},
            // Content-Type 会在 Post 函数参数中指定，但显式写在这里也可以
        };

        // 3. 发送 POST 请求
        // 注意：httplib 的 Post 签名中可以传入 headers
        auto res = cli.Post(path.c_str(), headers, request_body, "application/x-www-form-urlencoded");

        if (!res)
        {
            auto err = res.error();
            std::cerr << "[HTTP Error] Form Request failed: " << httplib::to_string(err) << std::endl;
            return std::nullopt;
        }

        if (res->status != 200)
        {
            std::cerr << "[HTTP Error] Server responded with status code: " << res->status << std::endl;
            std::cerr << "Response body: " << res->body << std::endl;
            return std::nullopt;
        }

        try
        {
            auto data = json::parse(res->body);
            std::cout << "[HTTP Success] Received response: " << data.dump() << std::endl;
            return data;
        }
        catch (json::parse_error &e)
        {
            std::cerr << "[JSON Error] Failed to parse response body: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Exception] An exception occurred: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<json> post_json_data(const std::string &path, const json &request_data)
{
    try
    {
        cli.set_connection_timeout(5, 0); // 5秒连接超时
        cli.set_read_timeout(5, 0);       // 5秒读取超时

        std::string request_body = request_data.dump();
        auto res = cli.Post(path.c_str(), request_body, "application/json");

        if (!res)
        {
            auto err = res.error();
            std::cerr << "[HTTP Error] Request failed: " << httplib::to_string(err) << std::endl;
            return std::nullopt;
        }

        if (res->status != 200)
        {
            std::cerr << "[HTTP Error] Server responded with status code: " << res->status << std::endl;
            std::cerr << "Response body: " << res->body << std::endl;
            return std::nullopt;
        }

        try
        {
            auto data = json::parse(res->body);
            std::cout << "[HTTP Success] Received response: " << data.dump() << std::endl;
            return data;
        }
        catch (json::parse_error &e)
        {
            std::cerr << "[JSON Error] Failed to parse response body: " << e.what() << std::endl;
            std::cerr << "Original response body: " << res->body << std::endl;
            return std::nullopt;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Exception] An exception occurred: " << e.what() << std::endl;
        return std::nullopt;
    }
}

/**
 * @brief 获取算法任务列表。
 * @param host 服务器主机名或 IP。
 * @param port 服务器端口。
 * @param filters 包含请求参数的 json 对象 (如 algorithmCode, deviceIp, fwqCode)。
 * @return 如果成功，返回一个包含 AiTaskVo 对象的 vector；如果失败，返回 std::nullopt。
 */
std::optional<std::vector<AiTaskVo>> get_task_list(const json &filters)
{
    // 调用底层函数发送请求
    std::cout << filters.dump() << std::endl;
    auto response_opt = post_form_data(SERVER_TASK_PATH, filters);
    if (!response_opt)
    {
        std::cerr << "[API Error] Failed to get a valid response from the server." << std::endl;
        return std::nullopt;
    }

    const json &response_json = response_opt.value();

    // 检查业务返回码
    if (!response_json.contains("code") || response_json["code"] != 0)
    {
        std::cerr << "[API Error] The server returned a business error." << std::endl;
        std::cerr << "  Code: " << response_json.value("code", -1) << std::endl;
        std::cerr << "  Message: " << response_json.value("msg", "No message provided.") << std::endl;
        return std::nullopt;
    }

    // 检查并解析 data 字段
    if (!response_json.contains("data") || !response_json["data"].is_array())
    {
        std::cerr << "[API Error] Response format is incorrect: 'data' field is missing or is not an array." << std::endl;
        return std::nullopt;
    }

    std::vector<AiTaskVo> tasks;
    try
    {
        // 遍历 data 数组中的每个任务对象
        for (const auto &task_json : response_json["data"])
        {
            AiTaskVo task;

            // 使用 .value() 安全地获取每个字段的值，如果字段不存在则使用默认值
            task.fwqCode = get_value_or_default<std::string>(task_json, "fwqCode", "");
            task.id = get_value_or_default<int>(task_json, "id", 0);
            task.alarmTaskName = get_value_or_default<std::string>(task_json, "alarmTaskName", "");
            task.deviceId = get_value_or_default<int>(task_json, "deviceId", 0);
            task.deviceName = get_value_or_default<std::string>(task_json, "deviceName", "");
            task.deviceIp = get_value_or_default<std::string>(task_json, "deviceIp", "");
            task.deviceAlgorithmIp = get_value_or_default<std::string>(task_json, "deviceAlgorithmIp", "");
            task.cameraType = get_value_or_default<std::string>(task_json, "cameraType", "");
            task.deviceChannel = get_value_or_default<std::string>(task_json, "deviceChannel", "");
            task.videoName = get_value_or_default<std::string>(task_json, "videoName", "");
            task.videoPassword = get_value_or_default<std::string>(task_json, "videoPassword", "");
            task.algorithmCode = get_value_or_default<std::string>(task_json, "algorithmCode", "");
            task.algorithmName = get_value_or_default<std::string>(task_json, "algorithmName", "");
            task.beforeDrawPhoto = get_value_or_default<std::string>(task_json, "beforeDrawPhoto", "");
            task.electricFence = get_value_or_default<std::string>(task_json, "electricFence", "");
            task.playbackAddress = get_value_or_default<std::string>(task_json, "playbackAddress", "");
            task.deviceCode = get_value_or_default<std::string>(task_json, "deviceCode", "");
            task.sleepJudgeTime = get_value_or_default<int>(task_json, "sleepJudgeTime", 0);
            task.personLimitNum = get_value_or_default<int>(task_json, "personLimitNum", 0);

            tasks.push_back(task);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[JSON Error] An error occurred while parsing the 'data' array: " << e.what() << std::endl;
        return std::nullopt;
    }

    return tasks;
}

int get_channel_by_ip(const std::string &device_ip)
{
    // 简单映射示例，根据实际需求修改
    if (device_ip.empty())
    {
        return -1; // 默认通道
    }
    json filters;
    filters["deviceIp"] = device_ip;
    filters["algorithmCode"] = std::to_string(CLASS_INDEX);
    auto tasks_opt = get_task_list(filters);
    if (!tasks_opt || tasks_opt->empty())
    {
        return -1;
    }
    return std::stoi(tasks_opt->front().deviceChannel);
}

/**
 * @brief 将图片和相关数据上传到服务器
 *
 * @param imagePath 要上传的本地图片文件路径
 * @param channelId 通道 ID
 * @param classIndex 类别索引
 * @param ipAddress 相机 IP 地址
 * @param videoTime 视频时间戳
 */
void upload_image_to_server(const std::string &filename, const std::string &imageData, int channelId, const std::string &videoTime)
{
    // httplib::Client cli(SERVER_IP.c_str()); // 在实际使用时，请确保cli已正确初始化
    cli.set_connection_timeout(5, 0);

    // 1. 构建 UploadFormDataItems 列表
    //    直接使用内存中的 imageData
    httplib::UploadFormDataItems items = {
        // 文件部分
        {"file", imageData, filename, "image/jpeg"},
        {"channel", std::to_string(channelId), "", ""},
        {"classIndex", std::to_string(CLASS_INDEX), "", ""},
        {"ip", ALGM_IP, "", ""},
        {"videoTime", videoTime, "", ""}};

    printf("Uploading image %s (%zu bytes) from memory to %s%s...\n", filename.c_str(), imageData.length(), SERVER_IP.c_str(), SERVER_UPLOAD_PATH.c_str());

    // 2. 发送 POST 请求
    auto res = cli.Post(SERVER_UPLOAD_PATH.c_str(), items);

    // 3. 处理响应
    if (res)
    {
        if (res->status == 200)
        {
            try
            {
                auto jsonResponse = nlohmann::json::parse(res->body);
                if (jsonResponse.contains("code") && jsonResponse["code"] == 0)
                {
                    printf("Image uploaded successfully: %s\n", filename.c_str());
                }
                else
                {
                    printf("Image upload failed (server logic error): %s, Response: %s\n", filename.c_str(), res->body.c_str());
                }
            }
            catch (const nlohmann::json::parse_error &e)
            {
                printf("JSON parsing error for response: %s, Error: %s\n", res->body.c_str(), e.what());
            }
        }
        else
        {
            printf("HTTP request failed for image: %s, Status code: %d, Body: %s\n",
                   filename.c_str(), res->status, res->body.c_str());
        }
    }
    else
    {
        auto err = res.error();
        printf("HTTP request failed for image: %s, Error: %s\n", filename.c_str(), httplib::to_string(err).c_str());
    }
}