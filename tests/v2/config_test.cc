#include <unigui/config/config.h>

#include <gtest/gtest.h>
using namespace unigui::config;
class StoreTest : public ::testing::Test {
protected:
    void TearDown() override { Store::Instance().Clear(); }
};
TEST_F(StoreTest, SetGet_String) {
    Store::Instance().SetString("k", "v");
    EXPECT_EQ(Store::Instance().GetString("k"), "v");
}
TEST_F(StoreTest, SetGet_Int) {
    Store::Instance().SetInt("n", 42);
    EXPECT_EQ(Store::Instance().GetInt("n"), 42);
}
TEST_F(StoreTest, Has_Works) {
    Store::Instance().SetInt("a", 1);
    EXPECT_TRUE(Store::Instance().Has("a"));
}
