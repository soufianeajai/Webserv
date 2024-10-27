#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <cstring>

class Buffer {
private:
    static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024;
    static const size_t CHUNK_SIZE = 8 * 1024;
    std::vector<uint8_t> data;
    size_t size;

public:
    Buffer() : size(0) {
        data.reserve(MAX_BODY_SIZE);
    }

    void append(const uint8_t* newData, size_t readSize) {
        if (size + readSize > MAX_BODY_SIZE) {
            // Handle the error of the body is bigger than the limit size
        }
        data.insert(data.end(), newData, newData + readSize);
        size += readSize;
    }

    // Method to clear the buffer
    void clear() {
        data.clear();
        size = 0;
    }

    // Method to get the current size of the buffer
    size_t size() const {
        return size;
    }

    const std::vector<uint8_t>& getData() const {
        return data;
    }

    bool isFull() const {
        return size >= MAX_BODY_SIZE;
    }

    bool isEmpty() const {
        return size == 0;
    }
};
