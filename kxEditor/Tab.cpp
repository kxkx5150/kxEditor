#include "Tab.h"
#include "WebMgr.h"
#include "WebView.h"

Tab::Tab(HWND hWnd, HWND txthwnd, TextEditor* txtee, EditView* editview, HWND whWnd, WebView* webview, int tabid)
{
    m_tabid = tabid;
    m_hwnd = hWnd;
    m_txthWnd = txthwnd;
    m_txteditr = txtee;
    m_editview = editview;
    m_webhwnd = whWnd;
    m_webeditr = webview;
    create();
}
Tab::~Tab()
{
    close_document();
}
LONG Tab::init_file()
{
    close_document();
    m_Document = new LinkedList();
    m_docmgr = new DocMgr(m_Document, m_txteditr, m_editview, m_txthWnd);
    m_webmgr = new WebMgr(m_webhwnd, m_webeditr, m_tabid);
    return TRUE;
}
void Tab::close_document()
{
    if (m_docmgr) {
        delete m_docmgr;
        m_docmgr = nullptr;
    }

    if (m_Document) {
        delete m_Document;
        m_Document = nullptr;
    }
}
LONG Tab::create_emptyfile()
{
    m_Document->create_empty();
    return TRUE;
}
LONG Tab::OpenFile(TCHAR* szFileName)
{
    return create_file(szFileName);
}
LONG Tab::create()
{
    if (m_mode == Mode::TEXT) {
        create_file();

    } else if (m_mode == Mode::TERMINAL) {
        change_webview();

    } else if (m_mode == Mode::WEBVIEW) {
        change_webview();

    }
    return 0;
}
LONG Tab::create_file(TCHAR* szFileName)
{
    init_file();
    if (szFileName) {
        if (m_Document->open_file(szFileName, m_txteditr->m_tabwidth)) {
        } else {
            create_emptyfile();
        }
    } else {
        create_emptyfile();
    }

    m_txteditr->UpdateMetrics();
    m_txteditr->UpdateMarginWidth();
    m_docmgr->set_caret(0, 0);
    return TRUE;
}
void Tab::change_webview()
{
    m_mode = Mode::WEBVIEW;
    change_view();
    m_webmgr->create_webview(m_tabid, L"https://www.google.com/");
}
void Tab::change_cmdview()
{
    m_mode = Mode::TERMINAL;
    change_view();
    m_webmgr->create_webview(m_tabid, L"http://localhost:8890", false);
}
void Tab::change_txtview()
{
    m_mode = Mode::TEXT;
    change_view();
}
void Tab::change_view()
{
    HDWP hdwp = BeginDeferWindowPos(2);
    if (m_mode == Mode::TEXT) {
        DeferWindowPos(hdwp, m_webhwnd, 0, 0, 0, 0, 0, SWP_SHOWWINDOW);
        MoveWindow(m_webhwnd, 0, 0, 0, 0, TRUE);

        DeferWindowPos(hdwp, m_txthWnd, 0, 0, 0, m_width, m_height, SWP_SHOWWINDOW);
        MoveWindow(m_txthWnd, 0, 0, m_width, m_height, TRUE);

    } else if (m_mode == Mode::TERMINAL || m_mode == Mode::WEBVIEW) {
        DeferWindowPos(hdwp, m_txthWnd, 0, 0, 0, 0, 0, SWP_SHOWWINDOW);
        MoveWindow(m_txthWnd, 0, 0, 0, 0, TRUE);

        DeferWindowPos(hdwp, m_webhwnd, 0, 0, 0, m_width, m_height, SWP_SHOWWINDOW);
        MoveWindow(m_webhwnd, 0, 0, m_width, m_height, TRUE);
    }
}
void Tab::resize_view(HDWP hdwp, int width, int height, int x, int y)
{
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;

    if (m_mode == Mode::TEXT) {
        DeferWindowPos(hdwp, m_txthWnd, 0, 0, 0, width, height, SWP_SHOWWINDOW);
        MoveWindow(m_txthWnd, 0, 0, width, height, TRUE);
    } else if (m_mode == Mode::TERMINAL || m_mode == Mode::WEBVIEW) {
        DeferWindowPos(hdwp, m_webhwnd, 0, 0, 0, width, height, SWP_SHOWWINDOW);
        MoveWindow(m_webhwnd, 0, 0, width, height, TRUE);
    }
}
void Tab::resize_webview()
{
    m_webmgr->resize_webview();
}
