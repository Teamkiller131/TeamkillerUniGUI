#include <unigui/ipc/shmem.h>

#include <gtest/gtest.h>
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
