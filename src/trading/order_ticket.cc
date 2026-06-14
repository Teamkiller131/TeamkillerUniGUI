#include <unigui/core/format_num.h>
#include <unigui/core/strutil.h>
#include <unigui/trading/order_ticket.h>

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cstdio>

namespace unigui::trading {

OrderTicket::OrderTicket(std::string name)
        : FluentWidget(std::move(name)) {
    // Ctrl+Enter submits from anywhere in the ticket.
    hotkeys_.Register(ImGuiKey_Enter, /*ctrl*/ true, [this] { RequestSubmit(); }, "Submit order");
}

void OrderTicket::SetTickSize(double t) {
    tickSize_ = t; // <= 0 disables snapping; the input step falls back below
}

void OrderTicket::SetQuantityStep(std::int64_t step) {
    qtyStep_ = step < 1 ? 1 : step;
}

void OrderTicket::SetPriceDecimals(int d) {
    priceDecimals_ = std::clamp(d, 0, 8);
}

OrderValidation OrderTicket::Validate() const {
    if (Trim(draft_.symbol).empty())
        return {false, "Symbol is required"};
    if (draft_.qty <= 0)
        return {false, "Quantity must be greater than 0"};
    if (maxQty_ > 0 && draft_.qty > maxQty_)
        return {false, "Quantity exceeds the maximum"};
    if (draft_.NeedsPrice() && draft_.price <= 0.0)
        return {false, "Limit price must be greater than 0"};
    if (draft_.NeedsStop() && draft_.stopPrice <= 0.0)
        return {false, "Stop price must be greater than 0"};
    return {true, ""};
}

OrderValidation OrderTicket::Submit() {
    OrderValidation v = Validate();
    if (!v.ok)
        return v;
    if (tickSize_ > 0.0) {
        if (draft_.NeedsPrice())
            draft_.price = format::TickAlign(draft_.price, tickSize_);
        if (draft_.NeedsStop())
            draft_.stopPrice = format::TickAlign(draft_.stopPrice, tickSize_);
    }
    if (onSubmit_)
        onSubmit_(draft_);
    return v;
}

void OrderTicket::RequestSubmit() {
    if (!Validate().ok)
        return;
    if (confirm_) {
        confirmPending_ = true;
        ImGui::OpenPopup("##order-confirm");
    } else {
        Submit();
    }
}

// ── Sub-sections ─────────────────────────────────────────────────────────────

void OrderTicket::DrawSideToggle() {
    const ImU32 buyCol = IM_COL32(38, 166, 91, 255);
    const ImU32 sellCol = IM_COL32(217, 60, 60, 255);
    const float w = ImGui::GetContentRegionAvail().x;
    const float half = (w - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

    const bool isBuy = draft_.side == Side::Buy;
    // Buy half.
    ImGui::PushStyleColor(ImGuiCol_Button, isBuy ? buyCol : ImGui::GetColorU32(ImGuiCol_Button));
    if (ImGui::Button("Buy", ImVec2(half, 0)))
        draft_.side = Side::Buy;
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // Sell half.
    ImGui::PushStyleColor(ImGuiCol_Button, !isBuy ? sellCol : ImGui::GetColorU32(ImGuiCol_Button));
    if (ImGui::Button("Sell", ImVec2(half, 0)))
        draft_.side = Side::Sell;
    ImGui::PopStyleColor();
}

void OrderTicket::DrawTypeAndTif() {
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##type", OrderTypeName(draft_.type))) {
        for (auto t :
             {OrderType::Market, OrderType::Limit, OrderType::Stop, OrderType::StopLimit}) {
            if (ImGui::Selectable(OrderTypeName(t), draft_.type == t))
                draft_.type = t;
        }
        ImGui::EndCombo();
    }
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##tif", TimeInForceName(draft_.tif))) {
        for (auto t : {TimeInForce::Day, TimeInForce::GTC, TimeInForce::IOC, TimeInForce::FOK}) {
            if (ImGui::Selectable(TimeInForceName(t), draft_.tif == t))
                draft_.tif = t;
        }
        ImGui::EndCombo();
    }
}

void OrderTicket::DrawQtyAndPrice() {
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputScalar("##qty", ImGuiDataType_S64, &draft_.qty, &qtyStep_, nullptr, "%lld");

    const double step = tickSize_ > 0.0 ? tickSize_ : 0.01;
    char fmt[16];
    std::snprintf(fmt, sizeof(fmt), "%%.%df", priceDecimals_);

    if (!draft_.NeedsPrice())
        ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputScalar("##price", ImGuiDataType_Double, &draft_.price, &step, nullptr, fmt);
    if (!draft_.NeedsPrice())
        ImGui::EndDisabled();

    if (!draft_.NeedsStop())
        ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputScalar("##stop", ImGuiDataType_Double, &draft_.stopPrice, &step, nullptr, fmt);
    if (!draft_.NeedsStop())
        ImGui::EndDisabled();
}

// ── Render ───────────────────────────────────────────────────────────────────

void OrderTicket::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    if (ImGui::BeginChild("##ticket", size_,
                          ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {
        // Symbol.
        char symBuf[32];
        std::snprintf(symBuf, sizeof(symBuf), "%s", draft_.symbol.c_str());
        ImGui::TextUnformatted("Symbol");
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputTextWithHint("##symbol", "e.g. AAPL", symBuf, sizeof(symBuf)))
            draft_.symbol = symBuf;

        ImGui::Spacing();
        DrawSideToggle();
        ImGui::Spacing();

        ImGui::TextUnformatted("Type / TIF");
        DrawTypeAndTif();
        ImGui::Spacing();

        ImGui::TextUnformatted("Qty / Price / Stop");
        DrawQtyAndPrice();
        ImGui::Spacing();

        const OrderValidation v = Validate();
        if (!v.ok)
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(217, 60, 60, 255)), "%s",
                               v.message.c_str());

        if (!v.ok)
            ImGui::BeginDisabled();
        const std::string label =
            std::string(SideName(draft_.side)) + " " + OrderTypeName(draft_.type);
        if (ImGui::Button(label.c_str(), ImVec2(-FLT_MIN, 0)))
            RequestSubmit();
        if (!v.ok)
            ImGui::EndDisabled();

        if (hotkeySubmit_)
            hotkeys_.Process();

        // Confirmation modal.
        if (ImGui::BeginPopupModal("##order-confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s %s %s", SideName(draft_.side),
                        format::Thousands(static_cast<long long>(draft_.qty)).c_str(),
                        draft_.symbol.c_str());
            ImGui::Text("%s, TIF %s", OrderTypeName(draft_.type), TimeInForceName(draft_.tif));
            if (draft_.NeedsPrice())
                ImGui::Text("Limit %s", format::Fixed(draft_.price, priceDecimals_).c_str());
            if (draft_.NeedsStop())
                ImGui::Text("Stop %s", format::Fixed(draft_.stopPrice, priceDecimals_).c_str());
            ImGui::Separator();
            if (ImGui::Button("Confirm")) {
                Submit();
                confirmPending_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                confirmPending_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
}

} // namespace unigui::trading
