#include "EditView.h"

EditView::EditView(HWND hwnd, TextEditor* txteditr, Tabs* tabs)
{
    m_hWnd = hwnd;
    m_txteditr = txteditr;
    m_tabs = tabs;

    m_buff = new TCHAR[MAX_SIZE];
    m_attr = new ATTR[MAX_SIZE];

    m_uspCache = new USPCACHE[USP_CACHE_SIZE];
    for (int i = 0; i < USP_CACHE_SIZE; i++) {
        m_uspCache[i].usage = 0;
        m_uspCache[i].lineno = 0;
        m_uspCache[i].uspData = UspAllocate();
    }
}
EditView::~EditView()
{
    delete[] m_buff;
    delete[] m_attr;

    for (int i = 0; i < USP_CACHE_SIZE; i++)
        UspFree(m_uspCache[i].uspData);
    delete[] m_uspCache;
}
void EditView::clear_select_pos()
{
    m_paint_sel_stat.lineno = -1;
    m_paint_sel_stat.stridx = -1;

    m_paint_sel_end.lineno = -1;
    m_paint_sel_end.stridx = -1;
}
LONG EditView::OnPaint()
{
    PAINTSTRUCT ps;
    ULONG first;
    ULONG last;

    HRGN hrgnUpdate;
    HDC hdcMem;
    HBITMAP hbmMem;
    RECT rect;

    hrgnUpdate = CreateRectRgn(0, 0, 1, 1);
    GetUpdateRgn(m_hWnd, hrgnUpdate, FALSE);
    BeginPaint(m_hWnd, &ps);
    GetClientRect(m_hWnd, &rect);

    hdcMem = CreateCompatibleDC(ps.hdc);
    hbmMem = CreateCompatibleBitmap(ps.hdc, rect.right - rect.left, m_txteditr->m_nLineHeight);
    SelectObject(hdcMem, hbmMem);

    first = m_tabs->m_active_tab->m_docmgr->m_nVScrollPos + ps.rcPaint.top / m_txteditr->m_nLineHeight;
    last = m_tabs->m_active_tab->m_docmgr->m_nVScrollPos + ps.rcPaint.bottom / m_txteditr->m_nLineHeight;

    if (last < first)
        last = -1;

    for (ULONG i = first; i <= last; i++) {
        int sx = 0;
        int sy = (i - m_tabs->m_active_tab->m_docmgr->m_nVScrollPos) * m_txteditr->m_nLineHeight;
        int width = rect.right - rect.left;
        PaintRect(hdcMem, 0, 0, width, m_txteditr->m_nLineHeight, RGB(0xff, 0xff, 0xff));
        PaintLine(hdcMem, i, -1 * m_tabs->m_active_tab->m_docmgr->m_nHScrollPos * m_txteditr->m_nFontWidth, 0, hrgnUpdate);
        BitBlt(ps.hdc, sx, sy, width, m_txteditr->m_nLineHeight, hdcMem, 0, 0, SRCCOPY);
    }

    EndPaint(m_hWnd, &ps);
    DeleteDC(hdcMem);
    DeleteObject(hbmMem);
    DeleteObject(hrgnUpdate);

    return 0;
}
void EditView::PaintLine(HDC hdc, ULONG nLineNo, int xpos, int ypos, HRGN hrgnUpdate)
{
    RECT bounds;
    HRGN hrgnBounds;

    GetClientRect(m_hWnd, &bounds);
    SelectClipRgn(hdc, NULL);

    if (hrgnUpdate != NULL) {
        bounds.left = (long)(-1 * m_tabs->m_active_tab->m_docmgr->m_nHScrollPos * m_txteditr->m_nFontWidth + LeftMarginWidth());
        bounds.top = (long)((nLineNo - m_tabs->m_active_tab->m_docmgr->m_nVScrollPos) * m_txteditr->m_nLineHeight);
        bounds.right = (long)(bounds.right);
        bounds.bottom = (long)(bounds.top + m_txteditr->m_nLineHeight);

        hrgnBounds = CreateRectRgnIndirect(&bounds);
        CombineRgn(hrgnBounds, hrgnUpdate, hrgnBounds, RGN_AND);
        GetRgnBox(hrgnBounds, &bounds);
        bounds.top = 0;
        bounds.bottom = m_txteditr->m_nLineHeight;
        DeleteObject(hrgnBounds);
    }

    PaintText(hdc, nLineNo, xpos + LeftMarginWidth(), ypos, &bounds);
    SelectClipRgn(hdc, NULL);

    if (LeftMarginWidth() > 0) {
        PaintMargin(hdc, nLineNo, 0, 0);
    }
}
void EditView::PaintText(HDC hdc, ULONG nLineNo, int xpos, int ypos, RECT* bounds)
{
    USPDATA* uspData;
    ULONG lineOffset;
    uspData = GetUspData(hdc, nLineNo, &lineOffset);
    UspSetSelColor(uspData, RGB(0xff, 0xff, 0xff), RGB(0x00, 0x77, 0x77));
    ApplySelection(uspData, nLineNo, lineOffset, uspData->stringLen);
    UspTextOut(uspData, hdc, xpos, ypos, m_txteditr->m_nLineHeight, m_txteditr->m_nHeightAbove, bounds);
}
USPDATA* EditView::GetUspData(HDC hdc, ULONG nLineNo, ULONG* nOffset /*=0*/)
{
    long long colno = 0;
    int len;
    HDC hdcTemp;

    USPDATA* uspData;
    ULONG lru_usage = -1;
    int lru_index = 0;

    for (int i = 0; i < USP_CACHE_SIZE; i++) {
        if (m_uspCache[i].usage < lru_usage) {
            lru_index = i;
            lru_usage = m_uspCache[i].usage;
        }

        if (m_uspCache[i].usage > 0 && m_uspCache[i].lineno == nLineNo) {
            if (nOffset)
                *nOffset = m_uspCache[i].offset;

            m_uspCache[i].usage++;
            return m_uspCache[i].uspData;
        }
    }

    m_uspCache[lru_index].lineno = nLineNo;
    m_uspCache[lru_index].usage = 1;
    uspData = m_uspCache[lru_index].uspData;

    if (hdc == 0)
        hdcTemp = GetDC(m_hWnd);
    else
        hdcTemp = hdc;

    long long off_chars = 0;
    m_tabs->m_active_tab->m_Document->iterater(nLineNo, m_buff, &len);

    m_uspCache[lru_index].offset = off_chars;
    m_uspCache[lru_index].length = len;
    m_uspCache[lru_index].length_CRLF = len - m_tabs->m_active_tab->m_Document->CRLF_size(m_buff, len);

    len = set_textatributes(nLineNo, off_chars, colno, m_buff, len, m_attr);

    int tablist[] = { m_txteditr->m_tabwidth };
    SCRIPT_TABDEF tabdef = { 1, 0, tablist, 0 };
    SCRIPT_CONTROL scriptControl = { 0 };
    SCRIPT_STATE scriptState = { 0 };

    UspAnalyze(
        uspData,
        hdcTemp,
        m_buff,
        len,
        m_attr,
        0,
        m_txteditr->m_uspFontList,
        &scriptControl,
        &scriptState,
        &tabdef);

    if (hdc == 0)
        ReleaseDC(m_hWnd, hdcTemp);

    m_uspCache[lru_index].offset = off_chars;
    if (nOffset)
        *nOffset = off_chars;

    return uspData;
}
int EditView::set_textatributes(long long nLineNo, long long nOffset, long long& nColumn, TCHAR* szText, int nTextLen, ATTR* attr)
{
    for (int i = 0; i < nTextLen; i++) {
        attr[i].len = 1;
        attr[i].font = 0;
        attr[i].eol = 0;
        attr[i].reserved = 0;
        attr[i].fg = RGB(0x22, 0x22, 0x22);
        attr[i].bg = RGB(0xff, 0xff, 0xff);
        attr[i].sel = 0;

        if (szText[i] == '\t')
            nColumn += m_txteditr->m_tabwidth - (nColumn % m_txteditr->m_tabwidth);
        else
            nColumn += 1;
    }

    nTextLen = StripCRLF(szText, attr, nTextLen, false);

    for (int i = 0; i < nTextLen; i++) {
        ULONG ch = szText[i];
        attr[i].ctrl = ch < 0x20 ? 1 : 0;
    }

    return nTextLen;
}
int EditView::StripCRLF(TCHAR* szText, ATTR* attr, int nLength, bool fAllow)
{
    if (nLength >= 2) {
        if (szText[nLength - 2] == '\r' && szText[nLength - 1] == '\n') {
            attr[nLength - 2].eol = TRUE;

            if (m_txteditr->m_nCRLFMode & TXL_CRLF) {
                // convert CRLF to a single space
                szText[nLength - 2] = ' ';
                return nLength - 1 - (int)fAllow;
            } else {
                return nLength;
            }
        }
    }

    if (nLength >= 1) {
        if (szText[nLength - 1] == '\r') {
            attr[nLength - 1].eol = TRUE;

            if (m_txteditr->m_nCRLFMode & TXL_CR) {
                szText[nLength - 1] = ' ';
                return nLength - (int)fAllow;
            }
        }

        if (szText[nLength - 1] == '\n') {
            attr[nLength - 1].eol = TRUE;

            if (m_txteditr->m_nCRLFMode & TXL_LF) {
                szText[nLength - 1] = ' ';
                return nLength - (int)fAllow;
            }
        }
    }

    return nLength;
}
int EditView::ApplySelection(USPDATA* uspData, ULONG nLine, ULONG nOffset, ULONG nTextLen)
{
    int selstart = 0;
    int selend = 0;
    DocMgr* m_docmgr = m_tabs->m_active_tab->m_docmgr;
   
    if (m_docmgr->select_paint_mode()) {
        if (m_docmgr->m_stat_sel.lineno == nLine && m_docmgr->m_end_sel.lineno == nLine) {
            selstart = m_docmgr->m_stat_sel.stridx;
            selend = m_docmgr->m_end_sel.stridx;
        } else {
            if (m_docmgr->m_stat_sel.lineno < nLine && nLine < m_docmgr->m_end_sel.lineno) {
                selstart = 0;
                selend = nTextLen - 1;
            } else if (m_docmgr->m_stat_sel.lineno == nLine) {
                selstart = m_docmgr->m_stat_sel.stridx;
                selend = nTextLen - 1;
            } else if (m_docmgr->m_end_sel.lineno == nLine) {
                selstart = 0;
                selend = m_docmgr->m_end_sel.stridx;
            }
        }
    }
    m_paint_sel_stat = m_docmgr->m_stat_sel;
    m_paint_sel_end = m_docmgr->m_end_sel;
    UspApplySelection(uspData, selend, selstart);
    return 0;
}
int EditView::NeatTextYOffset(USPFONT* font)
{
    return m_txteditr->m_nMaxAscent + m_txteditr->m_nHeightAbove - font->tm.tmAscent;
}
void EditView::PaintRect(HDC hdc, int x, int y, int width, int height, COLORREF fill)
{
    RECT rect = { x, y, x + width, y + height };
    fill = SetBkColor(hdc, fill);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
    SetBkColor(hdc, fill);
}
void EditView::PaintRect(HDC hdc, RECT* rect, COLORREF fill)
{
    fill = SetBkColor(hdc, fill);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, 0, 0, 0);
    SetBkColor(hdc, fill);
}
int EditView::PaintMargin(HDC hdc, ULONG nLineNo, int xpos, int ypos)
{
    RECT rect = { xpos, ypos, xpos + LeftMarginWidth(), ypos + m_txteditr->m_nLineHeight };
    int selwidth = 0;

    TCHAR ach[32];

    if (true) {
        HANDLE hOldFont = SelectObject(hdc, m_txteditr->m_uspFontList[0].hFont);

        int len = wsprintf(ach, LINENO_FORMAT, nLineNo + 1);
        int width = m_txteditr->TextWidth(hdc, ach, len);

        if (nLineNo >= m_txteditr->GetLineCount())
            len = 0;

        rect.right = rect.left + m_txteditr->m_nLinenoWidth;
        rect.right += 0;

        SetTextColor(hdc, RGB(0x77, 0x77, 0x77));
        SetBkColor(hdc, RGB(0x22, 0x22, 0x22));

        ExtTextOut(hdc,
            rect.left + m_txteditr->m_nLinenoWidth - width,
            rect.top + NeatTextYOffset(&m_txteditr->m_uspFontList[0]),
            ETO_OPAQUE | ETO_CLIPPED,
            &rect,
            ach,
            len,
            0);

        rect.left = rect.right;
        rect.right += 1;
        PaintRect(hdc, &rect, RGB(0xff, 0xff, 0xff));
        SelectObject(hdc, hOldFont);
    } else {
        DrawCheckedRect(hdc, &rect, RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff));
    }

    return rect.right - rect.left;
}
void EditView::DrawCheckedRect(HDC hdc, RECT* rect, COLORREF fg, COLORREF bg)
{
    static WORD wCheckPat[8] = {
        0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555
    };

    HBRUSH hbr, hbrold;
    COLORREF fgold, bgold;

    HBITMAP hbmp = CreateBitmap(8, 8, 1, 1, wCheckPat);
    hbr = CreatePatternBrush(hbmp);

    SetBrushOrgEx(hdc, rect->left, 0, 0);
    hbrold = (HBRUSH)SelectObject(hdc, hbr);

    fgold = SetTextColor(hdc, fg);
    bgold = SetBkColor(hdc, bg);

    PatBlt(hdc, rect->left, rect->top,
        rect->right - rect->left,
        rect->bottom - rect->top,
        PATCOPY);

    SetBkColor(hdc, bgold);
    SetTextColor(hdc, fgold);

    SelectObject(hdc, hbrold);
    DeleteObject(hbr);
    DeleteObject(hbmp);
}
int EditView::LeftMarginWidth()
{
    int width = 0;
    int cx = 0;
    int cy = 0;

    if (true) {
        width += m_txteditr->m_nLinenoWidth + 1;
        return width;
    } else {
        return width;
    }

    return 0;
}
void EditView::reset_usp_cache()
{
    for (int i = 0; i < USP_CACHE_SIZE; i++) {
        m_uspCache[i].usage = 0;
    }
}
void EditView::RefreshWindow()
{
    InvalidateRect(m_hWnd, NULL, FALSE);
}
