// include/Statistics.h
#ifndef STATISTICS_H
#define STATISTICS_H

#include <atomic>
#include <unordered_map>
#include <string>

struct Statistics {
    std::atomic<size_t> total_reads{0};
    std::atomic<size_t> unknown_reads{0};
    std::unordered_map<std::string, size_t> sample_counts;
    // 校正后的统计信息
    std::unordered_map<std::string, size_t> corrected_counts;
};

#endif // STATISTICS_H

