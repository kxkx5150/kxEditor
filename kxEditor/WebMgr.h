#pragma once
#include "WebView.h"

class WebMgr {
    HWND m_hwnd;
    WebView* m_webeditr = nullptr;
    int m_tabid = 0;

public:
    WebMgr(HWND hwnd, WebView* webeditr, int tabid);
    ~WebMgr();

    void create_webview(int tabid, LPCWSTR url, BOOL showbar = true);
    void resize_webview();

private:
};
