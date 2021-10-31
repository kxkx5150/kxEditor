#include "ContMgr.h"
#include "TextEditor.h"
#include <CommCtrl.h>

ContMgr::ContMgr(NodeMgr* nodemgr)
{
    m_nodemgr = nodemgr;
    m_cmdmgr = new CmdMgr(nodemgr, this);
}
ContMgr::~ContMgr()
{
    delete m_cmdmgr;
    delete_all_editor_container();
}
void ContMgr::create_editor_container(HWND hwnd, RECT rect)
{
    if (!m_mainhwnd) {
        m_mainhwnd = hwnd;
        m_hwndStatusbar = CreateStatusBar(hwnd);
        int sheight = resize_statusbar(rect.right - rect.left, rect.bottom - rect.top);
        rect.bottom -= sheight + 1;
    }

    TextEditor* g_ptv = new TextEditor(hwnd, m_cmdmgr);
    EditorContainer econt = g_ptv->create_editor_container(this, m_containers.size(), rect);
    m_containers.push_back(econt);
    SetWindowLongPtr(econt.txthwnd, 0, (LONG)g_ptv);
    SetWindowLongPtr(econt.webvhwnd, 0, (LONG)econt.webview);
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
void ContMgr::set_active_container(int idx)
{
    _RPTN(_CRT_WARN, "active cont no :%d\n", idx);
    m_active_cont_no = idx;
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

int ContMgr::resize_statusbar(int width, int height)
{
    if (!m_hwndStatusbar)
        return 0;
    RECT rect;
    GetWindowRect(m_hwndStatusbar, &rect);
    int heightsb = rect.bottom - rect.top;

    if (true) {
        MoveWindow(m_hwndStatusbar, 0, height - heightsb, width, heightsb, TRUE);
        SetStatusBarParts(m_hwndStatusbar);
    } else {
        MoveWindow(m_hwndStatusbar, 0, height, width, 0, TRUE);
    }

    ShowWindow(m_hwndStatusbar, SW_SHOW);
    return heightsb;
}
void ContMgr::send_resize_msg_containers(int width, int height, int x, int y)
{
    int size = m_containers.size();
    if (size < 1)
        return;

    for (int i = 0; i < size; i++) {
        auto rect = m_containers[i].rect;
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        m_containers[i].tabs->resize_view(width, height, rect.left, rect.top);
    }
}
void ContMgr::send_resize_msg_textview(HWND hwnd)
{
    for (int i = 0; i < m_containers.size(); i++) {
        if (hwnd == m_containers[i].txthwnd) {
            m_containers[i].tabs->m_active_tab->resize_textview();
            break;
        }
    }
}
void ContMgr::send_resize_msg_webview(HWND hwnd)
{
    for (int i = 0; i < m_containers.size(); i++) {
        if (hwnd == m_containers[i].webvhwnd) {
            m_containers[i].tabs->m_active_tab->resize_webview();
            break;
        }
    }
}
void ContMgr::on_select_tab(HWND hwnd)
{
    check_active_tabctrl(hwnd);
    m_containers[m_active_cont_no].tabs->on_select_tab(hwnd);
}
void ContMgr::check_active_tabctrl(HWND hwnd)
{
    for (int i = 0; i < m_containers.size(); i++) {
        if (hwnd == m_containers[i].tabhwnd) {
            set_active_container(i);
            break;
        }
    }
}
void ContMgr::split_vertical()
{
    RECT erect = m_containers[m_active_cont_no].rect;
    RECT rect;

    erect.right = erect.right / 2;

    rect.top = erect.top;
    rect.bottom = erect.bottom;
    rect.right = erect.right + erect.right;
    rect.left = erect.right;

    m_containers[m_active_cont_no].rect = erect;
    create_editor_container(m_mainhwnd, rect);
}
void ContMgr::split_horizontal()
{
    RECT erect = m_containers[m_active_cont_no].rect;
    RECT rect;

    erect.bottom = erect.bottom / 2;

    rect.top = erect.bottom;
    rect.bottom = erect.bottom + erect.bottom;
    rect.right = erect.right;
    rect.left = erect.left;

    m_containers[m_active_cont_no].rect = erect;
    create_editor_container(m_mainhwnd, rect);
}