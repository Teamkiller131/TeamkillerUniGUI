#include <unigui/v2/config.h>
#include <gtest/gtest.h>
using namespace unigui::v2;
class ConfigTest:public ::testing::Test{protected:void TearDown()override{Config::Instance().Clear();}};
TEST_F(ConfigTest,SetGet_String){Config::Instance().SetString("k","v");EXPECT_EQ(Config::Instance().GetString("k"),"v");}
TEST_F(ConfigTest,SetGet_Int){Config::Instance().SetInt("n",42);EXPECT_EQ(Config::Instance().GetInt("n"),42);}
TEST_F(ConfigTest,Has_Works){Config::Instance().SetInt("a",1);EXPECT_TRUE(Config::Instance().Has("a"));}
