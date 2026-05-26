#include <unigui/ipc/shmem.h>
#include <unigui/core/log.h>
namespace unigui::ipc {
SharedMemory::SharedMemory(const std::string& name, size_t size) : size_(size), name_(name) {
#ifdef _WIN32
    handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, (DWORD)size, name.c_str());
    if (handle_) data_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size);
#else
    fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd_ >= 0) { ftruncate(fd_, size); data_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0); }
#endif
    if (data_) UNIGUI_LOG_INFO("SHM: {} ({} bytes)", name, (int)size);
}
SharedMemory::~SharedMemory() {
    if (data_) {
#ifdef _WIN32
        UnmapViewOfFile(data_); if (handle_) CloseHandle(handle_);
#else
        munmap(data_, size_); if (fd_>=0) { close(fd_); shm_unlink(name_.c_str()); }
#endif
    }
}
void SharedMemory::Write(const void* d, size_t s, size_t off) { if (data_ && off+s <= size_) std::memcpy((char*)data_+off, d, s); }
void SharedMemory::Read(void* d, size_t s, size_t off) { if (data_ && off+s <= size_) std::memcpy(d, (char*)data_+off, s); }
}
