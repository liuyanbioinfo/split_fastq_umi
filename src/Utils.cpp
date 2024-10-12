// src/Utils.cpp
#include "Utils.h"
#include <sstream>
#include <iostream>
#include <stdexcept>

// 计算两个字符串的Hamming距离
int hamming_distance(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return a.length(); // 不同长度，返回最大距离
    int dist = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        if (a[i] != b[i]) dist++;
    }
    return dist;
}

// 将逗号分隔的字符串转换为整数集合（1-based）
std::set<int> parse_allowed_positions(const std::string& p_str) {
    std::set<int> positions;
    std::stringstream ss(p_str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        try {
            int pos = std::stoi(item);
            if (pos <= 0) {
                std::cerr << "Invalid position in -p: " << pos << ". Positions must be positive integers." << std::endl;
                continue;
            }
            positions.insert(pos);
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Invalid position format in -p: " << item << std::endl;
        }
        catch (const std::out_of_range&) {
            std::cerr << "Position out of range in -p: " << item << std::endl;
        }
    }
    return positions;
}

