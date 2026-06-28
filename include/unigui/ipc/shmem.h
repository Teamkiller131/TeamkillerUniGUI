#pragma once
// The OS mapping headers (<windows.h> / <sys/mman.h> …) are implementation details
// used in src/ipc/shmem.cc; the class members are all void*/int, so they are not
// needed here, and keeping them out spares every IPC consumer the heavy include.
#include <cstddef>
#include <string>

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
