#include "WebView.h"
#include "shlobj.h"
#include <Urlmon.h>
#pragma comment(lib, "Urlmon.lib")

WebView::WebView(HWND hWnd)
{
    m_hWnd = hWnd;
    m_hInst = GetModuleHandle(0);
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}
WebView::~WebView()
{
}
HRESULT WebView::ResizeUIWebViews(int tabid)
{
    if (m_controlsWebViews[tabid] != nullptr) {
        RECT bounds;
        GetClientRect(m_hWnd, &bounds);

        if (m_showbar_tabs[tabid]) {
            bounds.bottom = bounds.top + GetDPIAwareBound(c_uiBarHeight);
            bounds.bottom += 1;
        } else {
            bounds.bottom = 0;
        }
        RETURN_IF_FAILED(m_controlsControllers[tabid]->put_Bounds(bounds));

    }

    if (m_optionsWebViews[tabid] != nullptr) {
        RECT bounds;
        GetClientRect(m_hWnd, &bounds);

        if (m_showbar_tabs[tabid]) {
            bounds.top = GetDPIAwareBound(c_uiBarHeight);
        } else {
            bounds.top = 0;
        }

        bounds.bottom = bounds.top + GetDPIAwareBound(c_optionsDropdownHeight);
        bounds.left = bounds.right - GetDPIAwareBound(c_optionsDropdownWidth);
        RETURN_IF_FAILED(m_optionsControllers[tabid]->put_Bounds(bounds));
    }

    if (m_tabsmap[tabid].find(m_activeTabIds[tabid]) != m_tabsmap[tabid].end())
        m_tabsmap[tabid].at(m_activeTabIds[tabid])->ResizeWebView(m_showbar_tabs[tabid]);

    HWND wvWindow = GetWindow(m_hWnd, GW_CHILD);
    while (wvWindow != nullptr) {
        UpdateWindow(wvWindow);
        wvWindow = GetWindow(wvWindow, GW_HWNDNEXT);
    }

    return S_OK;
}
void WebView::create_webview(int tabid, LPCWSTR url, BOOL showbar)
{
    m_url = url;
    m_showbar_tabs[tabid] = showbar;
    SetUIMessageBroker(tabid);
    std::wstring userDataDirectory = GetAppDataDirectory();
    userDataDirectory.append(L"\\User Data");

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataDirectory.c_str(),
        nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this, tabid](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            RETURN_IF_FAILED(result);
            m_contentEnvs[tabid] = env;
            return InitUIWebViews(tabid);
        }).Get());
}
HRESULT WebView::InitUIWebViews(int tabid)
{
    std::wstring browserDataDirectory = GetAppDataDirectory();
    browserDataDirectory.append(L"\\Browser Data");

    return CreateCoreWebView2EnvironmentWithOptions(nullptr, browserDataDirectory.c_str(),
        nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this, tabid](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            m_uiEnvs[tabid] = env;
            RETURN_IF_FAILED(CreateBrowserControlsWebView(tabid));
            RETURN_IF_FAILED(CreateBrowserOptionsWebView(tabid));
            return S_OK;
        }).Get());
}

HRESULT WebView::CreateBrowserControlsWebView(int tabid)
{
    return m_uiEnvs[tabid]->CreateCoreWebView2Controller(m_hWnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this, tabid](HRESULT result, ICoreWebView2Controller* host) -> HRESULT {
            if (!SUCCEEDED(result))
                return result;

            m_controlsControllers[tabid] = host;
            CheckFailure(m_controlsControllers[tabid]->get_CoreWebView2(&m_controlsWebViews[tabid]), L"");

            wil::com_ptr<ICoreWebView2Settings> settings;
            RETURN_IF_FAILED(m_controlsWebViews[tabid]->get_Settings(&settings));
            RETURN_IF_FAILED(settings->put_AreDevToolsEnabled(FALSE));

            RETURN_IF_FAILED(m_controlsControllers[tabid]->add_ZoomFactorChanged(
                Callback<ICoreWebView2ZoomFactorChangedEventHandler>(
                    [](ICoreWebView2Controller* host, IUnknown* args) -> HRESULT {
                        host->put_ZoomFactor(1.0);
                        return S_OK;
                    })
                    .Get(),
                &m_controlsZoomToken));

            RETURN_IF_FAILED(m_controlsWebViews[tabid]->add_WebMessageReceived(m_uiMessageBrokers[tabid].Get(), &m_controlsUIMessageBrokerToken));
            RETURN_IF_FAILED(ResizeUIWebViews(tabid));

            std::wstring controlsPath = GetFullPathFor(L"wvbrowser_ui\\controls_ui\\default.html");
            RETURN_IF_FAILED(m_controlsWebViews[tabid]->Navigate(controlsPath.c_str()));

            return S_OK;
        }).Get());
}

HRESULT WebView::CreateBrowserOptionsWebView(int tabid)
{
    return m_uiEnvs[tabid]->CreateCoreWebView2Controller(m_hWnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this, tabid](HRESULT result, ICoreWebView2Controller* host) -> HRESULT {
            if (!SUCCEEDED(result)) {
                return result;
            }

            m_optionsControllers[tabid] = host;
            CheckFailure(m_optionsControllers[tabid]->get_CoreWebView2(&m_optionsWebViews[tabid]), L"");

            wil::com_ptr<ICoreWebView2Settings> settings;
            RETURN_IF_FAILED(m_optionsWebViews[tabid]->get_Settings(&settings));
            RETURN_IF_FAILED(settings->put_AreDevToolsEnabled(FALSE));

            RETURN_IF_FAILED(m_optionsControllers[tabid]->add_ZoomFactorChanged(
                Callback<ICoreWebView2ZoomFactorChangedEventHandler>(
                    [](ICoreWebView2Controller* host, IUnknown* args) -> HRESULT {
                        host->put_ZoomFactor(1.0);
                        return S_OK;
                    })
                    .Get(),
                &m_optionsZoomToken));

            RETURN_IF_FAILED(m_optionsControllers[tabid]->put_IsVisible(FALSE));
            RETURN_IF_FAILED(m_optionsWebViews[tabid]->add_WebMessageReceived(m_uiMessageBrokers[tabid].Get(), &m_optionsUIMessageBrokerToken));

            RETURN_IF_FAILED(m_optionsControllers[tabid]->add_LostFocus(
                Callback<ICoreWebView2FocusChangedEventHandler>(
                    [this, tabid](ICoreWebView2Controller* sender, IUnknown* args) -> HRESULT {
                        web::json::value jsonObj = web::json::value::parse(L"{}");
                        jsonObj[L"message"] = web::json::value(MG_OPTIONS_LOST_FOCUS);
                        jsonObj[L"args"] = web::json::value::parse(L"{}");
                        PostJsonToWebView(jsonObj, m_controlsWebViews[tabid].get());
                        return S_OK;
                    })
                    .Get(),
                &m_lostOptionsFocus));

            RETURN_IF_FAILED(ResizeUIWebViews(tabid));

            std::wstring optionsPath = GetFullPathFor(L"wvbrowser_ui\\controls_ui\\options.html");
            RETURN_IF_FAILED(m_optionsWebViews[tabid]->Navigate(optionsPath.c_str()));

            return S_OK;
        }).Get());
}
void WebView::SetUIMessageBroker(int tabid)
{
    m_uiMessageBrokers[tabid] = Callback<ICoreWebView2WebMessageReceivedEventHandler>(
        [this, tabid](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* eventArgs) -> HRESULT {
            wil::unique_cotaskmem_string jsonString;
            CheckFailure(eventArgs->get_WebMessageAsJson(&jsonString), L"");
            web::json::value jsonObj = web::json::value::parse(jsonString.get());

            if (!jsonObj.has_field(L"message") || !jsonObj.has_field(L"args"))
                return S_OK;

            int message = jsonObj.at(L"message").as_integer();
            web::json::value args = jsonObj.at(L"args");

            switch (message) {
            case MG_CREATE_TAB: {
                size_t id = args.at(L"tabId").as_number().to_uint32();
                bool shouldBeActive = args.at(L"active").as_bool();
                std::unique_ptr<WebTab> newTab = WebTab::CreateNewTab(m_hWnd, tabid, m_contentEnvs[tabid].get(), id, shouldBeActive, m_url);
                std::map<size_t, std::unique_ptr<WebTab>>::iterator it = m_tabsmap[tabid].find(id);
                if (it == m_tabsmap[tabid].end()) {
                    m_tabsmap[tabid].insert(std::pair<size_t, std::unique_ptr<WebTab>>(id, std::move(newTab)));
                } else {
                    m_tabsmap[tabid].at(id)->m_contentController->Close();
                    it->second = std::move(newTab);
                }

            } break;
            case MG_NAVIGATE: {
                std::wstring uri(args.at(L"uri").as_string());
                std::wstring browserScheme(L"browser://");

                if (uri.substr(0, browserScheme.size()).compare(browserScheme) == 0) {
                    std::wstring path = uri.substr(browserScheme.size());
                    if (path.compare(L"favorites") == 0 || path.compare(L"settings") == 0 || path.compare(L"history") == 0) {
                        std::wstring filePath(L"wvbrowser_ui\\content_ui\\");
                        filePath.append(path);
                        filePath.append(L".html");
                        std::wstring fullPath = GetFullPathFor(filePath.c_str());
                        CheckFailure(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->Navigate(fullPath.c_str()), L"Can't navigate to browser page.");
                    } else {
                        OutputDebugString(L"Requested unknown browser page\n");
                    }
                } else if (!SUCCEEDED(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->Navigate(uri.c_str()))) {
                    CheckFailure(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->Navigate(args.at(L"encodedSearchURI").as_string().c_str()), L"Can't navigate to requested page.");
                }

            } break;
            case MG_GO_FORWARD: {
                CheckFailure(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->GoForward(), L"");

            } break;
            case MG_GO_BACK: {
                CheckFailure(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->GoBack(), L"");

            } break;
            case MG_RELOAD: {
                CheckFailure(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->Reload(), L"");

            } break;
            case MG_CANCEL: {
                CheckFailure(m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentWebView->CallDevToolsProtocolMethod(L"Page.stopLoading", L"{}", nullptr), L"");

            } break;
            case MG_SWITCH_TAB: {
                size_t tabId = args.at(L"tabId").as_number().to_uint32();
                SwitchToTab(tabId, tabid);

            } break;
            case MG_CLOSE_TAB: {
                size_t id = args.at(L"tabId").as_number().to_uint32();
                m_tabsmap[tabid].at(id)->m_contentController->Close();
                m_tabsmap[tabid].erase(id);

            } break;
            case MG_CLOSE_WINDOW: {
                DestroyWindow(m_hWnd);

            } break;
            case MG_SHOW_OPTIONS: {
                CheckFailure(m_optionsControllers[tabid]->put_IsVisible(TRUE), L"");
                m_optionsControllers[tabid]->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

            } break;
            case MG_HIDE_OPTIONS: {
                CheckFailure(m_optionsControllers[tabid]->put_IsVisible(FALSE), L"Something went wrong when trying to close the options dropdown.");

            } break;
            case MG_OPTION_SELECTED: {
                m_tabsmap[tabid].at(m_activeTabIds[tabid])->m_contentController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

            } break;
            case MG_GET_FAVORITES:
            case MG_GET_SETTINGS:
            case MG_GET_HISTORY: {
                size_t tabId = args.at(L"tabId").as_number().to_uint32();
                jsonObj[L"args"].erase(L"tabId");
                CheckFailure(PostJsonToWebView(jsonObj, m_tabsmap[tabid].at(tabId)->m_contentWebView.Get()), L"Requesting history failed.");

            } break;
            default: {
            }
            }

            return S_OK;
        });
}

HRESULT WebView::SwitchToTab(size_t tabId, int wtabid)
{
    size_t previousActiveTab = m_activeTabIds[wtabid];
    RETURN_IF_FAILED(m_tabsmap[wtabid].at(tabId)->ResizeWebView(m_showbar_tabs[wtabid]));
    RETURN_IF_FAILED(m_tabsmap[wtabid].at(tabId)->m_contentController->put_IsVisible(TRUE));
    m_activeTabIds[wtabid] = tabId;

    if (previousActiveTab != INVALID_TAB_ID && previousActiveTab != m_activeTabIds[wtabid]) {
        RETURN_IF_FAILED(m_tabsmap[wtabid].at(previousActiveTab)->m_contentController->put_IsVisible(FALSE));
    }

    return S_OK;
}
HRESULT WebView::HandleTabURIUpdate(int wtabid, size_t tabId, ICoreWebView2* webview)
{
    wil::unique_cotaskmem_string source;
    RETURN_IF_FAILED(webview->get_Source(&source));

    web::json::value jsonObj = web::json::value::parse(L"{}");
    jsonObj[L"message"] = web::json::value(MG_UPDATE_URI);
    jsonObj[L"args"] = web::json::value::parse(L"{}");
    jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);
    jsonObj[L"args"][L"uri"] = web::json::value(source.get());

    std::wstring uri(source.get());
    std::wstring favoritesURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\favorites.html"));
    std::wstring settingsURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\settings.html"));
    std::wstring historyURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\history.html"));

    if (uri.compare(favoritesURI) == 0) {
        jsonObj[L"args"][L"uriToShow"] = web::json::value(L"browser://favorites");
    } else if (uri.compare(settingsURI) == 0) {
        jsonObj[L"args"][L"uriToShow"] = web::json::value(L"browser://settings");
    } else if (uri.compare(historyURI) == 0) {
        jsonObj[L"args"][L"uriToShow"] = web::json::value(L"browser://history");
    }

    RETURN_IF_FAILED(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()));

    return S_OK;
}
HRESULT WebView::HandleTabHistoryUpdate(int wtabid, size_t tabId, ICoreWebView2* webview)
{
    wil::unique_cotaskmem_string source;
    RETURN_IF_FAILED(webview->get_Source(&source));

    web::json::value jsonObj = web::json::value::parse(L"{}");
    jsonObj[L"message"] = web::json::value(MG_UPDATE_URI);
    jsonObj[L"args"] = web::json::value::parse(L"{}");
    jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);
    jsonObj[L"args"][L"uri"] = web::json::value(source.get());

    BOOL canGoForward = FALSE;
    RETURN_IF_FAILED(webview->get_CanGoForward(&canGoForward));
    jsonObj[L"args"][L"canGoForward"] = web::json::value::boolean(canGoForward);

    BOOL canGoBack = FALSE;
    RETURN_IF_FAILED(webview->get_CanGoBack(&canGoBack));
    jsonObj[L"args"][L"canGoBack"] = web::json::value::boolean(canGoBack);

    RETURN_IF_FAILED(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()));

    return S_OK;
}
HRESULT WebView::HandleTabNavStarting(int wtabid, size_t tabId, ICoreWebView2* webview)
{
    web::json::value jsonObj = web::json::value::parse(L"{}");
    jsonObj[L"message"] = web::json::value(MG_NAV_STARTING);
    jsonObj[L"args"] = web::json::value::parse(L"{}");
    jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);

    return PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get());
}
HRESULT WebView::HandleTabNavCompleted(int wtabid, size_t tabId, ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args)
{
    std::wstring getTitleScript(
        // Look for a title tag
        L"(() => {"
        L"    var links = document.links, i, length;for (i = 0, length = links.length; i < length; i++) {links[i].target == '_blank' && links[i].removeAttribute('target');}"
        L"    const titleTag = document.getElementsByTagName('title')[0];"
        L"    if (titleTag) {"
        L"        return titleTag.innerHTML;"
        L"    }"
        // No title tag, look for the file name
        L"    pathname = window.location.pathname;"
        L"    var filename = pathname.split('/').pop();"
        L"    if (filename) {"
        L"        return filename;"
        L"    }"

        // No file name, look for the hostname
        L"    const hostname =  window.location.hostname;"
        L"    if (hostname) {"
        L"        return hostname;"
        L"    }"
        // Fallback: let the UI use a generic title
        L"    return '';"
        L"})();");

    std::wstring getFaviconURI(
        L"(() => {"
        // Let the UI use a fallback favicon
        L"    let faviconURI = '';"
        L"    let links = document.getElementsByTagName('link');"
        // Test each link for a favicon
        L"    Array.from(links).map(element => {"
        L"        let rel = element.rel;"
        // Favicon is declared, try to get the href
        L"        if (rel && (rel == 'shortcut icon' || rel == 'icon')) {"
        L"            if (!element.href) {"
        L"                return;"
        L"            }"
        // href to icon found, check it's full URI
        L"            try {"
        L"                let urlParser = new URL(element.href);"
        L"                faviconURI = urlParser.href;"
        L"            } catch(e) {"
        // Try prepending origin
        L"                let origin = window.location.origin;"
        L"                let faviconLocation = `${origin}/${element.href}`;"
        L"                try {"
        L"                    urlParser = new URL(faviconLocation);"
        L"                    faviconURI = urlParser.href;"
        L"                } catch (e2) {"
        L"                    return;"
        L"                }"
        L"            }"
        L"        }"
        L"    });"
        L"    return faviconURI;"
        L"})();");

    CheckFailure(webview->ExecuteScript(getTitleScript.c_str(),
                     Callback<ICoreWebView2ExecuteScriptCompletedHandler>([this, tabId, wtabid](HRESULT error, PCWSTR result) -> HRESULT {
                         RETURN_IF_FAILED(error);

                         web::json::value jsonObj = web::json::value::parse(L"{}");
                         jsonObj[L"message"] = web::json::value(MG_UPDATE_TAB);
                         jsonObj[L"args"] = web::json::value::parse(L"{}");
                         jsonObj[L"args"][L"title"] = web::json::value::parse(result);
                         jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);

                         CheckFailure(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()), L"Can't update title.");
                         return S_OK;
                     }).Get()),
        L"Can't update title.");

    CheckFailure(webview->ExecuteScript(getFaviconURI.c_str(),
                     Callback<ICoreWebView2ExecuteScriptCompletedHandler>([this, tabId, wtabid](HRESULT error, PCWSTR result) -> HRESULT {
                         RETURN_IF_FAILED(error);

                         web::json::value jsonObj = web::json::value::parse(L"{}");
                         jsonObj[L"message"] = web::json::value(MG_UPDATE_FAVICON);
                         jsonObj[L"args"] = web::json::value::parse(L"{}");
                         jsonObj[L"args"][L"uri"] = web::json::value::parse(result);
                         jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);

                         CheckFailure(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()), L"Can't update favicon.");
                         return S_OK;
                     }).Get()),
        L"Can't update favicon");

    web::json::value jsonObj = web::json::value::parse(L"{}");
    jsonObj[L"message"] = web::json::value(MG_NAV_COMPLETED);
    jsonObj[L"args"] = web::json::value::parse(L"{}");
    jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);

    BOOL navigationSucceeded = FALSE;
    if (SUCCEEDED(args->get_IsSuccess(&navigationSucceeded))) {
        jsonObj[L"args"][L"isError"] = web::json::value::boolean(!navigationSucceeded);
    }

    return PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get());
}
HRESULT WebView::HandleTabSecurityUpdate(int wtabid, size_t tabId, ICoreWebView2* webview, ICoreWebView2DevToolsProtocolEventReceivedEventArgs* args)
{
    wil::unique_cotaskmem_string jsonArgs;
    RETURN_IF_FAILED(args->get_ParameterObjectAsJson(&jsonArgs));
    web::json::value securityEvent = web::json::value::parse(jsonArgs.get());

    web::json::value jsonObj = web::json::value::parse(L"{}");
    jsonObj[L"message"] = web::json::value(MG_SECURITY_UPDATE);
    jsonObj[L"args"] = web::json::value::parse(L"{}");
    jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);
    jsonObj[L"args"][L"state"] = securityEvent.at(L"securityState");

    return PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get());
}
void WebView::HandleTabCreated(size_t tabId, bool shouldBeActive, int wtabid)
{
    if (shouldBeActive) {
        CheckFailure(SwitchToTab(tabId, wtabid), L"");
    }
}
HRESULT WebView::HandleTabMessageReceived(int wtabid, size_t tabId, ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* eventArgs)
{
    wil::unique_cotaskmem_string jsonString;
    RETURN_IF_FAILED(eventArgs->get_WebMessageAsJson(&jsonString));
    web::json::value jsonObj = web::json::value::parse(jsonString.get());

    wil::unique_cotaskmem_string uri;
    RETURN_IF_FAILED(webview->get_Source(&uri));

    int message = jsonObj.at(L"message").as_integer();
    web::json::value args = jsonObj.at(L"args");

    wil::unique_cotaskmem_string source;
    RETURN_IF_FAILED(webview->get_Source(&source));

    switch (message) {
    case MG_GET_FAVORITES:
    case MG_REMOVE_FAVORITE: {
        std::wstring fileURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\favorites.html"));
        // Only the favorites UI can request favorites
        if (fileURI.compare(source.get()) == 0) {
            jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);
            CheckFailure(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()), L"Couldn't perform favorites operation.");
        }
    } break;
    case MG_GET_SETTINGS: {
        std::wstring fileURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\settings.html"));
        if (fileURI.compare(source.get()) == 0) {
            jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);
            CheckFailure(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()), L"Couldn't retrieve settings.");
        }
    } break;
    case MG_CLEAR_CACHE: {
        std::wstring fileURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\settings.html"));
        if (fileURI.compare(uri.get()) == 0) {
            jsonObj[L"args"][L"content"] = web::json::value::boolean(false);
            jsonObj[L"args"][L"controls"] = web::json::value::boolean(false);

            if (SUCCEEDED(ClearContentCache(wtabid))) {
                jsonObj[L"args"][L"content"] = web::json::value::boolean(true);
            }

            if (SUCCEEDED(ClearControlsCache(wtabid))) {
                jsonObj[L"args"][L"controls"] = web::json::value::boolean(true);
            }

            CheckFailure(PostJsonToWebView(jsonObj, m_tabsmap[wtabid].at(tabId)->m_contentWebView.Get()), L"");
        }
    } break;
    case MG_CLEAR_COOKIES: {
        std::wstring fileURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\settings.html"));
        if (fileURI.compare(uri.get()) == 0) {
            jsonObj[L"args"][L"content"] = web::json::value::boolean(false);
            jsonObj[L"args"][L"controls"] = web::json::value::boolean(false);

            if (SUCCEEDED(ClearContentCookies(wtabid))) {
                jsonObj[L"args"][L"content"] = web::json::value::boolean(true);
            }

            if (SUCCEEDED(ClearControlsCookies(wtabid))) {
                jsonObj[L"args"][L"controls"] = web::json::value::boolean(true);
            }

            CheckFailure(PostJsonToWebView(jsonObj, m_tabsmap[wtabid].at(tabId)->m_contentWebView.Get()), L"");
        }
    } break;
    case MG_GET_HISTORY:
    case MG_REMOVE_HISTORY_ITEM:
    case MG_CLEAR_HISTORY: {
        std::wstring fileURI = GetFilePathAsURI(GetFullPathFor(L"wvbrowser_ui\\content_ui\\history.html"));
        if (fileURI.compare(uri.get()) == 0) {
            jsonObj[L"args"][L"tabId"] = web::json::value::number(tabId);
            CheckFailure(PostJsonToWebView(jsonObj, m_controlsWebViews[wtabid].get()), L"Couldn't perform history operation");
        }
    } break;
    default: {

    } break;
    }

    return S_OK;
}
HRESULT WebView::ClearContentCache(int wtabid)
{
    return m_tabsmap[wtabid].at(m_activeTabIds[wtabid])->m_contentWebView->CallDevToolsProtocolMethod(L"Network.clearBrowserCache", L"{}", nullptr);
}
HRESULT WebView::ClearControlsCache(int wtabid)
{
    return m_controlsWebViews[wtabid]->CallDevToolsProtocolMethod(L"Network.clearBrowserCache", L"{}", nullptr);
}
HRESULT WebView::ClearContentCookies(int wtabid)
{
    return m_tabsmap[wtabid].at(m_activeTabIds[wtabid])->m_contentWebView->CallDevToolsProtocolMethod(L"Network.clearBrowserCookies", L"{}", nullptr);
}
HRESULT WebView::ClearControlsCookies(int wtabid)
{
    return m_controlsWebViews[wtabid]->CallDevToolsProtocolMethod(L"Network.clearBrowserCookies", L"{}", nullptr);
}
void WebView::UpdateMinWindowSize()
{
    RECT clientRect;
    RECT windowRect;

    GetClientRect(m_hWnd, &clientRect);
    GetWindowRect(m_hWnd, &windowRect);

    int bordersWidth = (windowRect.right - windowRect.left) - clientRect.right;
    int bordersHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom;

    m_minWindowWidth = GetDPIAwareBound(MIN_WINDOW_WIDTH) + bordersWidth;
    m_minWindowHeight = GetDPIAwareBound(MIN_WINDOW_HEIGHT) + bordersHeight;
}
void WebView::CheckFailure(HRESULT hr, LPCWSTR errorMessage)
{
    if (FAILED(hr)) {
        std::wstring message;
        if (!errorMessage || !errorMessage[0]) {
            message = std::wstring(L"Something went wrong.");
        } else {
            message = std::wstring(errorMessage);
        }

        MessageBoxW(nullptr, message.c_str(), nullptr, MB_OK);
    }
}
int WebView::GetDPIAwareBound(int bound)
{
    return (bound * GetDpiForWindow(m_hWnd) / DEFAULT_DPI);
}
std::wstring WebView::GetAppDataDirectory()
{
    TCHAR path[MAX_PATH];
    std::wstring dataDirectory;
    HRESULT hr = SHGetFolderPath(nullptr, CSIDL_APPDATA, NULL, 0, path);
    if (SUCCEEDED(hr)) {
        dataDirectory = std::wstring(path);
        dataDirectory.append(L"\\kxkx5150\\");
    } else {
        dataDirectory = std::wstring(L".\\");
    }

    dataDirectory.append(L"kxEditor");
    return dataDirectory;
}
std::wstring WebView::GetFullPathFor(LPCWSTR relativePath)
{
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(m_hInst, path, MAX_PATH);
    std::wstring pathName(path);

    std::size_t index = pathName.find_last_of(L"\\") + 1;
    pathName.replace(index, pathName.length(), relativePath);

    return pathName;
}
std::wstring WebView::GetFilePathAsURI(std::wstring fullPath)
{
    std::wstring fileURI;
    ComPtr<IUri> uri;
    DWORD uriFlags = Uri_CREATE_ALLOW_IMPLICIT_FILE_SCHEME;
    HRESULT hr = CreateUri(fullPath.c_str(), uriFlags, 0, &uri);

    if (SUCCEEDED(hr)) {
        wil::unique_bstr absoluteUri;
        uri->GetAbsoluteUri(&absoluteUri);
        fileURI = std::wstring(absoluteUri.get());
    }

    return fileURI;
}
HRESULT WebView::PostJsonToWebView(web::json::value jsonObj, ICoreWebView2* webview)
{
    utility::stringstream_t stream;
    jsonObj.serialize(stream);
    return webview->PostWebMessageAsJson(stream.str().c_str());
}
