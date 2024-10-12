// include/BarcodeMatcher.h
#ifndef BARCODEMATCHER_H
#define BARCODEMATCHER_H

#include "BarcodeGroupLoader.h"
#include "SampleConfigLoader.h"
#include <string>
#include <unordered_map>
#include <set>

struct SampleMatch {
    std::string sample;
    bool corrected;
    std::string barcode; // matched barcode
};

class BarcodeMatcher {
public:
    BarcodeMatcher(const BarcodeGroupLoader& bg_loader,
                  const SampleConfigLoader& sc_loader,
                  const std::set<int>& allowed_positions);

    // 根据条形码匹配样本名称
    // 只允许在指定位置有不匹配
    SampleMatch match(const std::string& barcode) const;

private:
    std::unordered_map<std::string, std::string> barcode_to_sample;
    std::set<int> allowed_positions_; // 1-based positions where mismatches are allowed
};

#endif // BARCODEMATCHER_H

