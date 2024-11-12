# Normal File
../bin/split_fastq_umi -G barcodes.example.conf -S sample.example.conf -i example.R1.fq.gz -I example.R2.fq.gz -o ./ -p 7 > example.split.log 

# Empty File
../bin/split_fastq_umi -G barcodes.empty.conf  -S sample.empty.conf -i empty_1.fq.gz -I empty_2.fq.gz  -o ./ -p 7 >empty.split.log 
