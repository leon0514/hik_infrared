#ifndef UPLOAD_HPP
#define UPLOAD_HPP

#include <string>

// 上传违章图片到系统服务器
void uploadImageToServer(const std::string& filename, const std::string& imageData, int channelId, const std::string& videoTime);
#endif // UPLOAD_HPP