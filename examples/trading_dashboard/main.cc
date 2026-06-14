// ─────────────────────────────────────────────────────────────────────────────
// trading_dashboard — assembles the UniGUI trading toolkit (Horizon 3) into one
// runnable screen: a candlestick chart, a depth-of-market ladder, an order
// ticket, and the positions / orders / watchlist / time-&-sales blotters, all
// driven by small in-memory models with synthetic, self-animating data.
//
// Presentation-only: there is no real market feed or OMS — a deterministic
// pseudo-random walk stands in for both so the demo is reproducible and runs
// headless via `--frames N` (for screenshots / CI smoke).
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/unigui.h>

#include <implot.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

using namespace unigui::trading;

namespace {

// Tiny deterministic LCG so the demo animates identically every run.
struct Rng {
    std::uint64_t s = 0x2545F4914F6CDD1Dull;
    double Next() { // [0,1)
        s = s * 6364136223846793005ull + 1442695040888963407ull;
        return static_cast<double>(s >> 11) / static_cast<double>(1ull << 53);
    }
    double Range(double a, double b) { return a + (b - a) * Next(); }
};

} // namespace

int main(int argc, char** argv) {
    int maxFrames = 0;
    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            maxFrames = std::atoi(argv[++i]);
    }

    unigui::AppConfig cfg;
    cfg.width = 1440;
    cfg.height = 900;
    cfg.title = "UniGUI — Trading Dashboard";
    if (!unigui::Init(cfg)) {
        std::fprintf(stderr, "Init failed\n");
        return 1;
    }
    ImPlot::CreateContext();

    Rng rng;
    const std::string kSymbol = "ACME";

    // ── Models ───────────────────────────────────────────────────────────────
    OhlcSeries series(/*interval*/ 60.0, /*maxBars*/ 180);
    double px = 100.0;
    for (int i = 0; i < 120; ++i) {
        const double t = i * 60.0;
        px += rng.Range(-0.8, 0.8);
        series.AddTick(t, px);                        // open
        series.AddTick(t + 15, px + rng.Range(0, 1)); // high-ish
        series.AddTick(t + 30, px - rng.Range(0, 1)); // low-ish
        series.AddTick(t + 45, px + rng.Range(-0.5, 0.5),
                       static_cast<std::int64_t>(rng.Range(100, 900)));
    }
    double last = series.Empty() ? 100.0 : series.Back().close;

    OrderBook book;
    auto rebuildBook = [&] {
        book.Clear();
        const double tick = 0.05;
        for (int i = 1; i <= 12; ++i) {
            book.SetBid(last - i * tick, static_cast<std::int64_t>(rng.Range(50, 1500)));
            book.SetAsk(last + i * tick, static_cast<std::int64_t>(rng.Range(50, 1500)));
        }
    };
    rebuildBook();

    std::vector<Position> positions{
        {kSymbol, 500, 98.40, last},
        {"GLOB", -200, 53.10, 51.90},
        {"NOVA", 1200, 12.05, 12.66},
    };
    std::vector<Order> orders{
        {"1001", kSymbol, Side::Buy, 99.50, 500, 500, OrderStatus::Filled},
        {"1002", "GLOB", Side::Sell, 53.00, 200, 120, OrderStatus::PartiallyFilled},
    };
    std::vector<Trade> tape;
    std::vector<Quote> watch{
        {kSymbol, last - 0.05, last + 0.05, last, 99.10, 0, 0, 1'200'000},
        {"GLOB", 51.88, 51.92, 51.90, 52.40, 0, 0, 540'000},
        {"NOVA", 12.65, 12.67, 12.66, 12.10, 0, 0, 880'000},
    };

    // ── Widgets ──────────────────────────────────────────────────────────────
    CandlestickChart chart("chart");
    chart.WithSeries(&series).WithVolumePanel(true).WithCrosshair(true);

    DepthLadder dom("dom");
    dom.WithBook(&book).WithDepth(12).WithAutoCenter(true);

    int nextOrderId = 1003;
    OrderTicket ticket("ticket");
    ticket.WithSymbol(kSymbol)
        .WithOrderType(OrderType::Limit)
        .WithQuantity(100)
        .WithPrice(last)
        .WithTickSize(0.05)
        .WithConfirm(true)
        .WithOnSubmit([&](const OrderDraft& o) {
            orders.push_back({std::to_string(nextOrderId++), o.symbol, o.side, o.price, o.qty, 0,
                              OrderStatus::New});
            tape.push_back({std::to_string(nextOrderId), o.symbol, o.side, o.price, o.qty,
                            static_cast<double>(maxFrames)});
        });

    auto positionsBlotter = MakePositionsBlotter("positions");
    positionsBlotter.SetDataSource(&positions);
    auto ordersBlotter = MakeOrdersBlotter("orders");
    ordersBlotter.SetDataSource(&orders);
    auto tradesTape = MakeTradesTape("tape");
    tradesTape.SetDataSource(&tape);
    auto watchlist = MakeWatchlist("watch");
    watchlist.SetDataSource(&watch);

    // ── Loop ─────────────────────────────────────────────────────────────────
    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

        // Animate: advance the last price, fold it into the series, refresh the
        // book + top-of-book quote.
        last += rng.Range(-0.15, 0.15);
        series.AddTick(120 * 60.0 + frame, last, static_cast<std::int64_t>(rng.Range(50, 400)));
        rebuildBook();
        watch[0].bid = book.BestBid();
        watch[0].ask = book.BestAsk();
        watch[0].last = last;
        positions[0].last = last;

        ImGui::SetNextWindowSize(ImVec2(760, 520), ImGuiCond_FirstUseEver);
        ImGui::Begin("Chart");
        chart.Render();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(300, 520), ImGuiCond_FirstUseEver);
        ImGui::Begin("Depth of Market");
        dom.Render();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(320, 520), ImGuiCond_FirstUseEver);
        ImGui::Begin("Order Ticket");
        ticket.Render();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(560, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin("Positions");
        positionsBlotter.Render();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(560, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin("Watchlist");
        watchlist.Render();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(560, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin("Orders");
        ordersBlotter.Render();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(560, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin("Time & Sales");
        tradesTape.Render();
        ImGui::End();

        unigui::Render();
        ++frame;
        if (maxFrames > 0 && frame >= maxFrames)
            break;
    }

    unigui::Shutdown();
    ImPlot::DestroyContext();
    std::printf("[trading_dashboard] Done. %d frames.\n", frame);
    return 0;
}
