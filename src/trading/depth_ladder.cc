#include <unigui/core/format_num.h>
#include <unigui/trading/depth_ladder.h>

#include <imgui.h>

#include <algorithm>
#include <vector>

namespace unigui::trading {

DepthLadder::DepthLadder(std::string name)
        : FluentWidget(std::move(name)) {}

void DepthLadder::SetDepth(int levels) {
    depth_ = levels < 0 ? 0 : levels;
}

void DepthLadder::SetRowHeight(float h) {
    rowH_ = std::clamp(h, 8.0f, 64.0f);
}

void DepthLadder::SetSizeColumnWidth(float w) {
    sizeColW_ = w < 0.0f ? 0.0f : w;
}

void DepthLadder::SetPriceColumnWidth(float w) {
    priceColW_ = std::clamp(w, 24.0f, 256.0f);
}

void DepthLadder::SetBarOpacity(float a) {
    barOpacity_ = std::clamp(a, 0.0f, 1.0f);
}

void DepthLadder::SetPriceDecimals(int d) {
    priceDecimals_ = std::clamp(d, 0, 8);
}

ImU32 DepthLadder::BarColor(ImU32 base) const {
    ImVec4 c = ImGui::ColorConvertU32ToFloat4(base);
    c.w = barOpacity_;
    return ImGui::ColorConvertFloat4ToU32(c);
}

// ─────────────────────────────────────────────────────────────────────────────
// One ladder level: [ bid-size | price | ask-size ]. A full-width invisible
// button captures hover/click; the depth bar grows outward from the price
// column toward the populated side; text is aligned toward the price column.
// ─────────────────────────────────────────────────────────────────────────────
bool DepthLadder::DrawLevelRow(int rowIndex, Side side, const Level& level, std::int64_t maxSize,
                               float totalW) {
    ImGui::PushID(rowIndex);
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("row", ImVec2(totalW, rowH_));
    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();
    ImGui::PopID();

    ImDrawList* dl = ImGui::GetWindowDrawList();

    const float sizeColW =
        sizeColW_ > 0.0f ? sizeColW_ : std::max(8.0f, (totalW - priceColW_) * 0.5f);
    const float bidX0 = p.x;
    const float priceX0 = bidX0 + sizeColW;
    const float askX0 = priceX0 + priceColW_;
    const float yMid = p.y + rowH_ * 0.5f;

    if (hovered)
        dl->AddRectFilled(p, ImVec2(p.x + totalW, p.y + rowH_),
                          ImGui::GetColorU32(ImGuiCol_HeaderHovered));

    const float frac =
        maxSize > 0
            ? static_cast<float>(static_cast<double>(level.size) / static_cast<double>(maxSize))
            : 0.0f;

    const std::string sizeText = format::Thousands(static_cast<long long>(level.size));
    const std::string priceText = format::Fixed(level.price, priceDecimals_);
    const ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);
    const float pad = 4.0f;

    if (side == Side::Bid) {
        // Bar grows leftward from the price column edge.
        const float barW = frac * (sizeColW - pad);
        dl->AddRectFilled(ImVec2(priceX0 - barW, p.y), ImVec2(priceX0, p.y + rowH_),
                          BarColor(bid_));
        // Size text right-aligned against the price column.
        const ImVec2 ts = ImGui::CalcTextSize(sizeText.c_str());
        dl->AddText(ImVec2(priceX0 - pad - ts.x, yMid - ts.y * 0.5f), textCol, sizeText.c_str());
    } else {
        // Bar grows rightward from the price column edge.
        const float barW = frac * (sizeColW - pad);
        dl->AddRectFilled(ImVec2(askX0, p.y), ImVec2(askX0 + barW, p.y + rowH_), BarColor(ask_));
        // Size text left-aligned against the price column.
        const ImVec2 ts = ImGui::CalcTextSize(sizeText.c_str());
        dl->AddText(ImVec2(askX0 + pad, yMid - ts.y * 0.5f), textCol, sizeText.c_str());
    }

    // Price centred in the middle column, tinted by side.
    const ImVec2 pts = ImGui::CalcTextSize(priceText.c_str());
    const ImU32 priceCol = (side == Side::Bid) ? bid_ : ask_;
    dl->AddText(ImVec2(priceX0 + (priceColW_ - pts.x) * 0.5f, yMid - pts.y * 0.5f), priceCol,
                priceText.c_str());

    return clicked;
}

void DepthLadder::DrawSpreadRow(float totalW) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(totalW, rowH_));
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const ImU32 sep = ImGui::GetColorU32(ImGuiCol_Separator);
    dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + totalW, p.y), sep);
    dl->AddLine(ImVec2(p.x, p.y + rowH_), ImVec2(p.x + totalW, p.y + rowH_), sep);

    const double spread = book_->Spread();
    const double mid = book_->Mid();
    std::string text;
    if (book_->HasBids() && book_->HasAsks())
        text = format::Fixed(spread, priceDecimals_) + "  |  " + format::Fixed(mid, priceDecimals_);
    else
        text = "—";
    const ImVec2 ts = ImGui::CalcTextSize(text.c_str());
    dl->AddText(ImVec2(p.x + (totalW - ts.x) * 0.5f, p.y + (rowH_ - ts.y) * 0.5f),
                ImGui::GetColorU32(ImGuiCol_TextDisabled), text.c_str());
}

void DepthLadder::Render() {
    if (!IsVisible() || book_ == nullptr)
        return;

    ImGui::PushID(GetName().c_str());

    int pushedColors = 0;
    if (themeBackground_) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        pushedColors = 1;
    }

    const ImGuiChildFlags childFlags = border_ ? ImGuiChildFlags_Borders : ImGuiChildFlags_None;
    if (ImGui::BeginChild("##ladder", size_, childFlags)) {
        const float totalW = ImGui::GetContentRegionAvail().x;

        // Asks: book gives low→high; render highest→lowest (best ask nearest the
        // spread row), so reverse.
        std::vector<Level> asks = book_->Asks(depth_);
        std::reverse(asks.begin(), asks.end());
        std::vector<Level> bids = book_->Bids(depth_); // already high→low

        // Derive maxSize from the level vectors already built above, rather than
        // calling book_->MaxSize(depth_) which rebuilds the same Bids()/Asks()
        // vectors (two extra heap allocations + map traversals) every frame.
        std::int64_t maxSize = 0;
        for (const auto& l : asks)
            maxSize = std::max(maxSize, l.size);
        for (const auto& l : bids)
            maxSize = std::max(maxSize, l.size);

        // Announce a click-to-trade level pick (fires only on interaction, so no
        // per-frame cost; the a11y check inside Announce makes it a no-op when off).
        auto announceLevel = [this](Side side, const Level& lvl) {
            a11y::Announce((side == Side::Ask ? "Ask " : "Bid ") +
                           format::Fixed(lvl.price, priceDecimals_) + " x " +
                           format::Thousands(static_cast<long long>(lvl.size)) + " clicked");
        };

        int rowIndex = 0;
        for (const auto& lvl : asks) {
            if (DrawLevelRow(rowIndex++, Side::Ask, lvl, maxSize, totalW) && onLevelClick_) {
                announceLevel(Side::Ask, lvl);
                onLevelClick_(Side::Ask, lvl.price, lvl.size);
            }
        }

        // Record the inside-market Y for optional centring.
        const float boundaryY = ImGui::GetCursorPosY() + (showSpread_ ? rowH_ * 0.5f : 0.0f);

        if (showSpread_)
            DrawSpreadRow(totalW);

        for (const auto& lvl : bids) {
            if (DrawLevelRow(rowIndex++, Side::Bid, lvl, maxSize, totalW) && onLevelClick_) {
                announceLevel(Side::Bid, lvl);
                onLevelClick_(Side::Bid, lvl.price, lvl.size);
            }
        }

        if (autoCenter_ || centerRequested_) {
            const float target = boundaryY - ImGui::GetWindowSize().y * 0.5f;
            ImGui::SetScrollY(std::max(0.0f, target));
            centerRequested_ = false;
        }
    }
    ImGui::EndChild();

    // Register the ladder with the inside market as its value — what a screen-reader
    // user needs from a DOM at a glance. IsEnabled() keeps the high-refresh render
    // loop allocation-free when a11y is off.
    if (a11y::IsEnabled()) {
        std::string v;
        if (book_->HasBids() || book_->HasAsks()) {
            v = "bid " + format::Fixed(book_->BestBid(), priceDecimals_) + " x " +
                format::Thousands(static_cast<long long>(book_->BestBidSize())) + ", ask " +
                format::Fixed(book_->BestAsk(), priceDecimals_) + " x " +
                format::Thousands(static_cast<long long>(book_->BestAskSize())) + ", spread " +
                format::Fixed(book_->Spread(), priceDecimals_);
        } else {
            v = "empty book";
        }
        ReportAccessible(a11y::Role::Table, ImGui::IsItemFocused(), v);
    }

    if (pushedColors > 0)
        ImGui::PopStyleColor(pushedColors);
    ImGui::PopID();
}

} // namespace unigui::trading
