// src/BarcodeGroupLoader.cpp
#include "BarcodeGroupLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool BarcodeGroupLoader::load(const std::string& filepath) {
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "无法打开条形码组文件: " << filepath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string group, barcode;
        if (!(iss >> group >> barcode)) {
            // 文件格式错误，忽略该行
            continue;
        }
        group_to_barcodes[group].push_back(barcode);
        barcode_length = barcode.length();
    }
    infile.close();
    return true;
}

const std::unordered_map<std::string, std::vector<std::string>>& BarcodeGroupLoader::get_group_barcodes() const {
    return group_to_barcodes;
}

const int& BarcodeGroupLoader::get_barcode_length() const {
    return barcode_length;
}