#include "ContMgr.h"
#include "TextEditor.h"

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
void ContMgr::set_resize_containers(HDWP hdwp, int width, int height)
{
    int contsize = m_containers.size();
    int contwidth = width / contsize;
    int xpos = 0;

    for (int i = 0; i < contsize; i++) {
        m_containers[i].tabs->m_active_tab->resize_view(hdwp, contwidth, height, xpos, 0);
        xpos += contwidth;
    }
}

void ContMgr::open_file_container(int idx, TCHAR* szFileName)
{
    m_containers[idx].txteditor->OpenFile(szFileName);
}
LONG ContMgr::send_msg_container(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_containers.size() > 0) {
        for (int i = 0; i < m_containers.size(); i++) {
            if (hwnd == m_containers[i].wndhwnd) {
                return m_containers[i].txteditor->WndProc(hwnd, msg, wParam, lParam);
            }
        }
    }
    return 0;
}
void ContMgr::send_resize_msg_webview(HWND hwnd)
{
    if (m_containers.size() > 0) {
        for (int i = 0; i < m_containers.size(); i++) {
            if (hwnd == m_containers[i].webvhwnd) {
                if (m_containers[i].tabs->m_active_tab->m_webmgr)
                    m_containers[i].tabs->m_active_tab->resize_webview();
            }
        }
    }
}
void ContMgr::change_webview()
{
    m_containers[m_active_cont_no].tabs->m_active_tab->change_webview();
}
void ContMgr::change_txtview()
{
    m_containers[m_active_cont_no].tabs->m_active_tab->change_txtview();
}
