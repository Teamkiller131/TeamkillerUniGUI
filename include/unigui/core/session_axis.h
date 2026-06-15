#pragma once

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace unigui {

/// SessionAxis — maps intraday wall-clock time onto a continuous "session" axis
/// that collapses non-trading gaps (lunch break, pre/post-market), and back.
///
/// Intraday charts otherwise hand-wire an epoch→session-second transform plus an
/// inverse tick formatter in every strategy tab, so a flat-time X axis doesn't
/// leave dead space across the lunch break. SessionAxis is that transform, made
/// reusable and pure (no ImGui, no global state) so it is fully unit-testable.
///
/// Times are seconds-of-day (0..86399). Construct with ordered, non-overlapping
/// trading spans; `ToAxis`/`FromAxis` convert between wall-clock seconds and a
/// gap-free axis coordinate in [0, TotalSeconds()].
class SessionAxis {
public:
    struct Span {
        int startSec;
        int endSec;
    }; // [startSec, endSec)

    explicit SessionAxis(std::vector<Span> spans) : spans_(std::move(spans)) {
        std::sort(spans_.begin(), spans_.end(),
                  [](const Span& a, const Span& b) { return a.startSec < b.startSec; });
        int cum = 0;
        cum_.reserve(spans_.size());
        for (const auto& s : spans_) {
            cum_.push_back(cum);
            cum += std::max(0, s.endSec - s.startSec);
        }
        total_ = cum;
    }

    /// China A-share index-futures day session: 09:30–11:30 and 13:00–15:00.
    static SessionAxis AShareFutures() {
        return SessionAxis({{9 * 3600 + 30 * 60, 11 * 3600 + 30 * 60},
                            {13 * 3600, 15 * 3600}});
    }

    int TotalSeconds() const { return total_; }
    const std::vector<Span>& Spans() const { return spans_; }

    /// Wall-clock seconds-of-day → continuous axis position in [0, Total].
    /// Before the first span → 0; after the last → Total; inside a gap → snapped
    /// to the end of the preceding span.
    double ToAxis(int secOfDay) const {
        if (spans_.empty())
            return 0.0;
        if (secOfDay <= spans_.front().startSec)
            return 0.0;
        if (secOfDay >= spans_.back().endSec)
            return static_cast<double>(total_);
        for (std::size_t i = 0; i < spans_.size(); ++i) {
            const Span& s = spans_[i];
            if (secOfDay < s.startSec) // in the gap before span i → end of span i-1
                return static_cast<double>(cum_[i]);
            if (secOfDay < s.endSec)
                return static_cast<double>(cum_[i] + (secOfDay - s.startSec));
        }
        return static_cast<double>(total_);
    }

    /// Inverse: axis position → wall-clock seconds-of-day (clamped to [0,Total]).
    int FromAxis(double axisPos) const {
        if (spans_.empty())
            return 0;
        if (axisPos <= 0.0)
            return spans_.front().startSec;
        if (axisPos >= total_)
            return spans_.back().endSec;
        const int a = static_cast<int>(axisPos);
        for (std::size_t i = 0; i < spans_.size(); ++i) {
            const int spanLen = spans_[i].endSec - spans_[i].startSec;
            if (a < cum_[i] + spanLen)
                return spans_[i].startSec + (a - cum_[i]);
        }
        return spans_.back().endSec;
    }

    /// Format an axis position as "HH:MM" for a chart tick label.
    std::string FormatAxis(double axisPos) const {
        const int t = FromAxis(axisPos);
        const int hh = (t / 3600) % 24;
        const int mm = (t / 60) % 60;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%02d:%02d", hh, mm);
        return buf;
    }

private:
    std::vector<Span> spans_;
    std::vector<int> cum_; // axis offset at the start of each span
    int total_ = 0;
};

} // namespace unigui
