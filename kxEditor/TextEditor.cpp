#include "TextEditor.h"
#include "WebView.h"

TextEditor::TextEditor(HWND hwnd, CmdMgr* cmdmgr)
{
    m_hWnd = hwnd;
    m_cmdmgr = cmdmgr;
    memset(m_uspFontList, 0, sizeof(m_uspFontList));
    LoadRegSettings();
    ApplyRegSettings();
    OnSetFont();
}
TextEditor::~TextEditor()
{
    DeleteObject(g_hFont);
}
EditorContainer TextEditor::create_editor_container(ContMgr* contmgr, int contno)
{
    m_hwnd_tabctrl = CreateTabControl(m_hWnd);
    m_hWnd_txtedit = CreateTextView(m_hwnd_tabctrl);
    m_hwnd_webview = CreateWebView(m_hwnd_tabctrl);

    m_contno = contno;
    m_contmgr = contmgr;
    m_tabs = new Tabs();
    m_editview = new EditView(m_hWnd_txtedit, this, m_tabs);
    m_webview = new WebView(m_hWnd, m_hwnd_webview, contno);

    m_cmdmgr->set_hwnd(m_hWnd_txtedit);
    m_tabs->init_tabs(m_hWnd, m_hwnd_tabctrl, this, m_hWnd_txtedit, m_editview, m_hwnd_webview, m_webview);

    EditorContainer econt;
    econt.prnthwnd = m_hWnd;
    econt.tabhwnd = m_hwnd_tabctrl;
    econt.txthwnd = m_hWnd_txtedit;
    econt.webvhwnd = m_hwnd_webview;
    econt.txteditor = this;
    econt.editview = m_editview;
    econt.webview = m_webview;
    econt.tabs = m_tabs;
    return econt;
}
LONG TextEditor::OpenFile(TCHAR* szFileName)
{
    m_tabs->m_active_tab->m_docmgr->m_nVScrollPos = 0;
    m_tabs->m_active_tab->m_docmgr->m_nHScrollPos = 0;
    return m_tabs->create_tab(szFileName);
}
long long TextEditor::GetLineCount()
{
    if (m_tabs && m_tabs->m_active_tab && m_tabs->m_active_tab->m_Document)
        return m_tabs->m_active_tab->m_Document->linecount();
    else
        return 0;
}
void TextEditor::set_focus()
{
    SetFocus(m_hWnd_txtedit);
}
void TextEditor::refres_edit_view()
{
    m_editview->OnPaint();
}

LONG WINAPI TextEditor::WndProc(int contno, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

    case WM_PAINT:
        if (m_editview)
            return m_editview->OnPaint();
        else
            return 0;

    case WM_SETFONT:
        return OnSetFont();

    case WM_SIZE:
        return OnSize(wParam, LOWORD(lParam), HIWORD(lParam));

    case TXM_OPENFILE:
        return OpenFile((TCHAR*)lParam);

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
            return TRUE;
        else
            break;

    case WM_VSCROLL:
        return OnVScroll(LOWORD(wParam), HIWORD(wParam));

    case WM_HSCROLL:
        return OnHScroll(LOWORD(wParam), HIWORD(wParam));

    case WM_MOUSEACTIVATE:
        return OnMouseActivate((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_MOUSEWHEEL:
        return OnMouseWheel((short)HIWORD(wParam));

    case WM_SETFOCUS:
        OutputDebugString(L"fouces\n");
        m_contmgr->set_active_container(contno);
        return OnSetFocus((HWND)wParam);

    case WM_KILLFOCUS:
        return OnKillFocus((HWND)wParam);

    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

    case WM_MOUSEMOVE:
        return OnMouseMove(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

    case WM_TIMER:
        return OnTimer(wParam);

        //case WM_PASTE:
        //    return OnPaste();

    case WM_CHAR:
        return m_cmdmgr->OnChar(contno, wParam, lParam);

    case WM_KEYDOWN:
        return m_cmdmgr->on_keydown(contno, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
};
