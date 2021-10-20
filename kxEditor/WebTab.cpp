#include "WebTab.h"
#include "WebView.h"
int WebTab::m_wtabid = 0;

std::unique_ptr<WebTab> WebTab::CreateNewTab(HWND hWnd, int tabid,ICoreWebView2Environment* env, size_t id, bool shouldBeActive, LPCWSTR url)
{
    m_wtabid = tabid;
    std::unique_ptr<WebTab> tab = std::make_unique<WebTab>();
    tab->m_parentHWnd = hWnd;
    tab->m_tabId = id;
    tab->m_wtabid = tabid;
    tab->SetMessageBroker();
    tab->Init(env, shouldBeActive, url);
    return tab;
}
HRESULT WebTab::Init(ICoreWebView2Environment* env, bool shouldBeActive, LPCWSTR url)
{
    return env->CreateCoreWebView2Controller(m_parentHWnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this, shouldBeActive, url](HRESULT result, ICoreWebView2Controller* host) -> HRESULT {
            if (!SUCCEEDED(result))
                return result;

            m_contentController = host;
            WebView::CheckFailure(m_contentController->get_CoreWebView2(&m_contentWebView), L"");
            WebView* browserWindow = reinterpret_cast<WebView*>(GetWindowLongPtr(m_parentHWnd, GWLP_USERDATA));
            RETURN_IF_FAILED(m_contentWebView->add_WebMessageReceived(m_messageBroker.Get(), &m_messageBrokerToken));

            RETURN_IF_FAILED(m_contentWebView->add_HistoryChanged(
                Callback<ICoreWebView2HistoryChangedEventHandler>(
                    [this, browserWindow](ICoreWebView2* webview, IUnknown* args) -> HRESULT {
                        WebView::CheckFailure(browserWindow->HandleTabHistoryUpdate(m_wtabid,m_tabId, webview), L"Can't update go back/forward buttons.");
                        return S_OK;
                    })
                    .Get(),
                &m_historyUpdateForwarderToken));

            RETURN_IF_FAILED(m_contentWebView->add_SourceChanged(
                Callback<ICoreWebView2SourceChangedEventHandler>(
                    [this, browserWindow](ICoreWebView2* webview, ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
                        WebView::CheckFailure(browserWindow->HandleTabURIUpdate(m_wtabid,m_tabId, webview), L"Can't update address bar");
                        return S_OK;
                    })
                    .Get(),
                &m_uriUpdateForwarderToken));

            RETURN_IF_FAILED(m_contentWebView->add_NavigationStarting(
                Callback<ICoreWebView2NavigationStartingEventHandler>(
                    [this, browserWindow](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                        WebView::CheckFailure(browserWindow->HandleTabNavStarting(m_wtabid,m_tabId, webview), L"Can't update reload button");
                        return S_OK;
                    })
                    .Get(),
                &m_navStartingToken));

            RETURN_IF_FAILED(m_contentWebView->add_NavigationCompleted(
                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                    [this, browserWindow](ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                        WebView::CheckFailure(browserWindow->HandleTabNavCompleted(m_wtabid,m_tabId, webview, args), L"Can't udpate reload button");
                        return S_OK;
                    })
                    .Get(),
                &m_navCompletedToken));

            RETURN_IF_FAILED(m_contentWebView->CallDevToolsProtocolMethod(L"Security.enable", L"{}", nullptr));
            WebView::CheckFailure(m_contentWebView->GetDevToolsProtocolEventReceiver(L"Security.securityStateChanged", &m_securityStateChangedReceiver), L"");

            RETURN_IF_FAILED(m_securityStateChangedReceiver->add_DevToolsProtocolEventReceived(
                Callback<ICoreWebView2DevToolsProtocolEventReceivedEventHandler>(
                    [this, browserWindow](ICoreWebView2* webview, ICoreWebView2DevToolsProtocolEventReceivedEventArgs* args) -> HRESULT {
                        WebView::CheckFailure(browserWindow->HandleTabSecurityUpdate(m_wtabid,m_tabId, webview, args), L"Can't udpate security icon");
                        return S_OK;
                    })
                    .Get(),
                &m_securityUpdateToken));

            RETURN_IF_FAILED(m_contentWebView->add_NewWindowRequested(
                Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                    [this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) {
                        LPWSTR uri;
                        args->get_Uri(&uri);
                        OutputDebugString(uri);
                        m_contentWebView->Navigate(uri);
                        args->put_Handled(true);
                        return S_OK;
                    })
                    .Get(),
                nullptr));

            RETURN_IF_FAILED(m_contentWebView->Navigate(url));
            browserWindow->HandleTabCreated(m_tabId, shouldBeActive, m_wtabid);

            return S_OK;
        }).Get());
}

void WebTab::SetMessageBroker()
{
    m_messageBroker = Callback<ICoreWebView2WebMessageReceivedEventHandler>(
        [this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* eventArgs) -> HRESULT {
            WebView* browserWindow = reinterpret_cast<WebView*>(GetWindowLongPtr(m_parentHWnd, GWLP_USERDATA));
            WebView::CheckFailure(browserWindow->HandleTabMessageReceived(m_wtabid,m_tabId, webview, eventArgs), L"");
            return S_OK;
        });
}
HRESULT WebTab::ResizeWebView()
{
    RECT bounds;
    GetClientRect(m_parentHWnd, &bounds);
    WebView* browserWindow = reinterpret_cast<WebView*>(GetWindowLongPtr(m_parentHWnd, GWLP_USERDATA));
    bounds.top += browserWindow->GetDPIAwareBound(WebView::c_uiBarHeight);
    return m_contentController->put_Bounds(bounds);
}
