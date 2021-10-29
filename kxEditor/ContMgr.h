#pragma once
#include <vector>
#include <windows.h>
#include "StatusBar.h"

class TextEditor;
class editview;
class tabs;
class CmdMgr;
struct EditorContainer;

class ContMgr {

public:
    HWND m_mainhwnd;
    HWND m_hwndStatusbar;
    CmdMgr* m_cmdmgr = nullptr;

    std::vector<EditorContainer> m_containers;
    int m_active_cont_no = 0;


public:
    ContMgr();
    ~ContMgr();

    void create_editor_container(HWND hwnd);
    void delete_editor_container(int idx);
    void delete_all_editor_container();
    void set_focus_container(int idx);
    void open_file_container(int idx, TCHAR* szFileName);
    LONG send_msg_container(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void send_resize_msg_containers(int width, int height, int x, int y);
    void send_resize_msg_textview(HWND hwnd);
    void send_resize_msg_webview(HWND hwnd);
    void resize_statusbar(int width, int height);

    void on_select_tab(HWND hwnd);

    void change_webview();
    void change_cmdview();
    void change_txtview();

    void split_vertical();


private:
};
