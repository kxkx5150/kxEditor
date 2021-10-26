#include "ContMgr.h"
#include "TextEditor.h"
#include <CommCtrl.h>

ContMgr::ContMgr()
{
}
ContMgr::~ContMgr()
{
    delete_all_editor_container();
}
void ContMgr::create_editor_container(HWND hwnd, CmdMgr* cmdmgr)
{
    TextEditor* g_ptv = new TextEditor(hwnd, cmdmgr);
    EditorContainer econt = g_ptv->create_editor_container();
    m_cmdmgr = cmdmgr;
    m_containers.push_back(econt);
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
    if (m_containers.size() > 0) {
        for (int i = 0; i < m_containers.size(); i++) {
            if (hwnd == m_containers[i].txthwnd) {
                return m_containers[i].txteditor->WndProc(hwnd, msg, wParam, lParam);
            }
        }
    }
    return 0;
}
void ContMgr::send_resize_msg_containers(int width, int height, int x, int y)
{
    int contsize = m_containers.size();
    if (contsize < 1)
        return;
    int contwidth = width / contsize;

    for (int i = 0; i < contsize; i++) {
        m_containers[i].tabs->resize_view(contwidth, height, x,y);
        x += contwidth;
    }
}
void ContMgr::send_resize_msg_textview(HWND hwnd)
{
    if (m_containers.size() > 0) {
        for (int i = 0; i < m_containers.size(); i++) {
            if (hwnd == m_containers[i].txthwnd) {
                m_containers[i].tabs->m_active_tab->resize_textview();
            }
        }
    }
}
void ContMgr::send_resize_msg_webview(HWND hwnd)
{
    if (m_containers.size() > 0) {
        for (int i = 0; i < m_containers.size(); i++) {
            if (hwnd == m_containers[i].webvhwnd) {
                m_containers[i].tabs->m_active_tab->resize_webview();
            }
        }
    }
}
void ContMgr::on_select_tab(HWND hwnd)
{
    m_containers[m_active_cont_no].tabs->on_select_tab(hwnd);
}
