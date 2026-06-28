#include <unigui/ipc/shmem.h>

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>
using namespace unigui::ipc;
TEST(ShmemTest, WriteRead) {
    SharedMemory shm("ts", 1024);
    ASSERT_NE(shm.Data(), nullptr);
    const char* m = "hello";
    shm.Write(m, 6, 0);
    char b[16] = {};
    shm.Read(b, 6, 0);
    EXPECT_STREQ(b, "hello");
}
TEST(ShmemTest, LargeSize) {
    SharedMemory shm("tl", 65536);
    ASSERT_NE(shm.Data(), nullptr);
    EXPECT_EQ(shm.Size(), 65536u);
}

// ── Overflow-safe bounds check: a wrapped offset/size must not defeat the guard and
//    memcpy outside the mapped view. The old `off + s <= size_` wraps in unsigned
//    arithmetic (offset == SIZE_MAX → 0 <= size_), an OOB write across an IPC peer. ──
TEST(ShmemTest, OverflowOffset_IsNoOp) {
    SharedMemory shm("tovf", 64);
    ASSERT_NE(shm.Data(), nullptr);

    // Fill the region with a canary.
    char canary[64];
    std::memset(canary, 'A', sizeof(canary));
    shm.Write(canary, sizeof(canary), 0);

    // Malicious writes: offset == SIZE_MAX/size 1, and offset 0/size == SIZE_MAX.
    // Both wrap under the old check; both must now be no-ops.
    char poison = 'Z';
    shm.Write(&poison, 1, SIZE_MAX);
    shm.Write(&poison, SIZE_MAX, 0);

    // The mapped region is untouched.
    char buf[64] = {};
    shm.Read(buf, sizeof(buf), 0);
    for (char c : buf)
        EXPECT_EQ(c, 'A');

    // An out-of-range Read leaves its destination buffer untouched.
    char dst[8];
    std::memset(dst, 'B', sizeof(dst));
    shm.Read(dst, sizeof(dst), SIZE_MAX);
    for (char c : dst)
        EXPECT_EQ(c, 'B');
}
