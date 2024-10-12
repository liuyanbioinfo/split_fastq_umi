// src/BarcodeMatcher.cpp
#include "BarcodeMatcher.h"
#include "Utils.h"

BarcodeMatcher::BarcodeMatcher(const BarcodeGroupLoader& bg_loader,
                               const SampleConfigLoader& sc_loader,
                               const std::set<int>& allowed_positions)
    : allowed_positions_(allowed_positions) {
    const auto& group_barcodes = bg_loader.get_group_barcodes();
    const auto& group_sample = sc_loader.get_group_sample_map();

    for (const auto& [group, barcodes] : group_barcodes) {
        auto it = group_sample.find(group);
        if (it == group_sample.end()) {
            // 条形码组在样本配置中未找到对应样本，忽略
            continue;
        }
        const std::string& sample = it->second;
        for (const auto& barcode : barcodes) {
            barcode_to_sample[barcode] = sample;
        }
    }
}

// 根据条形码匹配样本名称
SampleMatch BarcodeMatcher::match(const std::string& barcode) const {
    SampleMatch samplematch;
    std::string best_match = "Unknown";
    bool corrected = false;
    std::string barcode_match = "NNNNNNNN";
    int min_dist = allowed_positions_.empty() ? 0 : barcode.length(); // 如果无容错位置，要求完全匹配
    //int min_dist = 1;

    for (const auto& [b, sample] : barcode_to_sample) {
        if (b.length() != barcode.length()) continue; // 长度不一致，跳过

        bool match_flag = true;
        int dist = 0;
        for (size_t i = 0; i < b.length(); ++i) {
            if (barcode[i] != b[i]) {
                if (allowed_positions_.find(i + 1) == allowed_positions_.end()) { // positions 1-based
                    match_flag = false;
                    break;
                }
                dist++;
            }
        }
        if (match_flag) {
            if (dist <= min_dist) {
                min_dist = dist;
                best_match = sample;
                barcode_match = b;
                if (dist == 0) break; // 最优匹配
                corrected = true;
            }
        }
    }
    //return best_match;
    samplematch.sample = best_match;
    samplematch.corrected = corrected;
    samplematch.barcode = barcode_match;
    return samplematch;
}

