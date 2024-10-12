// include/ProducerConsumer.h
#ifndef PRODUCERCONSUMER_H
#define PRODUCERCONSUMER_H

#include "ReadPair.h"
#include "SPSCQueue.h"
#include "BarcodeMatcher.h"
#include "Statistics.h"
#include <string>
#include <unordered_map>
#include <zlib.h>
#include <atomic>

// 生产者函数：读取FASTQ文件，匹配条形码，推入队列
void producer(SPSCQueue<ReadPair>& queue, 
              const std::string& fq1_input, 
              const std::string& fq2_input, 
              const BarcodeMatcher& matcher, 
              Statistics& stats, 
              std::atomic<bool>& finished);

// 消费者函数：从队列中弹出读对并写入到对应的输出文件
void consumer(SPSCQueue<ReadPair>& queue, 
              const std::string& output_dir, 
              std::unordered_map<std::string, gzFile>& sample_to_file_R1, 
              std::unordered_map<std::string, gzFile>& sample_to_file_R2, 
              std::atomic<bool>& finished);

#endif // PRODUCERCONSUMER_H

