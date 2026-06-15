#include <unigui/widgets/basketticket.h>

#include <imgui.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace unigui;

namespace {
struct BasketRow {
    std::string symbol;
    int lots = 0;
    double price = 0.0;
};
} // namespace

class BasketTicketTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1000, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }

    BasketTicket<BasketRow> MakeTicket() {
        return BasketTicket<BasketRow>("basket", {{"Symbol", 90}, {"Lots", 70}, {"Price", 80}});
    }
};

TEST_F(BasketTicketTest, AddAndRemoveRows) {
    auto t = MakeTicket();
    EXPECT_EQ(t.RowCount(), 0u);
    t.AddRow({"IF2506", 2, 3900.0});
    t.AddRow({"IC2506", 1, 5600.0});
    EXPECT_EQ(t.RowCount(), 2u);
    t.RemoveRow(0);
    EXPECT_EQ(t.RowCount(), 1u);
    EXPECT_EQ(t.Rows()[0].symbol, "IC2506");
}

TEST_F(BasketTicketTest, SetRows_ReplacesForImport) {
    auto t = MakeTicket();
    t.AddRow({"A", 1, 1.0});
    t.SetRows({{"X", 5, 10.0}, {"Y", 6, 20.0}, {"Z", 7, 30.0}}); // host-parsed import
    EXPECT_EQ(t.RowCount(), 3u);
    EXPECT_EQ(t.Rows()[1].symbol, "Y");
}

TEST_F(BasketTicketTest, Validator_CountsValidRows) {
    auto t = MakeTicket();
    t.SetValidator([](const BasketRow& r) { return !r.symbol.empty() && r.lots > 0; });
    t.SetRows({{"OK", 2, 1.0}, {"", 1, 1.0} /*bad sym*/, {"BAD", 0, 1.0} /*bad lots*/});
    EXPECT_EQ(t.RowCount(), 3u);
    EXPECT_EQ(t.ValidCount(), 1u);
    EXPECT_FALSE(t.AllValid());

    t.SetRows({{"A", 1, 1.0}, {"B", 2, 2.0}});
    EXPECT_TRUE(t.AllValid());
}

TEST_F(BasketTicketTest, RowFactory_AndGridConfig_Render) {
    auto t = MakeTicket();
    t.SetRowFactory([] { return BasketRow{"NEW", 1, 0.0}; });
    t.Grid().SetIntColumn(
        1, [](int, const BasketRow& r) { return r.lots; },
        [](int, int) { /* host writes back */ });
    t.SetRows({{"IF2506", 2, 3900.0}});
    EXPECT_NO_THROW(t.Render());
}

TEST_F(BasketTicketTest, Submit_OnlyWhenAllValid) {
    auto t = MakeTicket();
    bool submitted = false;
    t.SetValidator([](const BasketRow& r) { return r.lots > 0; });
    t.SetOnSubmit([&](const std::vector<BasketRow>&) { submitted = true; });
    t.SetRows({{"A", 0, 1.0}}); // invalid → submit disabled
    EXPECT_FALSE(t.AllValid());
    EXPECT_NO_THROW(t.Render());
    EXPECT_FALSE(submitted); // disabled button never fires
}

TEST_F(BasketTicketTest, Empty_RendersWithoutCrash) {
    auto t = MakeTicket();
    t.SetOnImportRequested([] {});
    EXPECT_NO_THROW(t.Render());
}
