#include "Tabs.h"
#include "EditView.h"

int Tabs::m_tabid = 0;

Tabs::Tabs()
{
}
Tabs::~Tabs()
{
    close_all_tabs();
}
void Tabs::init_tabs(HWND hWnd, TextEditor* txteditr, HWND txthWnd, EditView* editview, HWND whWnd, WebView* webview)
{
    m_hwnd = hWnd;
    m_txteditr = txteditr;
    m_editview = editview;
    m_webeditr = webview;

    m_txthWnd = txthWnd;
    m_webhwnd = whWnd;
    m_tabs.push_back(create_tab());
    select_tab(0);
}
Tab* Tabs::create_tab()
{
    m_tabid++;
    return new Tab(m_hwnd, m_txthWnd, m_txteditr, m_editview, m_webhwnd, m_webeditr, m_tabid);
}
void Tabs::select_tab(int tabno)
{
    m_active_tab = m_tabs[tabno];
}
void Tabs::close_all_tabs()
{
    for (int i = 0; i < m_tabs.size(); i++) {
        delete m_tabs[i];
    }
    m_tabs.clear();
}
