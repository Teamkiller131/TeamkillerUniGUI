#include <unigui/unigui.h>
#include <unigui/v2/plugin_manager.h>
#include <gtest/gtest.h>
using namespace unigui::v2;

class PluginTest : public ::testing::Test {
protected:
    void TearDown() override { PluginManager::Instance().Shutdown(); }
};

TEST_F(PluginTest, List_EmptyByDefault) {
    EXPECT_TRUE(PluginManager::Instance().List().empty());
}

TEST_F(PluginTest, Load_Nonexistent_ReturnsNull) {
    auto* p = PluginManager::Instance().Load("nonexistent.dll");
    EXPECT_EQ(p, nullptr);
}

TEST_F(PluginTest, Register_Builtin_Works) {
    class TestPlugin : public IPlugin {
        PluginInfo GetInfo() const override { return {"Test","1.0"}; }
        bool Init() override { return true; }
        void Shutdown() override {}
    };
    auto* p = PluginManager::Instance().Register(new TestPlugin());
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->GetInfo().name, "Test");
    EXPECT_FALSE(PluginManager::Instance().List().empty());
}

TEST_F(PluginTest, Unload_RemovesPlugin) {
    class TestPlugin : public IPlugin {
        PluginInfo GetInfo() const override { return {"T2","1.0"}; }
        bool Init() override { return true; }
        void Shutdown() override {}
    };
    PluginManager::Instance().Register(new TestPlugin());
    EXPECT_TRUE(PluginManager::Instance().Unload("T2"));
    EXPECT_TRUE(PluginManager::Instance().List().empty());
}
