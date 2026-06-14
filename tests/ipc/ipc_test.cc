#include <unigui/ipc/ipc.h>
#include <unigui/ipc/shmem.h>

#include <cstring>
#include <gtest/gtest.h>
using namespace unigui::ipc;

TEST(IPCTest, SharedMemory_WriteRead) {
    SharedMemory shm("test_shm", 1024);
    ASSERT_NE(shm.Data(), nullptr);
    const char* msg = "hello";
    shm.Write(msg, 6, 0);
    char buf[16] = {};
    shm.Read(buf, 6, 0);
    EXPECT_STREQ(buf, "hello");
}

TEST(IPCTest, SharedMemory_LargeSize) {
    SharedMemory shm("test_large", 65536);
    ASSERT_NE(shm.Data(), nullptr);
    EXPECT_EQ(shm.Size(), 65536u);
}
