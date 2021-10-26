#pragma once
#include <deque>
#include "Tab.h"
#include "TextEditor.h"

class EditView;
class Tab;
class TextEditor;
class WebView;

class Tabs {
    static int m_tabid;

private:
    std::deque<Tab*> m_tabs;
    HWND m_hwnd;
    HWND m_tabhWnd;
    HWND m_txthWnd;
    HWND m_webhwnd;

    TextEditor* m_txteditr = nullptr;
    EditView* m_editview = nullptr;
    WebView* m_webeditr = nullptr;

public:
    Tab* m_active_tab = nullptr;
    int m_active_tab_no = 0;

    int m_width = 0;
    int m_height = 0;
    int m_x = 0;
    int m_y = 0;

public:
    Tabs();
    ~Tabs();
    void init_tabs(HWND hWnd, HWND tabhWnd, TextEditor* txteditr, HWND txthWnd, EditView* editview, HWND whWnd, WebView* webview);
    LONG create_tab(TCHAR* szFileName = nullptr);
    int create_tab_control(TCHAR* szFileName);
    void select_tab(int tabno);
    void resize_view(int width, int height, int x, int y);
    void close_all_tabs();

    void on_select_tab(HWND hwnd);
    void hide_webviews();

};
