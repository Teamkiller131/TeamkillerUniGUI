#include <unigui/ipc/ipc.h>
#include <unigui/ipc/shmem.h>

#include <cstring>
#include <gtest/gtest.h>
#include <string>
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

// ── ZMQ PUB/SUB channel round-trip (was ZERO functional coverage) ─────────────
// inproc:// keeps it in-process + collision-free (no TCP port); Server binds
// before Client connects, as inproc requires.
TEST(IPCChannelTest, PubSub_RoundTrip) {
    const std::string addr = "inproc://unigui-ipc-roundtrip";
    Server server(addr);
    ASSERT_TRUE(server.Start());
    Client client(addr);
    ASSERT_TRUE(client.Connect());

    std::string got;
    client.OnReceive([&](const std::string& m) { got = m; });

    // PUB/SUB slow-joiner: the subscription may not be live for the first sends, so
    // publish repeatedly and poll until it arrives (or time out after ~2s).
    bool received = false;
    for (int i = 0; i < 200 && !received; ++i) {
        server.Send("hello");
        if (client.Poll(10) && got == "hello")
            received = true;
    }
    EXPECT_TRUE(received);
    EXPECT_EQ(got, "hello");

    client.Close();
    server.Close();
}

// Server is a PUB socket — OnReceive can never fire; it must be a documented no-op
// (and warn), not silently accept a dead handler.
TEST(IPCChannelTest, ServerOnReceive_IsNoOpNotCrash) {
    Server server("inproc://unigui-ipc-noop");
    EXPECT_NO_THROW(server.OnReceive([](const std::string&) {}));
}

// Shutdown() terminates the shared context; a channel created afterward gets a fresh
// one, so the module is reusable across a clean-shutdown cycle (no hang, no crash).
TEST(IPCChannelTest, Shutdown_ThenReuse_Works) {
    {
        Server s("inproc://unigui-ipc-sd");
        ASSERT_TRUE(s.Start());
        s.Close();
    } // channel closed + destroyed before terminating the context
    EXPECT_NO_THROW(Shutdown());

    Server s2("inproc://unigui-ipc-sd2");
    EXPECT_TRUE(s2.Start()); // fresh context created transparently
    s2.Close();
    EXPECT_NO_THROW(Shutdown());
}
