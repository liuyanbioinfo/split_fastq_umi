// include/SampleConfigLoader.h
#ifndef SAMPLECONFIGLOADER_H
#define SAMPLECONFIGLOADER_H

#include <string>
#include <unordered_map>
#include <vector>

class SampleConfigLoader {
public:
    // 加载样本配置文件
    bool load(const std::string& filepath);

    // 获取条形码组到样本的映射
    const std::unordered_map<std::string, std::string>& get_group_sample_map() const;

private:
    std::unordered_map<std::string, std::string> group_to_sample;

    // 辅助函数，用于分割字符串
    std::vector<std::string> split(const std::string& s, char delimiter);
};

#endif // SAMPLECONFIGLOADER_H

