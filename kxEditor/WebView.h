#pragma once
#include "TextEditor.h"
#include "WebView2.h"

#include "WebTab.h"
#include <atlstr.h>
#include <cpprest/json.h>
#include <malloc.h>
#include <map>
#include <memory.h>
#include <memory>
#include <stdlib.h>
#include <strsafe.h>
#include <tchar.h>
#include <wil/com.h>
#include <wil/result.h>
#include <windows.h>
#include <wrl.h>

#define DEFAULT_DPI 96
#define MIN_WINDOW_WIDTH 510
#define MIN_WINDOW_HEIGHT 75

#define INVALID_TAB_ID 0
#define MG_NAVIGATE 1
#define MG_UPDATE_URI 2
#define MG_GO_FORWARD 3
#define MG_GO_BACK 4
#define MG_NAV_STARTING 5
#define MG_NAV_COMPLETED 6
#define MG_RELOAD 7
#define MG_CANCEL 8
#define MG_CREATE_TAB 10
#define MG_UPDATE_TAB 11
#define MG_SWITCH_TAB 12
#define MG_CLOSE_TAB 13
#define MG_CLOSE_WINDOW 14
#define MG_SHOW_OPTIONS 15
#define MG_HIDE_OPTIONS 16
#define MG_OPTIONS_LOST_FOCUS 17
#define MG_OPTION_SELECTED 18
#define MG_SECURITY_UPDATE 19
#define MG_UPDATE_FAVICON 20
#define MG_GET_SETTINGS 21
#define MG_GET_FAVORITES 22
#define MG_REMOVE_FAVORITE 23
#define MG_CLEAR_CACHE 24
#define MG_CLEAR_COOKIES 25
#define MG_GET_HISTORY 26
#define MG_REMOVE_HISTORY_ITEM 27
#define MG_CLEAR_HISTORY 28

using namespace Microsoft::WRL;

class WebView {

public:
    static const int c_uiBarHeight = 70;
    static const int c_optionsDropdownHeight = 108;
    static const int c_optionsDropdownWidth = 200;

protected:
    HWND m_hWnd = nullptr;
    HINSTANCE m_hInst = nullptr;
    LPCWSTR m_url = L"";

    int m_minWindowWidth = 0;
    int m_minWindowHeight = 0;

    std::map<int, wil::com_ptr<ICoreWebView2Environment>> m_uiEnvs;
    std::map<int, wil::com_ptr<ICoreWebView2Environment>> m_contentEnvs;
    std::map<int, wil::com_ptr<ICoreWebView2Controller>> m_controlsControllers;
    std::map<int, wil::com_ptr<ICoreWebView2Controller>> m_optionsControllers;
    std::map<int, wil::com_ptr<ICoreWebView2>> m_controlsWebViews;
    std::map<int, wil::com_ptr<ICoreWebView2>> m_optionsWebViews;

    std::map<int, ComPtr<ICoreWebView2WebMessageReceivedEventHandler>> m_uiMessageBrokers;

    std::map<int, size_t> m_activeTabIds;
    std::map<int, std::map<size_t, std::unique_ptr<WebTab>>> m_tabsmap;

    EventRegistrationToken m_controlsUIMessageBrokerToken = {}; // Token for the UI message handler in controls WebView
    EventRegistrationToken m_controlsZoomToken = {};
    EventRegistrationToken m_optionsUIMessageBrokerToken = {}; // Token for the UI message handler in options WebView
    EventRegistrationToken m_optionsZoomToken = {};
    EventRegistrationToken m_lostOptionsFocus = {}; // Token for the lost focus handler in options WebView

    std::map<int, BOOL> m_showbar_tabs;

protected:
    HRESULT InitUIWebViews(int tabid);
    HRESULT CreateBrowserControlsWebView(int tabid);
    HRESULT CreateBrowserOptionsWebView(int tabid);
    HRESULT ClearContentCache(int wtabid);
    HRESULT ClearControlsCache(int wtabid);
    HRESULT ClearContentCookies(int wtabid);
    HRESULT ClearControlsCookies(int wtabid);

    void SetUIMessageBroker(int tabid);
    void UpdateMinWindowSize();
    HRESULT PostJsonToWebView(web::json::value jsonObj, ICoreWebView2* webview);
    std::wstring GetFilePathAsURI(std::wstring fullPath);

public:
    WebView(HWND hWnd);
    ~WebView();
    void create_webview(int tabid, LPCWSTR url, BOOL showbar = true);
    HRESULT SwitchToTab(size_t tabId, int wtabid);

    static std::wstring GetAppDataDirectory();
    std::wstring GetFullPathFor(LPCWSTR relativePath);
    HRESULT HandleTabURIUpdate(int wtabid, size_t tabId, ICoreWebView2* webview);
    HRESULT HandleTabHistoryUpdate(int wtabid, size_t tabId, ICoreWebView2* webview);
    HRESULT HandleTabNavStarting(int wtabid, size_t tabId, ICoreWebView2* webview);
    HRESULT HandleTabNavCompleted(int wtabid, size_t tabId, ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args);
    HRESULT HandleTabSecurityUpdate(int wtabid, size_t tabId, ICoreWebView2* webview, ICoreWebView2DevToolsProtocolEventReceivedEventArgs* args);
    void HandleTabCreated(size_t tabId, bool shouldBeActive, int wtabid);
    HRESULT HandleTabMessageReceived(int wtabid, size_t tabId, ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* eventArgs);
    int GetDPIAwareBound(int bound);
    static void CheckFailure(HRESULT hr, LPCWSTR errorMessage);

    HRESULT ResizeUIWebViews(int tabid);
    HRESULT hide_webview(int tabid);

};
