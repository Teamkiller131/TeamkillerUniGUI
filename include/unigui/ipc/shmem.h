#pragma once
#include <string>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace unigui::ipc {

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size);
    ~SharedMemory();
    void Write(const void* data, size_t size, size_t offset = 0);
    void Read(void* data, size_t size, size_t offset = 0);
    size_t Size() const { return size_; }
    void* Data() { return data_; }
private:
#ifdef _WIN32
    void* handle_ = nullptr;
#else
    int fd_ = -1;
#endif
    void* data_ = nullptr;
    size_t size_ = 0;
    std::string name_;
};

} // namespace unigui::ipc
