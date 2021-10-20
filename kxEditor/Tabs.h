#pragma once
#include "Tab.h"
#include "TextEditor.h"
#include <deque>

class EditView;
class Tab;
class TextEditor;
class WebView;

class Tabs {
    static int m_tabid;

private:
    std::deque<Tab*> m_tabs;
    HWND m_hwnd;
    HWND m_txthWnd;
    HWND m_webhwnd;

    TextEditor* m_txteditr = nullptr;
    EditView* m_editview = nullptr;
    WebView* m_webeditr = nullptr;

public:
    Tab* m_active_tab = nullptr;
    int m_active_tab_no = 0;


public:
    Tabs();
    ~Tabs();
    void init_tabs(HWND hWnd, TextEditor* txteditr, HWND txthWnd, EditView* editview, HWND whWnd, WebView* webview);
    Tab* create_tab();
    void select_tab(int tabno);
    void close_all_tabs();

};
