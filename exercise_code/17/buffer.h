#pragma once

#include <iostream>
#include <string>

class Buffer {
private:
    std::string buf_;

public:
    Buffer();
    ~Buffer();
    void Append(const char* data, size_t size);
    void Erase(size_t pos, size_t n);
    size_t size();
    const char* data();
    void Clear();
};