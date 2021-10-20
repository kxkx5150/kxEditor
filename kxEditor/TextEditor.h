﻿#pragma once
#include "ContMgr.h"
#include "DocMgr.h"
#include "EditView.h"
#include "Tabs.h"
#include "NodeMgr.h"
#include <crtdbg.h>
#include <tchar.h>
#include "kxEditor_define.h"

class Tabs;
class DocMgr;
class NodeMgr;
class CmdMgr;
class ContMgr;
class WebView;

struct EditorContainer {
    TextEditor* txteditor;
    EditView* editview;
    WebView* webview;
    Tabs* tabs;
    HWND wndhwnd;
    HWND webvhwnd;
};
struct FONT {
    HFONT hFont;
    TEXTMETRIC tm;
    int nInternalLeading;
    int nDescent;
};
static const TCHAR* CtrlStr(DWORD ch)
{
    static const TCHAR* reps[] = {
        _T("NUL"), _T("SOH"), _T("STX"), _T("ETX"), _T("EOT"), _T("ENQ"), _T("ACK"), _T("BEL"),
        _T("BS"), _T("HT"), _T("LF"), _T("VT"), _T("FF"), _T("CR"), _T("SO"), _T("SI"),
        _T("DLE"), _T("DC1"), _T("DC2"), _T("DC3"), _T("DC4"), _T("NAK"), _T("SYN"), _T("ETB"),
        _T("CAN"), _T("EM"), _T("SUB"), _T("ESC"), _T("FS"), _T("GS"), _T("RS"), _T("US")
    };

    return ch < _T(' ') ? reps[ch] : _T("???");
}

extern BOOL g_fShowScrollH;
extern NodeMgr* m_nodemgr;

class TextEditor {
    friend class EditView;
    friend class DocMgr;

private:
    HWND m_hWnd;
    Tabs* m_tabs = nullptr;
    CmdMgr* m_cmdmgr = nullptr;

    EditView* m_editview = nullptr;
    WebView* m_webview = nullptr;

    HWND m_hWnd_txtedit;
    HWND m_hwndwebview;


private:
    // font
    int m_nNumFonts = 1;
    FONT m_FontAttr[MAX_FONTS];
    USPFONT m_uspFontList[MAX_FONTS];

    int m_nFontWidth = 0;
    int m_nHeightAbove = 0;
    int m_nHeightBelow = 0;
    int m_nLineHeight = 0;
    int m_nLinenoWidth = 0;
    int m_nMaxAscent = 0;
    int m_nFontHeight = 0;

public:
    // scroll
    UINT m_nScrollTimer = 0;
    int m_nScrollCounter;

public:
    // settings
    int m_tabwidth = 4;
    int m_nCRLFMode = TXL_ALL;
    int m_nEditMode = MODE_INSERT;

    HFONT g_hFont;
    LONG g_nFontSize = 10;
    BOOL g_fFontBold = false;
    TCHAR g_szFontName[LF_FACESIZE] = _T("MS Gothic");
    LONG g_nFontSmoothing = DEFAULT_QUALITY;

public:
    TextEditor(HWND hwnd, CmdMgr* cmdmgr);
    ~TextEditor();
    EditorContainer create_editor_container();

    void LoadRegSettings();
    void ApplyRegSettings();
    LONG WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LONG OpenFile(TCHAR* szFileName);
    void set_focus();

public:
    long long GetLineCount();
    long long longestline();

public:
    //keyboard
    LONG OnChar(UINT nChar, UINT nFlags);
    LONG OnKeyDown(UINT nKeyCode, UINT nFlags);
    ULONG keydown_text(TCHAR* szText, ULONG nLength);
    ULONG keydown_enter(TCHAR* szText, ULONG nLength);
    ULONG keydown_backspace();
    ULONG keydown_delete();
    ULONG move_caret(int key);

public:
    //copy & paste
    LONG OnPaste();

public:
    // font
    HFONT CreateEditorFont(int nPointSize, BOOL fBold, DWORD dwQuality, TCHAR* szFace);
    int PointsToLogical(int nPointSize);
    LONG OnSetFont();
    LONG SetFont(HFONT hFont, int idx);
    void InitCtrlCharFontAttr(HDC hdc, FONT* font);
    void RecalcLineHeight();
    void UpdateMarginWidth();

    int TextWidth(HDC hdc, TCHAR* buf, int len);
    int TabWidth();
    void UpdateMetrics();
    int CtrlCharWidth(HDC hdc, long long chValue, FONT* font);

public:
    //soroll
    VOID SetupScrollbars();
    LONG OnSize(UINT nFlags, int width, int height);
    bool PinToBottomCorner();
    LONG OnVScroll(UINT nSBCode, UINT nPos);
    LONG OnHScroll(UINT nSBCode, UINT nPos);
    HRGN Scroll(int dx, int dy, bool fReturnUpdateRgn = false);
    LONG OnMouseWheel(int nDelta);
    LONG OnTimer(UINT nTimerId);

public:
    //mouse
    LONG OnSetFocus(HWND hwndOld);
    LONG OnKillFocus(HWND hwndNew);
    LONG OnMouseActivate(HWND hwndTop, UINT nHitTest, UINT nMessage);
    LONG OnLButtonDown(UINT nFlags, int mx, int my);
    LONG OnLButtonUp(UINT nFlags, int mx, int my);
    LONG OnMouseMove(UINT nFlags, int mx, int my);
};