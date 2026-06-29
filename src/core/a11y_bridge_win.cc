// Platform screen-reader bridge for unigui::a11y.
//
// On Windows it raises UI Automation **notification events** (spoken by Narrator / NVDA /
// JAWS) for focus changes and live announcements. Notifications are fire-and-forget, so
// this only needs a minimal UIA provider for the app window — it does NOT expose a full
// element tree (which immediate-mode ImGui can't cheaply provide). On every other
// platform InstallSystemBridge() falls back to the reference logging bridge.
#include <unigui/core/accessibility.h>
#include <unigui/core/log.h>

#include <string>

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

    // IUnknown
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

    // IRawElementProviderSimple
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

private:
    LONG ref_ = 1;
    HWND hwnd_ = nullptr;
};

// Process-lifetime singleton (intentional one-time leak; the window outlives it).
WindowProvider* g_provider = nullptr;

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
        return; // no screen reader listening — skip the work
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

    SetOnFocusChanged([](const Node& n) {
        if (n.role == Role::Unknown && n.name.empty())
            return; // focus cleared — nothing to speak
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

#else // !_WIN32

namespace unigui::a11y {
void InstallSystemBridge(void* /*nativeWindowHandle*/) {
    // No native AT bridge on this platform yet — the logging bridge is the reference
    // implementation a real NSAccessibility / AT-SPI / web-ARIA bridge would replace.
    InstallLoggingBridge();
}
} // namespace unigui::a11y

#endif
