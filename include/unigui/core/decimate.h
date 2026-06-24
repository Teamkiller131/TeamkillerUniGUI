#pragma once

#include <cstddef>
#include <vector>

namespace unigui {

// ─────────────────────────────────────────────────────────────────────────────
// Series decimation (namespace unigui)
//
// Downsampling for data-dense charts: a 100k+-point tick/spread series renders
// far faster (and reads no worse) when reduced to ~the pixel width of the plot
// before upload. Pure, header-only, ImGui-free — so it is trivially testable and
// usable by any chart path (e.g. TimeSeriesChart::SetMaxRenderPoints).
//
//   • LttbIndices / Lttb  — Largest-Triangle-Three-Buckets: preserves the visual
//     shape (peaks/troughs) of a line far better than naive every-Nth sampling.
//   • MinMaxBuckets       — per-bucket min+max: guarantees extremes survive
//     (better for volatile OHLC/price data where spikes must not vanish).
// ─────────────────────────────────────────────────────────────────────────────

/// LTTB: select ~`threshold` representative indices from an (xs,ys) series of
/// `n` points. The first and last points are always kept. Returns `0..n-1` when
/// `n <= threshold` or `threshold < 3`. Indices are returned in ascending order.
inline std::vector<std::size_t> LttbIndices(const double* xs, const double* ys, std::size_t n,
                                            std::size_t threshold) {
    std::vector<std::size_t> out;
    if (n == 0)
        return out;
    if (threshold < 3 || n <= threshold) {
        out.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            out.push_back(i);
        return out;
    }
    out.reserve(threshold);
    out.push_back(0); // always keep the first point

    // `threshold - 2` buckets span the interior points [1, n-1).
    const double every = static_cast<double>(n - 2) / static_cast<double>(threshold - 2);
    std::size_t a = 0; // index of the last selected point

    for (std::size_t i = 0; i < threshold - 2; ++i) {
        // Average point of the *next* bucket (used to form the triangle).
        std::size_t avgStart = static_cast<std::size_t>((i + 1) * every) + 1;
        std::size_t avgEnd = static_cast<std::size_t>((i + 2) * every) + 1;
        if (avgEnd > n)
            avgEnd = n;
        double avgX = 0.0, avgY = 0.0;
        const std::size_t avgCount = (avgEnd > avgStart) ? (avgEnd - avgStart) : 1;
        for (std::size_t j = avgStart; j < avgEnd; ++j) {
            avgX += xs[j];
            avgY += ys[j];
        }
        avgX /= static_cast<double>(avgCount);
        avgY /= static_cast<double>(avgCount);

        // Current bucket range.
        std::size_t rangeStart = static_cast<std::size_t>(i * every) + 1;
        std::size_t rangeEnd = static_cast<std::size_t>((i + 1) * every) + 1;
        if (rangeEnd > n - 1)
            rangeEnd = n - 1;

        // Pick the point in the bucket forming the largest triangle with `a`
        // (the last selected point) and the next bucket's average.
        const double ax = xs[a], ay = ys[a];
        double maxArea = -1.0;
        std::size_t best = rangeStart;
        for (std::size_t j = rangeStart; j < rangeEnd; ++j) {
            const double area =
                0.5 * (((ax - avgX) * (ys[j] - ay)) - ((ax - xs[j]) * (avgY - ay)));
            const double absArea = area < 0.0 ? -area : area;
            if (absArea > maxArea) {
                maxArea = absArea;
                best = j;
            }
        }
        out.push_back(best);
        a = best;
    }

    out.push_back(n - 1); // always keep the last point
    return out;
}

/// Convenience: write the LTTB-decimated series into `outX`/`outY`.
inline void Decimate(const std::vector<double>& xs, const std::vector<double>& ys,
                     std::size_t threshold, std::vector<double>& outX, std::vector<double>& outY) {
    outX.clear();
    outY.clear();
    const std::size_t n = xs.size() < ys.size() ? xs.size() : ys.size();
    const auto idx = LttbIndices(xs.data(), ys.data(), n, threshold);
    outX.reserve(idx.size());
    outY.reserve(idx.size());
    for (std::size_t i : idx) {
        outX.push_back(xs[i]);
        outY.push_back(ys[i]);
    }
}

/// Per-bucket min+max decimation: splits the series into `~buckets` ranges and
/// keeps the min and the max of each (in index order), so spikes never vanish.
/// Returns ascending, de-duplicated indices; `0..n-1` when n is already small.
inline std::vector<std::size_t> MinMaxBuckets(const double* ys, std::size_t n,
                                              std::size_t buckets) {
    std::vector<std::size_t> out;
    if (n == 0)
        return out;
    if (buckets == 0 || n <= buckets * 2) {
        out.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            out.push_back(i);
        return out;
    }
    const double step = static_cast<double>(n) / static_cast<double>(buckets);
    for (std::size_t b = 0; b < buckets; ++b) {
        std::size_t s = static_cast<std::size_t>(b * step);
        std::size_t e = static_cast<std::size_t>((b + 1) * step);
        if (e > n)
            e = n;
        if (s >= e)
            continue;
        std::size_t lo = s, hi = s;
        for (std::size_t j = s + 1; j < e; ++j) {
            if (ys[j] < ys[lo])
                lo = j;
            if (ys[j] > ys[hi])
                hi = j;
        }
        std::size_t first = lo < hi ? lo : hi;
        std::size_t second = lo < hi ? hi : lo;
        out.push_back(first);
        if (second != first)
            out.push_back(second);
    }
    return out;
}

} // namespace unigui
