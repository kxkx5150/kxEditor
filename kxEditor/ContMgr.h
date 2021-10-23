#pragma once
#include <vector>
#include <windows.h>

class TextEditor;
class editview;
class tabs;
class CmdMgr;
struct EditorContainer;

class ContMgr {

public:
    std::vector<EditorContainer> m_containers;
    int m_active_cont_no = 0;

private:
    CmdMgr* m_cmdmgr = nullptr;


public:
    ContMgr();
    ~ContMgr();

    void create_editor_container(HWND hwnd, CmdMgr* cmdmgr);
    void delete_editor_container(int idx);
    void delete_all_editor_container();
    void set_focus_container(int idx);
    void send_resize_msg_containers(HDWP hdwp, int width, int height, int x, int y);
    void open_file_container(int idx, TCHAR* szFileName);
    LONG send_msg_container(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void send_resize_msg_textview(HWND hwnd);
    void send_resize_msg_webview(HWND hwnd);
    void change_webview();
    void change_cmdview();
    void change_txtview();


private:
};
