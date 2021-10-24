#include "Tabs.h"
#include "EditView.h"

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
    auto tab = new Tab(m_hwnd, m_tabhWnd, m_txthWnd, m_txteditr, m_editview, m_webhwnd, m_webeditr, m_tabid);
    m_tabs.push_back(tab);
    select_tab(m_tabid);
    return m_tabid;
}
void Tabs::select_tab(int tabno)
{
    m_active_tab_no = tabno;
    m_active_tab = m_tabs[tabno];
}
void Tabs::close_all_tabs()
{
    for (int i = 0; i < m_tabs.size(); i++) {
        delete m_tabs[i];
    }
    m_tabs.clear();
}
