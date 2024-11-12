# SPLIT_FASTQ_UMI
## 项目简介
本项目旨在拆分和校正带有条形码（UMI）标记的双端测序数据（FASTQ格式）文件。通过双端UMI匹配样本，将读取的双端read（Read1和Read2）按照样本及UMI对应关系拆分，输出到对应样本的结果文件。项目通过无锁的SPSC（Single Producer Single Consumer）队列实现多线程读写，提升了处理速度。

## 功能概述
+ 解析UMI和样本配置文件，自动获取UMI长度
+ 从压缩的FASTQ文件中读取双端reads
+ 使用UMI匹配样本
+ 校正UMI指定位置（-p）碱基错误
+ 修复MGI测序ID列，去除/1, /2字符，避免下游分析不兼容
+ 将匹配的READ按照样本分组输出

## 使用说明
1. **编译项目**

项目使用 `conan`与`cmake` 进行构建。执行以下命令：
```bash
git clone git@github.com:liuyanbioinfo/split_fastq_umi.git
cd split_fastq_umi
conan install . --output-folder=build --build=missing 
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
# 修改DCMAKE_INSTALL_PREFIX`/path/to/split_fastq_umi/`参数至当前项目路径或则其他安装路径
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/to/split_fastq_umi/
cmake --build .
cmake --install .
```

conan安装依赖包需要能正常下载"https://github.com/"的资源, 否则依赖包需手动安装配置。成功编译后，生成的可执行文件默认位于 `/path/to/split_fastq_umi/bin` 目录下。

2. **运行程序**

使用以下命令运行程序，使用示例数据：

```bash
cd testdata
../bin/split_fastq_umi -G barcodes.example.conf -S sample.example.conf -i example.R1.fq.gz -I example.R2.fq.gz -o ./ -p 7 > example.split.log 
```

3. **命令行参数**

| 参数 | 说明 |
| --- | --- |
| `-G` | UMI分组文件路径 |
| `-S` | 样本配置文件路径 |
| `-i` | 输入的 FASTQ Read1 文件 |
| `-I` | 输入的 FASTQ Read2 文件 |
| `-o` | 输出目录路径 |
| `-p` | UMI容错位置 （1-based位置计数，支持多个位置容错，逗号分隔）|
| `--disable_fix_mgi_id` | 关闭MGI ID行处理（不建议）|


4. **文件格式要求**

UMI分组文件格式：
```plain
<UMI分组> <UMI序列>
Group1 ACGTACGT
Group2 TGCTAGCT
```

样本配置文件格式：
```plain
<FC> <文库ID> <UMI> <样本名称>
FC1 Lib1 Group1,Group2 Sample1,Sample2
```

## 输出
程序会根据UMI序列匹配样本，将匹配成功的READS按照样本名称区分并输出到指定的目录中。输出文件将命名为 `<样本名>.R1.fq.gz` 和 `<样本名>.R2.fq.gz`，分别对应 Read1 和 Read2 的输出。


## 许可协议
本项目遵循 MIT 许可协议。详情参见 `LICENSE` 文件。

