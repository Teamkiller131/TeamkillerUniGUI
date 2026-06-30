// Platform screen-reader bridge for unigui::a11y. `InstallSystemBridge()` routes focus
// changes + live announcements to the OS assistive-technology runtime:
//
//   • Windows    — UI Automation notification events (Narrator / NVDA / JAWS).
//   • Web (wasm) — an ARIA live region in the page DOM (any browser screen reader).
//   • macOS      — NSAccessibility announcement notifications (VoiceOver).
//   • Linux      — AT-SPI2 Announcement events over the a11y D-Bus (Orca), opt-in via
//                  -DUNIGUI_A11Y_ATSPI=ON; otherwise the logging fallback.
//
// Notifications are fire-and-forget, so no full element tree is exposed (which
// immediate-mode ImGui can't cheaply provide). All branches enable a11y and subscribe to
// the same SetOnFocusChanged / SetOnAnnounce callbacks.
#include <unigui/core/accessibility.h>
#include <unigui/core/log.h>

#include <string>

// ─────────────────────────────────────────────────────────────────────────────
#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <oleauto.h>      // SysAllocString / SysFreeString
#include <uiautomation.h> // IRawElementProviderSimple, UiaRaiseNotificationEvent, …

namespace unigui::a11y {
namespace {

// Minimal server-side UIA provider for the app window — enough to raise notification
// events. We delegate the host (HWND) provider to UIA and answer only the control type.
class WindowProvider : public IRawElementProviderSimple {
public:
    explicit WindowProvider(HWND h)
            : hwnd_(h) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv)
            return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IRawElementProviderSimple)) {
            *ppv = static_cast<IRawElementProviderSimple*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&ref_); }
    ULONG STDMETHODCALLTYPE Release() override {
        const LONG r = InterlockedDecrement(&ref_);
        if (r == 0)
            delete this;
        return static_cast<ULONG>(r);
    }
    HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions* opts) override {
        if (!opts)
            return E_POINTER;
        *opts = ProviderOptions_ServerSideProvider;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID, IUnknown** ret) override {
        if (ret)
            *ret = nullptr;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID id, VARIANT* ret) override {
        if (!ret)
            return E_POINTER;
        ret->vt = VT_EMPTY;
        if (id == UIA_ControlTypePropertyId) {
            ret->vt = VT_I4;
            ret->lVal = UIA_WindowControlTypeId;
        }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(IRawElementProviderSimple** ret) override {
        if (!ret)
            return E_POINTER;
        return UiaHostProviderFromHwnd(hwnd_, ret);
    }

    void SetHwnd(HWND h) { hwnd_ = h; } // re-point after a window recreate

private:
    LONG ref_ = 1;
    HWND hwnd_ = nullptr;
};

WindowProvider* g_provider = nullptr; // process-lifetime singleton (intentional)

std::wstring ToWide(const std::string& s) {
    if (s.empty())
        return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int) s.size(), nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int) s.size(), w.data(), n);
    return w;
}

void Raise(const std::wstring& text, NotificationProcessing processing) {
    if (!g_provider || text.empty() || !UiaClientsAreListening())
        return;
    BSTR display = SysAllocString(text.c_str());
    BSTR activity = SysAllocString(L"unigui");
    UiaRaiseNotificationEvent(g_provider, NotificationKind_Other, processing, display, activity);
    SysFreeString(display);
    SysFreeString(activity);
}

} // namespace

void InstallSystemBridge(void* hwnd) {
    SetEnabled(true);
    if (!hwnd) {
        InstallLoggingBridge();
        return;
    }
    if (!g_provider)
        g_provider = new WindowProvider(static_cast<HWND>(hwnd));
    else
        g_provider->SetHwnd(static_cast<HWND>(hwnd)); // re-point after a window recreate
    SetOnFocusChanged([](const Node& n) {
        if (n.role == Role::Unknown && n.name.empty())
            return;
        std::string text = std::string(RoleName(n.role)) + " " + n.name;
        if (!n.value.empty())
            text += ", " + n.value;
        Raise(ToWide(text), NotificationProcessing_MostRecent);
    });
    SetOnAnnounce([](const Announcement& a) {
        Raise(ToWide(a.message), a.politeness == Live::Assertive
                                     ? NotificationProcessing_ImportantMostRecent
                                     : NotificationProcessing_All);
    });
    UNIGUI_LOG_INFO("[a11y] Windows UI Automation bridge installed");
}

} // namespace unigui::a11y

// ─────────────────────────────────────────────────────────────────────────────
#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

namespace unigui::a11y {
namespace {
// Mirror focus/announcements into hidden ARIA live regions in the page; any browser
// screen reader (VoiceOver, NVDA, Narrator, Orca, TalkBack) reads them.
void SpeakWeb(const std::string& text, bool assertive) {
    if (text.empty())
        return;
    // clang-format off
    EM_ASM({
        var id = $1 ? 'unigui-a11y-assertive' : 'unigui-a11y-polite';
        var d = document.getElementById(id);
        if (!d) {
            d = document.createElement('div');
            d.id = id;
            d.setAttribute('aria-live', $1 ? 'assertive' : 'polite');
            d.setAttribute('aria-atomic', 'true');
            d.style.cssText = 'position:absolute;width:1px;height:1px;overflow:hidden;' +
                              'clip:rect(0 0 0 0);clip-path:inset(50%);white-space:nowrap;border:0;';
            document.body.appendChild(d);
        }
        d.textContent = UTF8ToString($0);
    }, text.c_str(), assertive ? 1 : 0);
    // clang-format on
}
} // namespace

void InstallSystemBridge(void* /*hwnd*/) {
    SetEnabled(true);
    SetOnFocusChanged([](const Node& n) {
        if (n.role == Role::Unknown && n.name.empty())
            return;
        std::string text = std::string(RoleName(n.role)) + " " + n.name;
        if (!n.value.empty())
            text += ", " + n.value;
        SpeakWeb(text, /*assertive=*/false);
    });
    SetOnAnnounce(
        [](const Announcement& a) { SpeakWeb(a.message, a.politeness == Live::Assertive); });
    UNIGUI_LOG_INFO("[a11y] Web ARIA live-region bridge installed");
}

} // namespace unigui::a11y

// ─────────────────────────────────────────────────────────────────────────────
#elif defined(__APPLE__)

#import <AppKit/AppKit.h>

namespace unigui::a11y {
namespace {
NSWindow* g_window = nil;
void Speak(const std::string& text, bool assertive) {
    if (text.empty() || g_window == nil)
        return;
    NSString* msg = [NSString stringWithUTF8String:text.c_str()];
    NSDictionary* info = @{
        NSAccessibilityAnnouncementKey : (msg ? msg : @""),
        NSAccessibilityPriorityKey :
            @(assertive ? NSAccessibilityPriorityHigh : NSAccessibilityPriorityMedium)
    };
    NSAccessibilityPostNotificationWithUserInfo(
        g_window, NSAccessibilityAnnouncementRequestedNotification, info);
}
} // namespace

void InstallSystemBridge(void* nsWindow) {
    SetEnabled(true);
    if (!nsWindow) {
        InstallLoggingBridge();
        return;
    }
    g_window = (__bridge NSWindow*) nsWindow;
    SetOnFocusChanged([](const Node& n) {
        if (n.role == Role::Unknown && n.name.empty())
            return;
        std::string text = std::string(RoleName(n.role)) + " " + n.name;
        if (!n.value.empty())
            text += ", " + n.value;
        Speak(text, /*assertive=*/false);
    });
    SetOnAnnounce(
        [](const Announcement& a) { Speak(a.message, a.politeness == Live::Assertive); });
    UNIGUI_LOG_INFO("[a11y] macOS NSAccessibility bridge installed");
}

} // namespace unigui::a11y

// ─────────────────────────────────────────────────────────────────────────────
#elif defined(UNIGUI_HAS_ATSPI) // Linux AT-SPI (opt-in, -DUNIGUI_A11Y_ATSPI=ON)

#include <gio/gio.h>

namespace unigui::a11y {
namespace {
GDBusConnection* g_a11yBus = nullptr;
bool g_tried = false;

// The a11y D-Bus is a private bus whose address is published on the session bus by
// org.a11y.Bus. Connect once (lazily); if anything fails (no a11y stack running), the
// bridge silently no-ops — exactly when no screen reader would be listening.
GDBusConnection* A11yBus() {
    if (g_tried)
        return g_a11yBus;
    g_tried = true;
    GError* err = nullptr;
    GDBusConnection* session = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
    if (!session) {
        g_clear_error(&err);
        return nullptr;
    }
    GVariant* res = g_dbus_connection_call_sync(
        session, "org.a11y.Bus", "/org/a11y/bus", "org.a11y.Bus", "GetAddress", nullptr,
        G_VARIANT_TYPE("(s)"), G_DBUS_CALL_FLAGS_NONE, 500, nullptr, &err);
    g_object_unref(session);
    if (!res) {
        g_clear_error(&err);
        return nullptr;
    }
    const char* addr = nullptr;
    g_variant_get(res, "(&s)", &addr);
    g_a11yBus = g_dbus_connection_new_for_address_sync(
        addr,
        static_cast<GDBusConnectionFlags>(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                          G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
        nullptr, nullptr, &err);
    g_variant_unref(res);
    g_clear_error(&err);
    return g_a11yBus;
}

// Emit an AT-SPI "Announcement" event. Every org.a11y.atspi.Event.* signal body is
// (siiva{sv}): detail(s), detail1(i) (politeness: 1=polite, 2=assertive), detail2(i),
// any_data(v) (the text, as a variant), and an a{sv} properties dict (empty here). The
// sender is NOT in the body — the D-Bus daemon adds the unique name to the message header.
// This mirrors GTK's gtk_at_spi_context_announce(); using (so) made the signal undecodable
// by Orca/atk-bridge.
void EmitAnnouncement(const std::string& text, int politeness) {
    if (text.empty())
        return;
    GDBusConnection* bus = A11yBus();
    if (!bus)
        return;
    GVariant* body = g_variant_new("(siiva{sv})", "", politeness, 0,
                                   g_variant_new_string(text.c_str()), nullptr);
    g_dbus_connection_emit_signal(bus, nullptr, "/org/a11y/atspi/accessible/root",
                                  "org.a11y.atspi.Event.Object", "Announcement", body, nullptr);
}
} // namespace

void InstallSystemBridge(void* /*nativeWindowHandle*/) {
    SetEnabled(true);
    SetOnFocusChanged([](const Node& n) {
        if (n.role == Role::Unknown && n.name.empty())
            return;
        std::string text = std::string(RoleName(n.role)) + " " + n.name;
        if (!n.value.empty())
            text += ", " + n.value;
        EmitAnnouncement(text, /*polite=*/1);
    });
    SetOnAnnounce([](const Announcement& a) {
        EmitAnnouncement(a.message, a.politeness == Live::Assertive ? 2 : 1);
    });
    UNIGUI_LOG_INFO("[a11y] Linux AT-SPI announcement bridge installed");
}

} // namespace unigui::a11y

// ─────────────────────────────────────────────────────────────────────────────
#else // Linux / other — logging fallback (build with -DUNIGUI_A11Y_ATSPI=ON for AT-SPI)

namespace unigui::a11y {
void InstallSystemBridge(void* /*nativeWindowHandle*/) {
    // No native AT bridge compiled in; the logging bridge is the reference implementation.
    InstallLoggingBridge();
}
} // namespace unigui::a11y

#endif
