#pragma once

#include <iostream>
#include <string>

class Buffer {
private:
    std::string buf_;
    const u_int16_t sep_;  // 报文的分隔符：0-无分隔符（固定长度，视频会议）1-四字节的分隔符，2-"\r\n\r\n"分隔符（HTTP）

public:
    Buffer(u_int16_t sep = 1);

    ~Buffer();
    void Append(const char* data, size_t size);
    void AppendWithSep(const char* data, size_t size);
    void Erase(size_t pos, size_t n);
    size_t size();
    const char* data();
    void Clear();
    bool PickMessage(std::string& msg);  // 从buf_中拆分出一个报文，存在在msg中，如果buf_中没有报文，返回false
};