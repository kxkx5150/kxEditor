#include "ContMgr.h"
#include "TextEditor.h"
#include <CommCtrl.h>

ContMgr::ContMgr(NodeMgr* nodemgr)
{
    m_nodemgr = nodemgr;
    m_cmdmgr = new CmdMgr(nodemgr,this);
}
ContMgr::~ContMgr()
{
    delete m_cmdmgr;
    delete_all_editor_container();
}
void ContMgr::create_editor_container(HWND hwnd)
{
    if (!m_mainhwnd) {
        m_mainhwnd = hwnd;
        m_hwndStatusbar = CreateStatusBar(hwnd);
    }

    TextEditor* g_ptv = new TextEditor(hwnd, m_cmdmgr);
    EditorContainer econt = g_ptv->create_editor_container(m_containers.size());
    m_containers.push_back(econt);
    SetWindowLongPtr(econt.txthwnd, 0, (LONG)g_ptv);
}
void ContMgr::delete_editor_container(int idx)
{
    delete m_containers[idx].txteditor;
    delete m_containers[idx].editview;
    delete m_containers[idx].webview;
    delete m_containers[idx].tabs;
}
void ContMgr::delete_all_editor_container()
{
    for (int i = 0; i < m_containers.size(); i++) {
        delete_editor_container(i);
    }
    m_containers.clear();
}
void ContMgr::set_focus_container(int idx)
{
    m_containers[idx].txteditor->set_focus();
}
void ContMgr::open_file_container(int idx, TCHAR* szFileName)
{
    m_containers[idx].txteditor->OpenFile(szFileName);
}
void ContMgr::change_webview()
{
    m_containers[m_active_cont_no].tabs->m_active_tab->change_webview();
    m_containers[m_active_cont_no].tabs->hide_webviews();
}
void ContMgr::change_cmdview()
{
    m_containers[m_active_cont_no].tabs->m_active_tab->change_cmdview();
    m_containers[m_active_cont_no].tabs->hide_webviews();
}
void ContMgr::change_txtview()
{
    m_containers[m_active_cont_no].tabs->m_active_tab->change_txtview();
}
LONG ContMgr::send_msg_container(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL focusflg = false;
    if (msg == WM_SETFOCUS)
        focusflg = true;

    for (int i = 0; i < m_containers.size(); i++) {
        if (hwnd == m_containers[i].txthwnd) {
            if (focusflg)
                m_active_cont_no = i;
            return m_containers[i].txteditor->WndProc(i, hwnd, msg, wParam, lParam);
        }
    }

    return 0;
}
void ContMgr::resize_statusbar(int width, int height)
{
    if (!m_hwndStatusbar)
        return;
    RECT rect;
    GetWindowRect(m_hwndStatusbar, &rect);
    int heightsb = rect.bottom - rect.top;

    if (true) {
        MoveWindow(m_hwndStatusbar, 0, height - heightsb, width, heightsb, TRUE);
        SetStatusBarParts(m_hwndStatusbar);
        height -= heightsb;
    } else {
        MoveWindow(m_hwndStatusbar, 0, height, width, 0, TRUE);
    }

    ShowWindow(m_hwndStatusbar, SW_SHOW);
}
void ContMgr::send_resize_msg_containers(int width, int height, int x, int y)
{
    resize_statusbar(width, height);
    int contsize = m_containers.size();
    if (contsize < 1)
        return;

    int contwidth = width / contsize;
    for (int i = 0; i < contsize; i++) {
        m_containers[i].tabs->resize_view(contwidth, height, x, y);
        x += contwidth;
    }
}
void ContMgr::send_resize_msg_textview(HWND hwnd)
{
    for (int i = 0; i < m_containers.size(); i++) {
        if (hwnd == m_containers[i].txthwnd) {
            m_containers[i].tabs->m_active_tab->resize_textview();
        }
    }
}
void ContMgr::send_resize_msg_webview(HWND hwnd)
{
    for (int i = 0; i < m_containers.size(); i++) {
        if (hwnd == m_containers[i].webvhwnd) {
            m_containers[i].tabs->m_active_tab->resize_webview();
        }
    }
}
void ContMgr::on_select_tab(HWND hwnd)
{
    m_containers[m_active_cont_no].tabs->on_select_tab(hwnd);
}
void ContMgr::split_vertical()
{
    _RPTN(_CRT_WARN, "active cont : %d\n", m_active_cont_no);
    //create_editor_container(m_mainhwnd, m_cmdmgr);

    //for (int i = 0; i < m_containers.size(); i++) {
    //    InvalidateRect(m_containers[i].prnthwnd, NULL, FALSE);
    //}
}
