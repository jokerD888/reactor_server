#include "buffer.h"
Buffer::Buffer(u_int16_t sep) : sep_(sep) {}
Buffer ::~Buffer() {}
void Buffer::Append(const char* data, size_t size) { buf_.append(data, size); }
size_t Buffer::size() { return buf_.size(); }
const char* Buffer::data() { return buf_.data(); }
void Buffer::Clear() { buf_.clear(); }

void Buffer::Erase(size_t pos, size_t n) { buf_.erase(pos, n); }

void Buffer::AppendWithSep(const char* data, size_t size) {
    if (sep_ == 0) {
        buf_.append(data, size);

    } else if (sep_ == 1) {
        buf_.append((char*)&size, 4);
        buf_.append(data, size);
    } else {
        printf("TODO...\n");
    }
}

bool Buffer::PickMessage(std::string& msg) {
    if (buf_.empty()) return false;
    if (sep_ == 0) {
        msg = buf_;
        buf_.clear();
        return true;
    } else if (sep_ == 1) {
        if (buf_.size() < 4) return false;
        size_t size = *(u_int32_t*)buf_.data();
        if (buf_.size() < size + 4) return false;
        msg = buf_.substr(4, size);
        buf_.erase(0, size + 4);
        return true;
    } else {
        printf("TODO...\n");
    }
    return false;
}