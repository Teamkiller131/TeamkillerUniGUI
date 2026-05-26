#include <unigui/unigui.h>
#include <unigui/plugin/plugin_manager.h>
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
        PluginInfo GetInfo() const override { return {"Test","1.0"}; }
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
        PluginInfo GetInfo() const override { return {"T2","1.0"}; }
        bool Init() override { return true; }
        void Shutdown() override {}
    };
    Manager::Instance().Register(new TestPlugin());
    EXPECT_TRUE(Manager::Instance().Unload("T2"));
    EXPECT_TRUE(Manager::Instance().List().empty());
}
