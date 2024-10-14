// src/main.cpp
#include <zlib.h> 
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <set>
#include "iomanip"
#include "cmdline.h"
#include "ReadPair.h"
#include "SPSCQueue.h"
#include "Utils.h"
#include "BarcodeGroupLoader.h"
#include "SampleConfigLoader.h"
#include "BarcodeMatcher.h"
#include "Statistics.h"
#include "ProducerConsumer.h"
#include "Version.h"

using namespace std;

// 主函数
int main(int argc, char* argv[]) {
    // 定义命令行参数
    cmdline::parser cmd;
    cmd.add<std::string>("barcode_group_file",'G', "Barcode group file", true, "");
    cmd.add<std::string>("sample_config_file",'S', "Sample configuration file", true, "");
    cmd.add<std::string>("fastq1",'i', "Input fastq1.gz file", true, "");
    cmd.add<std::string>("fastq2",'I', "Input fastq2.gz file", true, "");
    cmd.add<std::string>("output_dir",'o', "Output directory", true, "");
    cmd.add<std::string>("p_pos",'p', "Mismatch tolerant position in barcode (1-8), 1-based, default=7, -1 to disable", false, "7");
    cmd.add("disable_fix_mgi_id",0,"the MGI FASTQ ID format is not compatible with many BAM operation tools, default is fix_mgi_id, enable this option to diable the fix.");

    cmd.parse_check(argc, argv);

    if(argc == 1) {
        cerr << cmd.usage() <<endl;
    }

    std::string barcode_group_file = cmd.get<std::string>("barcode_group_file");
    std::string sample_config_file = cmd.get<std::string>("sample_config_file");
    std::string fastq1_file = cmd.get<std::string>("fastq1");
    std::string fastq2_file = cmd.get<std::string>("fastq2");
    std::string output_dir = cmd.get<std::string>("output_dir");

    std::set<int> allowed_positions;
    std::string p_str = cmd.get<std::string>("p_pos");
    allowed_positions = parse_allowed_positions(p_str);
    bool fixMGI = !cmd.exist("disable_fix_mgi_id");

    std::time_t t1 = std::time(nullptr);
    // 加载配置文件
    BarcodeGroupLoader bg_loader;
    if (!bg_loader.load(barcode_group_file)) {
        cerr << barcode_group_file << " Error" << std::endl;
        return 1;
    }
    const int barcode_length = bg_loader.get_barcode_length();

    SampleConfigLoader sc_loader;
    if (!sc_loader.load(sample_config_file)) {
        cerr << sample_config_file << " Error" << std::endl;
        return 1;
    }

    // 初始化UMI匹配器
    BarcodeMatcher matcher(bg_loader, sc_loader, allowed_positions);

    // 初始化队列
    SPSCQueue<ReadPair> queue;

    // 初始化统计信息
    Statistics stats;

    // 初始化完成标志
    std::atomic<bool> finished(false);

    // 启动生产者线程
    std::thread producer_thread(producer, std::ref(queue), fastq1_file, fastq2_file, std::ref(matcher), barcode_length, fixMGI, std::ref(stats), std::ref(finished));

    // 创建映射样本名到文件句柄的映射
    std::unordered_map<std::string, gzFile> sample_to_file_R1;
    std::unordered_map<std::string, gzFile> sample_to_file_R2;

    // 启动消费者线程
    std::thread consumer_thread(consumer, std::ref(queue), output_dir, std::ref(sample_to_file_R1), std::ref(sample_to_file_R2), std::ref(finished));

    // 等待线程完成
    producer_thread.join();
    consumer_thread.join();

    // 获取当前时间
    std::time_t t2 = std::time(nullptr);
    char time_buf[100];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t2));

    // 构建命令行字符串
    std::string cmd_line = "";
    for (int i = 0; i < argc; ++i) {
        cmd_line += " ";
        cmd_line += argv[i];
    }

    // 输出统计信息到标准输出（可通过重定向保存到日志文件）
    std::cout << "Total: " << stats.total_reads.load() << "\n";
    std::cout << "Unknown: " << stats.unknown_reads.load() << "\tPercentage: "
              << setiosflags(ios::fixed)<<setprecision(3) 
              << (stats.total_reads.load() > 0 ? 
                  (static_cast<double>(stats.unknown_reads.load()) / stats.total_reads.load()) * 100 : 0) 
              << "\n";
    std::cout << "#Sample\tCount\tPercentage\tCorrectedCount\tCorrectedPercentage\tCorrectedPercentageAll\n";

    for (const auto& [sample, count] : stats.sample_counts) {
        // 拆分统计
        auto corrected_count = stats.corrected_counts[sample];
        double percentage = (stats.total_reads.load() > 0) ? 
            (static_cast<double>(count) / stats.total_reads.load()) * 100 : 0;
        
        double corrected_percentage = (count > 0) ? 
            (static_cast<double>(corrected_count) / count) * 100 : 0;
        double corrected_percentage_all = (stats.total_reads.load() > 0) ?
            (static_cast<double>(corrected_count) / stats.total_reads.load()) * 100 : 0;
        std::cout << setiosflags(ios::fixed)<<setprecision(3) 
                  << sample << "\t" << count << "\t" << percentage 
                  << "\t" << corrected_count << "\t" << corrected_percentage 
                  << "\t" << corrected_percentage_all << "\n";
    }

    std::cout << "[" << time_buf << "] " << cmd_line << "\n";
    std::cout << "[Version: "<< VERSION ", Time used: " << (t2)-t1  << " seconds]\n";

    return 0;
}

