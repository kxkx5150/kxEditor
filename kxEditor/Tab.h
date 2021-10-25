#pragma once
#include "Tabs.h"
#include <windows.h>

class EditView;
class Tab;
class TextEditor;
class LinkedList;
class DocMgr;
class WebView;
class WebMgr;

enum class Mode {
    TEXT,
    TERMINAL,
    WEBVIEW
};

class Tab {
    HWND m_hwnd;
    HWND m_tabhWnd;
    HWND m_txthWnd;
    HWND m_webhwnd;

public:
    TextEditor* m_txteditr = nullptr;
    EditView* m_editview = nullptr;
    LinkedList* m_Document = nullptr;
    DocMgr* m_docmgr = nullptr;
    WebView* m_webeditr = nullptr;
    WebMgr* m_webmgr = nullptr;

    Mode m_mode = Mode::TEXT;

private:
    int m_tabid = 0;

    int m_width = 0;
    int m_height = 0;
    int m_x = 0;
    int m_y = 0;

public:
    Tab(TCHAR* szFileName,HWND hWnd, HWND tabhWnd, HWND txthwnd, 
        TextEditor* txtee, EditView* editview, HWND whWnd, WebView* webview, int tabid, Mode mode);
    ~Tab();

    LONG create(TCHAR* szFileName, Mode mode = Mode::TEXT);
    LONG create_file(TCHAR* szFileName = nullptr);
    void create_webview();

    LONG init_file(TCHAR* szFileName);
    LONG create_emptyfile();
    void close_document();

    void set_size(int width, int height, int x, int y);
    void resize_view(HDWP hdwp, int width, int height, int x, int y);
    void change_webview();
    void change_cmdview();
    void change_txtview();
    void change_view();
    void resize_textview();
    void resize_webview();

};
