#include "TextEditor.h"

LONG TextEditor::OnSetFont()
{
    if (g_hFont)
        DeleteObject(g_hFont);

    g_hFont = CreateEditorFont(g_nFontSize, g_fFontBold, g_nFontSmoothing, g_szFontName);
    SetFont(g_hFont, 0);
    UpdateMetrics();
    return 0;
}
LONG TextEditor::SetFont(HFONT hFont, int idx)
{
    USPFONT* uspFont = &m_uspFontList[idx];
    HDC hdc = GetDC(m_hWnd);
    UspFreeFont(uspFont);
    UspInitFont(uspFont, hdc, hFont);
    ReleaseDC(m_hWnd, hdc);
    m_nFontWidth = m_uspFontList[0].tm.tmAveCharWidth;
    RecalcLineHeight();
    UpdateMarginWidth();
    if (m_editview)
        m_editview->reset_usp_cache();

    return 0;
}
HFONT TextEditor::CreateEditorFont(int nPointSize, BOOL fBold, DWORD dwQuality, TCHAR* szFace)
{
    return CreateFont(PointsToLogical(nPointSize),
        0, 0, 0,
        fBold ? FW_BOLD : 0,
        0, 0, 0, 0, 0, 0,
        dwQuality,
        0,
        szFace);
}
int TextEditor::PointsToLogical(int nPointSize)
{
    HDC hdc = GetDC(0);
    int nLogSize = -MulDiv(nPointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(0, hdc);
    return nLogSize;
}
void TextEditor::InitCtrlCharFontAttr(HDC hdc, FONT* font)
{
    HDC hdcTemp = CreateCompatibleDC(hdc);
    HBITMAP hbmTemp = CreateBitmap(font->tm.tmAveCharWidth, font->tm.tmHeight, 1, 1, 0);
    HANDLE hbmOld = SelectObject(hdcTemp, hbmTemp);
    HANDLE hfnOld = SelectObject(hdcTemp, font->hFont);

    SetTextColor(hdcTemp, RGB(0, 0, 0));
    SetBkColor(hdcTemp, RGB(255, 255, 255));
    SetBkMode(hdcTemp, OPAQUE);
    TextOut(hdcTemp, 0, 0, _T("E"), 1);

    font->nInternalLeading = font->tm.tmInternalLeading;
    font->nDescent = font->tm.tmDescent;

    for (int y = 0; y < font->tm.tmHeight; y++) {
        for (int x = 0; x < font->tm.tmAveCharWidth; x++) {
            COLORREF col;

            if ((col = GetPixel(hdcTemp, x, y)) == RGB(0, 0, 0)) {
                font->nInternalLeading = y;
                y = font->tm.tmHeight;
                break;
            }
        }
    }

    for (int y = font->tm.tmHeight - 1; y >= 0; y--) {
        for (int x = 0; x < font->tm.tmAveCharWidth; x++) {
            COLORREF col;

            if ((col = GetPixel(hdcTemp, x, y)) == RGB(0, 0, 0)) {
                font->nDescent = font->tm.tmHeight - y - 1;
                y = 0;
                break;
            }
        }
    }

    if (font->nInternalLeading > 1 && font->nDescent > 1 && font->tm.tmHeight > 18) {
        font->nInternalLeading--;
        font->nDescent--;
    }

    // cleanup
    SelectObject(hdcTemp, hbmOld);
    SelectObject(hdcTemp, hfnOld);
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
}

VOID TextEditor::RecalcLineHeight()
{
    m_nLineHeight = 0;
    m_nMaxAscent = 0;

    for (int i = 0; i < m_nNumFonts; i++) {
        int fontheight = m_uspFontList[i].tm.tmHeight + m_uspFontList[i].tm.tmExternalLeading;
        m_nLineHeight = max(m_nLineHeight, fontheight);
        m_nMaxAscent = max(m_nMaxAscent, m_uspFontList[i].tm.tmAscent);
    }

    m_nLineHeight += m_nHeightAbove + m_nHeightBelow;
    if (GetFocus() == m_hWnd) {
        OnKillFocus(0);
        OnSetFocus(0);
    }
}
int TextEditor::TextWidth(HDC hdc, TCHAR* buf, int len)
{
    SIZE sz;
    if (len == -1)
        len = lstrlen(buf);
    GetTextExtentPoint32(hdc, buf, len, &sz);
    return sz.cx;
}
void TextEditor::UpdateMetrics()
{
    RECT rect;
    GetClientRect(m_hWnd_txtedit, &rect);
    OnSize(0, rect.right, rect.bottom);
    if (m_editview != nullptr)
        m_editview->RefreshWindow();
}
void TextEditor::UpdateMarginWidth()
{
    HDC hdc = GetDC(m_hWnd);
    HANDLE hOldFont = SelectObject(hdc, m_uspFontList[0].hFont);
    TCHAR buf[32];
    int len = wsprintf(buf, LINENO_FORMAT, (int)GetLineCount());
    m_nLinenoWidth = TextWidth(hdc, buf, len);
    SelectObject(hdc, hOldFont);
    ReleaseDC(m_hWnd, hdc);
}
int TextEditor::TabWidth()
{
    return m_tabwidth * m_nFontWidth;
}
int TextEditor::CtrlCharWidth(HDC hdc, long long chValue, FONT* font)
{
    SIZE sz;
    const TCHAR* str = CtrlStr(chValue % 32);
    GetTextExtentPoint32(hdc, str, _tcslen(str), &sz);
    return sz.cx + 4;
}
