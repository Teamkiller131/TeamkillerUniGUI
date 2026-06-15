#pragma once

#include <unigui/widgets/sparkline.h>
#include <unigui/widgets/statuslamp.h>
#include <unigui/widgets/widget_base.h>

#include <string>

namespace unigui {

/// ConnectionStatusBar — a compact link-health strip that bundles a connection
/// lamp, an adaptive latency readout (µs/ms, colour-graded by warn/crit
/// thresholds), an optional FPS, a reconnect countdown, and an optional embedded
/// latency trend sparkline. Composes the existing `StatusLamp` + `Sparkline`
/// rather than reimplementing a ring buffer — the RTT averaging and reconnect
/// FSM stay in the caller's session layer; this widget only displays the values.
class ConnectionStatusBar : public FluentWidget<ConnectionStatusBar> {
public:
    explicit ConnectionStatusBar(std::string name);

    void Render() override;

    ConnectionStatusBar& WithConnected(bool on) {
        connected_ = on;
        return *this;
    }
    ConnectionStatusBar& WithCaption(std::string c) {
        caption_ = std::move(c);
        return *this;
    }
    /// Current latency in microseconds; `avgUs < 0` hides the average.
    ConnectionStatusBar& WithLatencyUs(double currentUs, double avgUs = -1.0) {
        latencyUs_ = currentUs;
        avgUs_ = avgUs;
        return *this;
    }
    /// Warn/crit latency thresholds (µs) for colour grading.
    ConnectionStatusBar& WithLatencyThresholds(double warnUs, double critUs) {
        warnUs_ = warnUs;
        critUs_ = critUs;
        return *this;
    }
    ConnectionStatusBar& WithFps(float fps) {
        fps_ = fps;
        return *this;
    }
    /// Reconnect countdown in seconds (< 0 hides it).
    ConnectionStatusBar& WithReconnectIn(double seconds) {
        reconnectIn_ = seconds;
        return *this;
    }
    /// Show an inline latency-history sparkline; feed it via PushLatencySample().
    ConnectionStatusBar& WithSparkline(bool on) {
        showSpark_ = on;
        return *this;
    }
    /// Append a latency sample (µs) to the embedded sparkline (rolling, capped).
    void PushLatencySample(double us);

private:
    bool connected_ = false;
    std::string caption_;
    double latencyUs_ = 0.0;
    double avgUs_ = -1.0;
    double warnUs_ = 2000.0;  // 2ms
    double critUs_ = 10000.0; // 10ms
    float fps_ = -1.f;
    double reconnectIn_ = -1.0;
    bool showSpark_ = false;
    StatusLamp lamp_;
    Sparkline spark_;
};

} // namespace unigui
