#pragma once
#include "StatusBar.h"
#include <vector>
#include <windows.h>

class TextEditor;
class editview;
class tabs;
class CmdMgr;
class NodeMgr;
struct EditorContainer;

class ContMgr {

public:
    HWND m_mainhwnd;
    HWND m_hwndStatusbar;

    NodeMgr* m_nodemgr = nullptr;
    CmdMgr* m_cmdmgr = nullptr;

    std::vector<EditorContainer> m_containers;
    int m_active_cont_no = 0;

public:
    ContMgr(NodeMgr* m_nodemgr);
    ~ContMgr();

    void create_editor_container(HWND hwnd, RECT rect);
    void delete_editor_container(int idx);
    void delete_all_editor_container();

    void set_active_container(int idx);
    void set_focus_container(int idx);
    void open_file_container(int idx, TCHAR* szFileName);

    void send_resize_msg_containers(int width, int height, int x, int y);
    void send_resize_msg_textview(HWND hwnd);
    void send_resize_msg_webview(HWND hwnd);
    int resize_statusbar(int width, int height);

    void on_select_tab(HWND hwnd);
    void check_active_tabctrl(HWND hwnd);

    void change_webview();
    void change_cmdview();
    void change_txtview();

    void split_vertical();
    void split_horizontal();



};
