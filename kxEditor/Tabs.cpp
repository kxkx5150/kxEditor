#include "Tabs.h"
#include "EditView.h"
#include <CommCtrl.h>

int Tabs::m_tabid = -1;

Tabs::Tabs()
{
}
Tabs::~Tabs()
{
    close_all_tabs();
}
void Tabs::init_tabs(HWND hWnd, HWND tabhWnd, TextEditor* txteditr, HWND txthWnd, EditView* editview, HWND whWnd, WebView* webview)
{
    m_hwnd = hWnd;
    m_tabhWnd = tabhWnd;
    m_txthWnd = txthWnd;
    m_webhwnd = whWnd;
    m_txteditr = txteditr;
    m_editview = editview;
    m_webeditr = webview;
    create_tab();
}
LONG Tabs::create_tab(TCHAR* szFileName)
{
    m_tabid++;
    auto tab = new Tab(szFileName,
        m_hwnd, m_tabhWnd, m_txthWnd,
        m_txteditr, m_editview, m_webhwnd, m_webeditr,
        m_tabid,
        Mode::TEXT);
    m_tabs.push_back(tab);
    select_tab(m_tabid);

    int tabid = 0;
    if (szFileName && wcslen(szFileName) > 0) {
        tabid = create_tab_control(szFileName);
    } else {
        tabid = create_tab_control((TCHAR*)L"Untitled");
    }
    tab->set_size(m_width, m_height, m_x, m_y);
    select_tab(tabid);
    return m_tabid;
}
int Tabs::create_tab_control(TCHAR* szFileName)
{
    TCITEM ti = { 0 };
    ti.mask = TCIF_TEXT;
    ti.pszText = szFileName;
    ti.cchTextMax = wcslen(szFileName);
    TabCtrl_InsertItem(m_tabhWnd, m_tabid, &ti);
    return m_tabid;
}
void Tabs::select_tab(int tabno)
{
    m_active_tab_no = tabno;
    m_active_tab = m_tabs[tabno];
    TabCtrl_SetCurSel(m_tabhWnd, tabno);

    Mode mode = m_active_tab->get_current_mode();
    m_active_tab->change_view();

    if (mode == Mode::TEXT) {
        m_active_tab->resize_view(m_width, m_height, m_x, m_y);
        m_editview->reset_usp_cache();
        m_editview->RefreshWindow();
        m_active_tab->m_docmgr->RepositionCaret();
    } else {
        //m_active_tab->m_docmgr->hide_caret();
        m_active_tab->resize_view(m_width, m_height, m_x, m_y);
        hide_webviews();
    }
}
void Tabs::on_select_tab(HWND hwnd)
{
    int tabno = TabCtrl_GetCurSel(hwnd);
    select_tab(tabno);
}
void Tabs::hide_webviews()
{
    for (int i = 0; i < m_tabs.size(); i++) {
        if (m_tabs[i]->m_tabid == m_active_tab_no) {
            m_tabs[i]->send_select_msg_webview(true);
        } else {
            m_tabs[i]->send_select_msg_webview(false);
        }
    }
}
void Tabs::resize_view(int width, int height, int x, int y)
{
    MoveWindow(m_tabhWnd, x, y, width, height, TRUE);
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    TabCtrl_AdjustRect(m_tabhWnd, FALSE, &rc);

    m_width = width;
    m_height = height - rc.top;
    m_x = x;
    m_y = y + rc.top;
    m_active_tab->resize_view(m_width, m_height, m_x, m_y);
}
void Tabs::close_all_tabs()
{
    for (int i = 0; i < m_tabs.size(); i++) {
        delete m_tabs[i];
    }
    m_tabs.clear();
}
