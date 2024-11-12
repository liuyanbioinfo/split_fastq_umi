#ifndef READPAIR_H
#define READPAIR_H

#include <string>

struct ReadPair {
    std::string sample_name; // 样本名称
    bool corrected;
    // Read1
    std::string read1_line1; // ID行
    std::string read1_line2; // 序列行
    std::string read1_line3; // Plus行
    std::string read1_line4; // 质量行
    // Read2
    std::string read2_line1; // ID行
    std::string read2_line2; // 序列行
    std::string read2_line3; // Plus行
    std::string read2_line4; // 质量行
};

#endif // READPAIR_H

