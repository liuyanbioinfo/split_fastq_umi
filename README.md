# SPLITE_FASTQ_UMI
## 项目简介
该项目旨在处理带有条形码（UMI）标记的双端测序数据（FASTQ格式）文件。通过双端UMI匹配样本名称，将读取的双端read（Read1和Read2）按照样本及UMI对应关系分类，输出到对应样本的结果文件。项目通过无锁的SPSC（Single Producer Single Consumer）队列实现多线程读写，提升了处理速度。

## 功能概述
+ 解析UMI和样本配置文件
+ 从压缩的FASTQ文件中读取双端reads
+ 使用UMI匹配样本名称
+ 将匹配的读对按照样本分类输出
+ 支持多线程生产和消费模型

## 使用说明
1. **编译项目**

项目使用 `conan`与`cmake` 进行构建。首先克隆并进入项目的根目录，执行以下命令：
```bash
git clone git@github.com:liuyanbioinfo/split_fastq_umi.git
cd split_fastq_umi
conan install . --output-folder=build --build=missing
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

成功编译后，生成的可执行文件将位于 `build/` 目录下。

2. **运行程序**

使用以下命令运行程序，使用示例数据：

```bash
cd testdata
../build/bin/split_fastq_umi -G barcodes.example.conf -S sample.example.conf -i example.R1.fq.gz -I example.R2.fq.gz -o ./ -p 7 > example.split.log 
```

3. **命令行参数**

| 参数 | 说明 |
| --- | --- |
| `-G` | UMI分组文件路径 |
| `-S` | 样本配置文件路径 |
| `-i` | 输入的 FASTQ Read1 文件 |
| `-I` | 输入的 FASTQ Read2 文件 |
| `-p` | UMI容错位置 |
| `-o` | 输出目录路径 |


4. **文件格式要求**
UMI分组文件格式：
```plain
<UMI分组> <UMI>
Group1 ACGTACGT
Group2 TGCTAGCT
```

样本配置文件格式：
```plain
<FC> <样本ID> <UMI> <样本名称>
FC001 Sample01 Group1,Group2 Sample01,Sample02
```

## 输出
程序会根据UMI匹配结果，将匹配成功的读对按照样本名称输出到指定的目录中。输出文件将命名为 `<样本名>.R1.fq.gz` 和 `<样本名>.R2.fq.gz`，分别对应 Read1 和 Read2 的输出。

## 贡献
欢迎提出问题和改进建议。你可以通过提交 Issue 或 Pull Request 的方式贡献代码。

## 许可协议
本项目遵循 MIT 许可协议。详情参见 `LICENSE` 文件。

