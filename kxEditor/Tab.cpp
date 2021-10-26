#include "Tab.h"
#include "WebMgr.h"
#include "WebView.h"

Tab::Tab(TCHAR* szFileName, HWND hWnd, HWND tabhWnd, HWND txthwnd,
    TextEditor* txtee, EditView* editview, HWND whWnd, WebView* webview, int tabid, Mode mode)
{
    m_tabid = tabid;
    m_hwnd = hWnd;
    m_tabhWnd = tabhWnd;
    m_txthWnd = txthwnd;
    m_txteditr = txtee;
    m_editview = editview;
    m_webhwnd = whWnd;
    m_webeditr = webview;
    create(szFileName, mode);
}
Tab::~Tab()
{
    close_document();
}
LONG Tab::create(TCHAR* szFileName, Mode mode)
{
    if (mode == Mode::TEXT) {
        set_current_mode(Mode::TEXT);
        create_file(szFileName);

    } else if (mode == Mode::TERMINAL) {
        set_current_mode(Mode::TERMINAL);
        create_webview();

    } else if (mode == Mode::WEBVIEW) {
        set_current_mode(Mode::WEBVIEW);
        create_webview();
    }

    return 0;
}
LONG Tab::create_file(TCHAR* szFileName)
{
    init_file(szFileName);
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
LONG Tab::init_file(TCHAR* szFileName)
{
    close_document();
    m_Document = new LinkedList();
    m_docmgr = new DocMgr(m_Document, m_txteditr, m_editview, m_txthWnd);
    m_webmgr = new WebMgr(m_webhwnd, m_webeditr, m_tabid);
    return TRUE;
}
void Tab::create_webview()
{
    create_file();
    change_webview();
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
    if (m_mode == Mode::TEXT) {
        MoveWindow(m_webhwnd, 0, 0, 0, 0, TRUE);
        MoveWindow(m_txthWnd, m_x, m_y, m_width, m_height, TRUE);

    } else if (m_mode == Mode::TERMINAL || m_mode == Mode::WEBVIEW) {
        MoveWindow(m_txthWnd, 0, 0, 0, 0, TRUE);
        MoveWindow(m_webhwnd, m_x, m_y, m_width, m_height, TRUE);
    }
}
Mode Tab::get_current_mode()
{
    return m_mode;
}
void Tab::set_current_mode(Mode mode)
{
    m_mode = mode;
}
void Tab::set_size(int width, int height, int x, int y)
{
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;
}
void Tab::resize_view(int width, int height, int x, int y)
{
    set_size(width, height, x, y);
    change_view();
    if (m_mode != Mode::TEXT)
        resize_webview();
}
void Tab::resize_textview()
{
}
void Tab::resize_webview()
{
    if (m_webmgr)
        m_webmgr->resize_webview();
}
void Tab::send_select_msg_webview(BOOL selected)
{
    if (selected) {
        m_webmgr->select_tab();
    } else {
        m_webmgr->unselect_tab();
    }
}
