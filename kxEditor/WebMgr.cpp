#include "WebMgr.h"
#include <crtdbg.h>

WebMgr::WebMgr(HWND hwnd, WebView* webeditr, int tabid)
{
    m_hwnd = hwnd;
    m_webeditr = webeditr;
    m_tabid = tabid;
}
WebMgr::~WebMgr()
{
}
void WebMgr::create_webview(int tabid, LPCWSTR url, BOOL showbar)
{
    m_webeditr->create_webview(tabid, url, showbar);
}
void WebMgr::resize_webview()
{
    m_webeditr->ResizeUIWebViews(m_tabid);
}
