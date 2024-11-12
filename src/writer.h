#ifndef _WRITER_H
#define _WRITER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "libdeflate.h"
#include <stdio.h>

using namespace std;

class Writer{
public:
        Writer(const string& filename, int compression);
        ~Writer();
        bool isZipped() const;
        bool writeString(const string& str);
        bool writeString(string* str);
        bool write(const char* strdata, size_t size);
        void flush();
        string filename() const;
        bool openFile(const string& filename);

public:
        static bool test();

private:
        void init();
        void close();
        bool writeInternal(const char* strdata, size_t size);

private:
        string mFilename;
        libdeflate_compressor* mCompressor;
        FILE* mFP;
        bool mZipped;
        int mCompression;
        bool haveToClose;
        char* mBuffer;
        size_t mBufDataLen;
        size_t mBufSize;
};
#endif
