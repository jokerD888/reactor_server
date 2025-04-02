#include "buffer.h"
Buffer::Buffer() {}
Buffer ::~Buffer() {}
void Buffer::Append(const char* data, size_t size) { buf_.append(data, size); }
size_t Buffer::size() { return buf_.size(); }
const char* Buffer::data() { return buf_.data(); }
void Buffer::Clear() { buf_.clear(); }

void Buffer::Erase(size_t pos, size_t n) { buf_.erase(pos, n); }
