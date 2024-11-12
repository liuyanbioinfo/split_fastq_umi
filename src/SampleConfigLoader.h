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

    // 获取所有输出样本
    const std::vector<std::string>& get_sample_names() const;

private:
    std::unordered_map<std::string, std::string> group_to_sample;

    // 辅助函数，用于分割字符串
    std::vector<std::string> split(const std::string& s, char delimiter);

    // 所有条形码对应的输出样本
    std::vector<std::string> sample_names;
};

#endif // SAMPLECONFIGLOADER_H

