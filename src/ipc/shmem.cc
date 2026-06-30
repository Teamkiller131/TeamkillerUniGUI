#include <unigui/core/log.h>
#include <unigui/ipc/shmem.h>

#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace unigui::ipc {
SharedMemory::SharedMemory(const std::string& name, size_t size)
        : size_(size)
        , name_(name) {
#ifdef _WIN32
    handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, (DWORD) size,
                                 name.c_str());
    if (handle_)
        data_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size);
#else
    fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd_ >= 0) {
        // Size the object before mapping. If ftruncate fails, mapping would expose a region
        // whose backing file is shorter than size_ — touching past the real end raises
        // SIGBUS, bypassing the size_-based bounds check in Read/Write. Fail closed.
        if (ftruncate(fd_, static_cast<off_t>(size)) == 0) {
            void* p = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            // mmap signals failure with MAP_FAILED ((void*)-1), NOT nullptr — so a failed
            // mmap would otherwise pass every `if (data_)` guard and memcpy through a wild
            // pointer. Only adopt the mapping when it actually succeeded.
            if (p != MAP_FAILED)
                data_ = p;
        }
        if (!data_) { // ftruncate or mmap failed: release the fd, leave data_ == nullptr
            close(fd_);
            shm_unlink(name_.c_str());
            fd_ = -1;
        }
    }
#endif
    if (data_)
        UNIGUI_LOG_INFO("SHM: {} ({} bytes)", name, (int) size);
}
SharedMemory::~SharedMemory() {
    // Release each resource on its OWN existence test, not gated on data_. If the mapping
    // view failed (data_ == nullptr) but the file-mapping HANDLE / fd was created, gating
    // the whole cleanup on `if (data_)` would leak the HANDLE/fd (and the named POSIX shm
    // object) for the process lifetime.
#ifdef _WIN32
    if (data_)
        UnmapViewOfFile(data_);
    if (handle_)
        CloseHandle(handle_);
#else
    if (data_)
        munmap(data_, size_);
    if (fd_ >= 0) {
        close(fd_);
        shm_unlink(name_.c_str());
    }
#endif
}
void SharedMemory::Write(const void* d, size_t s, size_t off) {
    // Overflow-safe bounds check (see ipc.cc): `off + s` would wrap in unsigned
    // arithmetic, so test the offset first then the size against remaining space.
    if (data_ && off <= size_ && s <= size_ - off)
        std::memcpy((char*) data_ + off, d, s);
}
void SharedMemory::Read(void* d, size_t s, size_t off) {
    if (data_ && off <= size_ && s <= size_ - off)
        std::memcpy(d, (char*) data_ + off, s);
}
} // namespace unigui::ipc
