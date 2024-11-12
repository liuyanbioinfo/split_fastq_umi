#include "ProducerConsumer.h"
#include "util.h"
#include <iostream>
#include <thread>
#include <unordered_map>
#include "writer.h"

// 生产者函数实现
void producer(SPSCQueue<ReadPair>& queue, 
              const std::string& fq1_input, 
              const std::string& fq2_input, 
              const BarcodeMatcher& matcher,
              const int barcode_length,
              const bool fixMGI, 
              Statistics& stats, 
              std::atomic<bool>& finished) {
    gzFile infile1 = gzopen(fq1_input.c_str(), "rb");
    gzFile infile2 = gzopen(fq2_input.c_str(), "rb");

    if (!infile1 || !infile2) {
        std::cerr << "无法打开输入文件。" << std::endl;
        if (infile1) gzclose(infile1);
        if (infile2) gzclose(infile2);
        finished.store(true, std::memory_order_release);
        return;
    }

    // 设置更大的缓冲区
    gzbuffer(infile1, 3 << 20); // 3MB
    gzbuffer(infile2, 3 << 20); // 3MB

    char buffer1[1024];
    char buffer2[1024];
    size_t totalReads = 0;
    size_t unknownReads = 0;

    while (true) {
        ReadPair read_pair;
        read_pair.corrected = false; //默认没有校正，后面检验barcode会更新        

        // 读取 READ1 的四行
        bool eof1 = false;
        for (int i = 0; i < 4; ++i) {
            if (!gzgets(infile1, buffer1, sizeof(buffer1))) {
                eof1 = true;
                break;
            }
            std::string line(buffer1);
            switch(i) {
                case 0: read_pair.read1_line1 = line; break;
                case 1: read_pair.read1_line2 = line; break;
                case 2: read_pair.read1_line3 = line; break;
                case 3: read_pair.read1_line4 = line; break;
            }
        }
        if (eof1) break;

        // 读取 READ2 的四行
        bool eof2 = false;
        for (int i = 0; i < 4; ++i) {
            if (!gzgets(infile2, buffer2, sizeof(buffer2))) {
                eof2 = true;
                break;
            }
            std::string line(buffer2);
            switch(i) {
                case 0: read_pair.read2_line1 = line; break;
                case 1: read_pair.read2_line2 = line; break;
                case 2: read_pair.read2_line3 = line; break;
                case 3: read_pair.read2_line4 = line; break;
            }
        }
        if (eof2) break;

        // 提取 UMI 和条形码
        std::string barcode1, barcode2;
        if (read_pair.read1_line2.length() >= barcode_length) {
            barcode1 = read_pair.read1_line2.substr(0, barcode_length);
        } else {
            barcode1 = read_pair.read1_line2;
        }

        if (read_pair.read2_line2.length() >= barcode_length) {
            barcode2 = read_pair.read2_line2.substr(0, barcode_length);
        } else {
            barcode2 = read_pair.read2_line2;
        }

        // 进行UMI匹配
        SampleMatch samplematch1 = matcher.match(barcode1);
        SampleMatch samplematch2 = matcher.match(barcode2);
        std::string sample1 = samplematch1.sample;
        std::string sample2 = samplematch2.sample;
        bool corrected1 = samplematch1.corrected;
        bool corrected2 = samplematch2.corrected;
        // 校正barcode
        if (corrected1){
            read_pair.read1_line2 = samplematch1.barcode + read_pair.read1_line2.substr(barcode_length);
        }
        if (corrected2){
            read_pair.read2_line2 = samplematch2.barcode + read_pair.read2_line2.substr(barcode_length);
        }

        // BGI /1 /2 SUFFIX FIX
        if (fixMGI && read_pair.read1_line1.length() >3 && read_pair.read2_line1.length() >3 && read_pair.read1_line1.substr(read_pair.read1_line1.length() - 3) == "/1\n" && read_pair.read2_line1.substr(read_pair.read2_line1.length() - 3) == "/2\n"){
            read_pair.read1_line1.erase(read_pair.read1_line1.length() - 3, 2);
            read_pair.read2_line1.erase(read_pair.read2_line1.length() - 3, 2);
        }

        // 只有当 READ1 和 READ2 都匹配到同一个样本时，才认为是正确的读对
        if (sample1 != "Unknown" && sample1 == sample2) {
            read_pair.sample_name = sample1;
            stats.sample_counts[read_pair.sample_name]++;
            if (corrected1 || corrected2){
                read_pair.corrected = true;
                stats.corrected_counts[read_pair.sample_name]++;
            }

            // 推入队列
            while (!queue.push(read_pair)) {
                std::this_thread::yield(); // 让出CPU，避免忙等
            }
        } else {
            unknownReads++;
        }

        totalReads++;
    }

    gzclose(infile1);
    gzclose(infile2);

    stats.total_reads += totalReads;
    stats.unknown_reads += unknownReads;

    std::cerr << "生产者完成，文件1: " << fq1_input << ", 文件2: " << fq2_input 
              << ", Total Reads: " << totalReads 
              << ", Unknown Reads: " << unknownReads << std::endl;

    finished.store(true, std::memory_order_release);
}

// 消费者函数实现
void consumer(SPSCQueue<ReadPair>& queue, 
              const std::string& output_dir,
              const std::vector<std::string>& samples,  
              std::atomic<bool>& finished) {
    size_t totalWritten = 0;

    // 使用 unique_ptr 存储 Writer 对象
    std::unordered_map<std::string, std::unique_ptr<Writer>> sample_to_file_R1;
    std::unordered_map<std::string, std::unique_ptr<Writer>> sample_to_file_R2;
    for (auto& sample: samples){
        // 构建输出文件名
        std::string filename_R1 = output_dir + "/" + sample + ".R1.fq.gz";
        std::string filename_R2 = output_dir + "/" + sample + ".R2.fq.gz";
        // 打开输出文件句柄
        sample_to_file_R1[sample] = std::make_unique<Writer>(filename_R1, 6);
        sample_to_file_R2[sample] = std::make_unique<Writer>(filename_R2, 6);
    }


    ReadPair read_pair;
    while (true) {
        if (queue.pop(read_pair)) {
            // 获取输出文件句柄
            auto& outfile_R1 = sample_to_file_R1[read_pair.sample_name];
            auto& outfile_R2 = sample_to_file_R2[read_pair.sample_name];

            // 写入 READ1 的四行
            if (!outfile_R1->writeString(read_pair.read1_line1.c_str()) || 
                !outfile_R1->writeString(read_pair.read1_line2.c_str()) ||
                !outfile_R1->writeString(read_pair.read1_line3.c_str()) ||
                !outfile_R1->writeString(read_pair.read1_line4.c_str())){
                std::cerr << "写入 READ1 到文件失败，样本: " << read_pair.sample_name << std::endl;
                continue;
            }

            // 写入 READ2 的四行
            if (!outfile_R2->writeString(read_pair.read2_line1.c_str()) || 
                !outfile_R2->writeString(read_pair.read2_line2.c_str()) ||
                !outfile_R2->writeString(read_pair.read2_line3.c_str()) ||
                !outfile_R2->writeString(read_pair.read2_line4.c_str())){
                std::cerr << "写入 READ1 到文件失败，样本: " << read_pair.sample_name << std::endl;
                continue;
            }

            totalWritten++;
        } else {
            if (finished.load(std::memory_order_acquire) && !queue.pop(read_pair)) {
                break; // 生产者已完成且队列为空
            }
            std::this_thread::yield(); // 让出CPU，避免忙等
        }
    }

    // 关闭所有输出文件, 无需手动flush, Writer析构函数会自动调用
    std::cerr << "消费者完成，Total Written Read Pairs: " << totalWritten << std::endl;
}

