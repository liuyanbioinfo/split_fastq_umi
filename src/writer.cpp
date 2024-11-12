#include "writer.h"
#include "util.h"
#include <string.h>
#include <stdexcept>
#include <iostream>

Writer::Writer(const string& filename, int compression) {
    mCompression = compression;
    mFilename = filename;
    mCompressor = NULL;
    mZipped = false;
    haveToClose = true;
    mBuffer = NULL;
    mBufDataLen = 0;
    mBufSize = 3*1024*1024;
    mFP = NULL;
    try {
        init();
    } catch (const std::exception& e) {
        cerr << "Error initializing Writer: " << e.what() << endl;
        throw;  // Re-throw the exception to be handled by the caller
    }
}

Writer::~Writer() {
    try {
        flush();
        if (haveToClose) {
            close();
        }
    } catch (const std::exception& e) {
        cerr << "Error during destruction: " << e.what() << endl;
    }
}

void Writer::flush() {
    if (mBufDataLen > 0) {
        if (!writeInternal(mBuffer, mBufDataLen)) {
            cerr << "Error flushing data to file." << endl;
        }
        mBufDataLen = 0;
    }
}

string Writer::filename() const {
    return mFilename;
}

void Writer::init() {
    // Allocate memory for the buffer
    mBuffer = static_cast<char*>(malloc(mBufSize));
    if (!mBuffer) {
        throw std::runtime_error("Failed to allocate write buffer with size: " + std::to_string(mBufSize));
    }

    // Open the file for writing (binary mode)
    if (!openFile(mFilename)) {
        throw std::runtime_error("Failed to open file for writing: " + mFilename);
    }

    // Initialize the compressor if file is .gz
    if (ends_with(mFilename, ".gz")) {
        mCompressor = libdeflate_alloc_compressor(mCompression);
        if (!mCompressor) {
            throw std::runtime_error("Failed to allocate libdeflate compressor.");
        }
        mZipped = true;
    }
}

bool Writer::openFile(const string& filename) {
    mFP = fopen(filename.c_str(), "wb");
    return mFP != NULL;
}

bool Writer::writeString(const string& str) {
    return write(str.data(), str.length());
}

bool Writer::writeString(string* str) {
    return write(str->data(), str->length());
}

bool Writer::write(const char* strdata, size_t size) {
    if (size + mBufDataLen > mBufSize) {
        flush();
    }
    if (size > mBufSize) {
        return writeInternal(strdata, size);
    } else {
        memcpy(mBuffer + mBufDataLen, strdata, size);
        mBufDataLen += size;
    }
    return true;
}

bool Writer::writeInternal(const char* strdata, size_t size) {
    bool status = false;

    try {
        if (mZipped) {
            size_t bound = libdeflate_gzip_compress_bound(mCompressor, size);
            void* out = malloc(bound);
            if (!out) {
                throw std::runtime_error("Failed to allocate memory for compressed output.");
            }

            size_t outsize = libdeflate_gzip_compress(mCompressor, strdata, size, out, bound);
            if (outsize > 0) {
                size_t ret = fwrite(out, 1, outsize, mFP);
                status = (ret > 0);
            }
            free(out);
        } else {
            size_t ret = fwrite(strdata, 1, size, mFP);
            status = (ret > 0);
        }
    } catch (const std::exception& e) {
        cerr << "Error during writeInternal: " << e.what() << endl;
    }
    return status;
}

void Writer::close() {
    if (mZipped && mCompressor) {
        if (ftell(mFP) == 0) {
            cerr << "Warning: Empty file " << mFilename << std::endl;
            writeInternal("", 0); // Ensure complete gzip file structure for empty files
        }
        libdeflate_free_compressor(mCompressor);
        mCompressor = NULL;
    }

    if (mBuffer) {
        free(mBuffer);
        mBuffer = NULL;
    }

    if (mFP) {
        fclose(mFP);
        mFP = NULL;
    }
}

bool Writer::isZipped() const {
    return mZipped;
}
