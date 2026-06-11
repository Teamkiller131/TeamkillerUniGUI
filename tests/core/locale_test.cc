#include <unigui/core/locale.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>
class LocaleTest : public ::testing::Test {
protected:
    void TearDown() override { unigui::Locale::Clear(); }
};
TEST_F(LocaleTest, DefaultLocaleIsEnUS) {
    EXPECT_EQ(unigui::Locale::GetCurrent(), "en_US");
}
TEST_F(LocaleTest, SetAndTranslate) {
    unigui::Locale::Set("en_US", "hello", "Hello");
    unigui::Locale::Set("zh_CN", "hello", "\344\275\240\345\245\275");
    unigui::Locale::SetCurrent("zh_CN");
    EXPECT_EQ(unigui::Locale::Tr("hello"), "\344\275\240\345\245\275");
    EXPECT_TRUE(unigui::Locale::Has("hello"));
}
