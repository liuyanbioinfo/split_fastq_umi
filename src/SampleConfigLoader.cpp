// src/SampleConfigLoader.cpp
#include "SampleConfigLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool SampleConfigLoader::load(const std::string& filepath) {
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "无法打开样本配置文件: " << filepath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string fc, identifier, groups_str, samples_str;
        if (!(iss >> fc >> identifier >> groups_str >> samples_str)) {
            // 文件格式错误，忽略该行
            continue;
        }

        std::vector<std::string> groups = split(groups_str, ',');
        std::vector<std::string> samples = split(samples_str, ',');

        if (groups.size() != samples.size()) {
            // 条形码组和样本数量不匹配，忽略该行
            continue;
        }

        for (size_t i = 0; i < groups.size(); ++i) {
            group_to_sample[groups[i]] = samples[i];
        }
    }
    infile.close();
    return true;
}

const std::unordered_map<std::string, std::string>& SampleConfigLoader::get_group_sample_map() const {
    return group_to_sample;
}

std::vector<std::string> SampleConfigLoader::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

