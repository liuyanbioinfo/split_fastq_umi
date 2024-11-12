#ifndef BARCODEGROUPLOADER_H
#define BARCODEGROUPLOADER_H

#include <string>
#include <unordered_map>
#include <vector>

class BarcodeGroupLoader {
public:
    // 加载UMI分组文件
    bool load(const std::string& filepath);

    // 获取所有UMI分组及其UMI条形码
    const std::unordered_map<std::string, std::vector<std::string>>& get_group_barcodes() const;

    // 获取UMI长度
    const int& get_barcode_length() const;

private:
    std::unordered_map<std::string, std::vector<std::string>> group_to_barcodes;
    int barcode_length;

};

#endif // BARCODEGROUPLOADER_H

