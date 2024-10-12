// include/BarcodeGroupLoader.h
#ifndef BARCODEGROUPLOADER_H
#define BARCODEGROUPLOADER_H

#include <string>
#include <unordered_map>
#include <vector>

class BarcodeGroupLoader {
public:
    // 加载条形码组文件
    bool load(const std::string& filepath);

    // 获取所有条形码组及其条形码
    const std::unordered_map<std::string, std::vector<std::string>>& get_group_barcodes() const;

private:
    std::unordered_map<std::string, std::vector<std::string>> group_to_barcodes;
};

#endif // BARCODEGROUPLOADER_H

