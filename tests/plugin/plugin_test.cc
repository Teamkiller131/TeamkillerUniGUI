#include <unigui/plugin/plugin_manager.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>
using namespace unigui::plugin;

class PluginTest : public ::testing::Test {
protected:
    void TearDown() override { Manager::Instance().Shutdown(); }
};

TEST_F(PluginTest, List_EmptyByDefault) {
    EXPECT_TRUE(Manager::Instance().List().empty());
}

TEST_F(PluginTest, Load_Nonexistent_ReturnsNull) {
    auto* p = Manager::Instance().Load("nonexistent.dll");
    EXPECT_EQ(p, nullptr);
}

TEST_F(PluginTest, Register_Builtin_Works) {
    class TestPlugin : public IPlugin {
        PluginInfo GetInfo() const override { return {"Test", "1.0"}; }
        bool Init() override { return true; }
        void Shutdown() override {}
    };
    auto* p = Manager::Instance().Register(new TestPlugin());
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->GetInfo().name, "Test");
    EXPECT_FALSE(Manager::Instance().List().empty());
}

TEST_F(PluginTest, Unload_RemovesPlugin) {
    class TestPlugin : public IPlugin {
        PluginInfo GetInfo() const override { return {"T2", "1.0"}; }
        bool Init() override { return true; }
        void Shutdown() override {}
    };
    Manager::Instance().Register(new TestPlugin());
    EXPECT_TRUE(Manager::Instance().Unload("T2"));
    EXPECT_TRUE(Manager::Instance().List().empty());
}

// ── Reload (DLL-lifetime pointer swap) — the classic stale-handle UAF surface ──

TEST_F(PluginTest, Reload_Unknown_ReturnsNull) {
    EXPECT_EQ(Manager::Instance().Reload("does-not-exist"), nullptr);
}

TEST_F(PluginTest, Reload_BuiltinWithoutPath_ReturnsNull) {
    class TestPlugin : public IPlugin {
        PluginInfo GetInfo() const override { return {"RT", "1.0"}; }
        bool Init() override { return true; }
        void Shutdown() override {}
    };
    Manager::Instance().Register(new TestPlugin());
    // Register leaves the DLL path empty, so Reload has nothing to reload and must
    // short-circuit to nullptr — never attempting to FreeLibrary a built-in plugin.
    EXPECT_EQ(Manager::Instance().Reload("RT"), nullptr);
    // The built-in is still registered and usable after the no-op reload.
    ASSERT_NE(Manager::Instance().Get("RT"), nullptr);
}
