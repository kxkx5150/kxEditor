#include "WebMgr.h"
#include <crtdbg.h>

WebMgr::WebMgr(HWND hwnd, WebView* webeditr, int tabid)
{
    _RPTN(_CRT_WARN, "tabid :%d\n", tabid);
    m_hwnd = hwnd;
    m_webeditr = webeditr;
    m_tabid = tabid;
}
WebMgr::~WebMgr()
{
}
void WebMgr::create_webview(int tabid, LPCWSTR url)
{
    m_webeditr->create_webview(tabid, url);
}
void WebMgr::resize_webview()
{
    m_webeditr->ResizeUIWebViews(m_tabid);
}
