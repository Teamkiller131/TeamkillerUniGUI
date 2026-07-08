#include <unigui/widgets/futuresriskbar.h>

#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace unigui {

FuturesRiskBar::FuturesRiskBar(std::string name)
        : FluentWidget<FuturesRiskBar>(std::move(name)) {}

void FuturesRiskBar::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const float barHeight = 30.0f;
    float barWidth = ImGui::GetContentRegionAvail().x;
    if (barWidth < 1.0f)
        barWidth = 1.0f;

    // ── Top row: account name (left, bold white) + margin text (right, gray) ──
    {
        ImGui::PushFont(nullptr); // TODO: replace with bold font when available
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", accountName_.c_str());
        ImGui::PopFont();

        if (!marginText_.empty()) {
            ImGui::SameLine();
            float textW = ImGui::CalcTextSize(marginText_.c_str()).x;
            float availW = ImGui::GetContentRegionAvail().x;
            if (textW < availW) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availW - textW);
            }
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", marginText_.c_str());
        }
    }

    // ── Progress bar ──────────────────────────────────────────────────────
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size(barWidth, barHeight);

        // Invisible button for interaction region
        ImGui::InvisibleButton((GetName() + "##futuresriskbar").c_str(), size);

        auto* dl = ImGui::GetWindowDrawList();

        // ── Dark background ────────────────────────────────────────────
        const ImU32 bgColor = IM_COL32(0x2a, 0x2a, 0x2e, 0xff);
        const float rounding = ImGui::GetStyle().FrameRounding;
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, rounding);

        // ── Animation lerp ─────────────────────────────────────────────
        const float targetActual =
            static_cast<float>(std::max(0.0, std::min(actualRatio_, 1.0))) * barWidth;
        const float targetEst =
            static_cast<float>(std::max(0.0, std::min(estimatedRatio_, 1.0))) * barWidth;
        const float targetOvernight =
            static_cast<float>(std::max(0.0, std::min(overnightRatio_, 1.0))) * barWidth;

        if (animated_) {
            const float lerpSpeed = 0.1f;
            animActual_ = animActual_ + (targetActual - animActual_) * lerpSpeed;
            animEst_ = animEst_ + (targetEst - animEst_) * lerpSpeed;
            animOvernight_ = animOvernight_ + (targetOvernight - animOvernight_) * lerpSpeed;

            if (std::abs(animActual_ - targetActual) < 0.5f)
                animActual_ = targetActual;
            if (std::abs(animEst_ - targetEst) < 0.5f)
                animEst_ = targetEst;
            if (std::abs(animOvernight_ - targetOvernight) < 0.5f)
                animOvernight_ = targetOvernight;
        } else {
            animActual_ = targetActual;
            animEst_ = targetEst;
            animOvernight_ = targetOvernight;
        }

        const float fillW = std::max(0.0f, std::min(animActual_, barWidth));
        const float estX = pos.x + std::max(0.0f, std::min(animEst_, barWidth));
        const float overnightX = pos.x + std::max(0.0f, std::min(animOvernight_, barWidth));

        // ── Green fill (#28a745, opacity 0.72) ─────────────────────────
        if (fillW > 0.0f) {
            const ImU32 fillColor = IM_COL32(0x28, 0xa7, 0x45, 184); // 0.72 * 255 ≈ 184
            dl->AddRectFilled(pos, ImVec2(pos.x + fillW, pos.y + size.y), fillColor, rounding);
        }

        // ── Yellow solid line marker (3px wide) at estimatedRatio ──────
        if (animEst_ >= 0.0f && animEst_ <= barWidth) {
            const ImU32 yellowColor = IM_COL32(0xf0, 0xc0, 0x40, 0xff);
            const float halfThickness = 1.5f;
            dl->AddRectFilled(ImVec2(estX - halfThickness, pos.y),
                              ImVec2(estX + halfThickness, pos.y + size.y), yellowColor);
        }

        // ── Red dashed line at overnightRatio (7px dash, 4px gap) ──────
        if (animOvernight_ >= 0.0f && animOvernight_ <= barWidth) {
            const ImU32 redColor = IM_COL32(0xe9, 0x45, 0x60, 0xff);
            const float dashLen = 7.0f;
            const float gapLen = 4.0f;
            const float totalPattern = dashLen + gapLen;
            float y = pos.y;
            while (y < pos.y + size.y) {
                float yEnd = std::min(y + dashLen, pos.y + size.y);
                dl->AddLine(ImVec2(overnightX, y), ImVec2(overnightX, yEnd), redColor, 1.0f);
                y += totalPattern;
            }
        }

        // ── Border (thin outline) ───────────────────────────────────────
        const ImU32 borderColor = IM_COL32(0x55, 0x55, 0x58, 0xff);
        dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, rounding);
    }

    // ── Bottom legend ──────────────────────────────────────────────────────
    {
        const float legendHeight = 18.0f;
        const ImVec2 cursor = ImGui::GetCursorScreenPos();
        auto* dl = ImGui::GetWindowDrawList();

        // "● actual" — green dot + text
        {
            const char* text = "实际  ";
            const ImVec2 textSize = ImGui::CalcTextSize(text);
            const float dotRadius = 3.0f;
            const float dotX = cursor.x + dotRadius + 2.0f;
            const float dotY = cursor.y + legendHeight * 0.5f;
            const float textY = cursor.y + (legendHeight - textSize.y) * 0.5f;

            dl->AddCircleFilled(ImVec2(dotX, dotY), dotRadius, IM_COL32(0x28, 0xa7, 0x45, 0xff));
            dl->AddText(ImVec2(dotX + dotRadius + 4.0f, textY), IM_COL32(0xaa, 0xaa, 0xaa, 0xff),
                        text);
        }

        // "│ estimated" — yellow line marker + text
        {
            const char* text = "预估  ";
            const ImVec2 textSize = ImGui::CalcTextSize(text);
            const float prevEnd = cursor.x + ImGui::CalcTextSize("实际  ").x + 16.0f;
            const float lineX = prevEnd + 6.0f;
            const float textY = cursor.y + (legendHeight - textSize.y) * 0.5f;

            dl->AddLine(ImVec2(lineX, cursor.y + 2.0f),
                        ImVec2(lineX, cursor.y + legendHeight - 2.0f),
                        IM_COL32(0xf0, 0xc0, 0x40, 0xff), 2.0f);
            dl->AddText(ImVec2(lineX + 6.0f, textY), IM_COL32(0xaa, 0xaa, 0xaa, 0xff), text);
        }

        // "- - overnight" — red dashed line + text
        {
            const char* text = "隔夜";
            const ImVec2 textSize = ImGui::CalcTextSize(text);
            const float prevEnd = cursor.x + ImGui::CalcTextSize("实际  预估  ").x + 32.0f;
            const float dashX = prevEnd + 6.0f;
            const float textY = cursor.y + (legendHeight - textSize.y) * 0.5f;

            // Two short red dashes
            const ImU32 redColor = IM_COL32(0xe9, 0x45, 0x60, 0xff);
            const float dashY = cursor.y + legendHeight * 0.5f;
            dl->AddLine(ImVec2(dashX, dashY), ImVec2(dashX + 6.0f, dashY), redColor, 1.0f);
            dl->AddLine(ImVec2(dashX + 9.0f, dashY), ImVec2(dashX + 15.0f, dashY), redColor, 1.0f);

            dl->AddText(ImVec2(dashX + 19.0f, textY), IM_COL32(0xaa, 0xaa, 0xaa, 0xff), text);
        }

        ImGui::Dummy(ImVec2(barWidth, legendHeight));
    }

    ImGui::PopID();
}

void FuturesRiskBar::SetAccountName(std::string name) {
    accountName_ = std::move(name);
}
void FuturesRiskBar::SetMarginText(std::string text) {
    marginText_ = std::move(text);
}
void FuturesRiskBar::SetActualRatio(double r) {
    actualRatio_ = r;
}
void FuturesRiskBar::SetEstimatedRatio(double r) {
    estimatedRatio_ = r;
}
void FuturesRiskBar::SetOvernightRatio(double r) {
    overnightRatio_ = r;
}
void FuturesRiskBar::SetAnimated(bool on) {
    animated_ = on;
}

} // namespace unigui
