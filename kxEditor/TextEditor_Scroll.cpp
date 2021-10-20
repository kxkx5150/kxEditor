#include "TextEditor.h"

LONG TextEditor::OnSize(UINT nFlags, int width, int height)
{
    int margin = 0;
    if (m_editview)
        margin = m_editview->LeftMarginWidth();

    if (m_tabs && m_tabs->m_active_tab && m_tabs->m_active_tab->m_docmgr) {
        m_tabs->m_active_tab->m_docmgr->m_nWindowLines = min((unsigned)height / m_nLineHeight, GetLineCount());
        m_tabs->m_active_tab->m_docmgr->m_nWindowColumns = min((width - margin) / m_nFontWidth, longestline());

        if (PinToBottomCorner()) {
            m_editview->RefreshWindow();
            if (m_tabs && m_tabs->m_active_tab && m_tabs->m_active_tab->m_docmgr)
                m_tabs->m_active_tab->m_docmgr->RepositionCaret();
        }

        SetupScrollbars();
    }

    return 0;
}
long long TextEditor::longestline()
{
    if (m_tabs && m_tabs->m_active_tab && m_tabs->m_active_tab->m_Document)
        return m_tabs->m_active_tab->m_Document->longestline();
    else
        return 0;
}
VOID TextEditor::SetupScrollbars()
{
    SCROLLINFO si = { sizeof(si) };

    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nPos = m_tabs->m_active_tab->m_docmgr->m_nVScrollPos; // scrollbar thumb position
    si.nPage = m_tabs->m_active_tab->m_docmgr->m_nWindowLines; // number of lines in a page
    si.nMin = 0;
    si.nMax = GetLineCount() - 1; // total number of lines in file
    SetScrollInfo(m_hWnd_txtedit, SB_VERT, &si, TRUE);

    if (g_fShowScrollH) {
        si.nPos = m_tabs->m_active_tab->m_docmgr->m_nHScrollPos; // scrollbar thumb position
        si.nPage = m_tabs->m_active_tab->m_docmgr->m_nWindowColumns; // number of lines in a page
        si.nMin = 0;
        si.nMax = longestline() - 1; // total number of lines in file
        SetScrollInfo(m_hWnd_txtedit, SB_HORZ, &si, TRUE);
    }

    m_tabs->m_active_tab->m_docmgr->m_nVScrollMax = GetLineCount() - m_tabs->m_active_tab->m_docmgr->m_nWindowLines;
    m_tabs->m_active_tab->m_docmgr->m_nHScrollMax = longestline() - m_tabs->m_active_tab->m_docmgr->m_nWindowColumns;
}
bool TextEditor::PinToBottomCorner()
{
    bool repos = false;
    if (m_tabs->m_active_tab->m_docmgr->m_nHScrollPos + m_tabs->m_active_tab->m_docmgr->m_nWindowColumns > longestline()) {
        m_tabs->m_active_tab->m_docmgr->m_nHScrollPos = longestline() - m_tabs->m_active_tab->m_docmgr->m_nWindowColumns;
        repos = true;
    }

    if (m_tabs->m_active_tab->m_docmgr->m_nVScrollPos + m_tabs->m_active_tab->m_docmgr->m_nWindowLines > GetLineCount()) {
        m_tabs->m_active_tab->m_docmgr->m_nVScrollPos = GetLineCount() - m_tabs->m_active_tab->m_docmgr->m_nWindowLines;
        repos = true;
    }

    return repos;
}
HRGN TextEditor::Scroll(int dx, int dy, bool fReturnUpdateRgn)
{
    RECT clip;
    GetClientRect(m_hWnd_txtedit, &clip);

    if (dy < 0) {
        dy = -(int)min((long long)-dy, m_tabs->m_active_tab->m_docmgr->m_nVScrollPos);
        clip.top = -dy * m_nLineHeight;
    } else if (dy > 0) {
        dy = min((long long)dy, m_tabs->m_active_tab->m_docmgr->m_nVScrollMax - m_tabs->m_active_tab->m_docmgr->m_nVScrollPos);
        clip.bottom = (m_tabs->m_active_tab->m_docmgr->m_nWindowLines - dy) * m_nLineHeight;
    }

    if (dx < 0) {
        dx = -(int)min(-dx, m_tabs->m_active_tab->m_docmgr->m_nHScrollPos);
        clip.left = -dx * m_nFontWidth * 4;
    } else if (dx > 0) {
        dx = min((unsigned)dx, (unsigned)m_tabs->m_active_tab->m_docmgr->m_nHScrollMax - m_tabs->m_active_tab->m_docmgr->m_nHScrollPos);
        clip.right = (m_tabs->m_active_tab->m_docmgr->m_nWindowColumns - dx - 4) * m_nFontWidth;
    }

    m_tabs->m_active_tab->m_docmgr->m_nHScrollPos += dx;
    m_tabs->m_active_tab->m_docmgr->m_nVScrollPos += dy;

    if (fReturnUpdateRgn == false)
        GetClientRect(m_hWnd_txtedit, &clip);

    clip.left += m_editview->LeftMarginWidth();

    if (dx != 0 || dy != 0) {
        // do the scroll!
        ScrollWindowEx(
            m_hWnd_txtedit,
            -dx * m_nFontWidth, // scale up to pixel coords
            -dy * m_nLineHeight,
            NULL, // scroll entire window
            &clip, // clip the non-scrolling part
            0,
            0,
            SW_INVALIDATE);

        SetupScrollbars();

        if (fReturnUpdateRgn) {
            RECT client;
            GetClientRect(m_hWnd_txtedit, &client);
            //clip.left -= m_editview->LeftMarginWidth();
            HRGN hrgnClient = CreateRectRgnIndirect(&client);
            HRGN hrgnUpdate = CreateRectRgnIndirect(&clip);
            CombineRgn(hrgnUpdate, hrgnClient, hrgnUpdate, RGN_XOR);
            DeleteObject(hrgnClient);
            return hrgnUpdate;
        }
    }

    if (dy != 0) {
        GetClientRect(m_hWnd_txtedit, &clip);
        clip.right = m_editview->LeftMarginWidth();
        InvalidateRect(m_hWnd_txtedit, &clip, 0);
    }

    return NULL;
}
LONG GetTrackPos32(HWND hwnd, int nBar)
{
    SCROLLINFO si = { sizeof(si), SIF_TRACKPOS };
    GetScrollInfo(hwnd, nBar, &si);
    return si.nTrackPos;
}
LONG TextEditor::OnVScroll(UINT nSBCode, UINT nPos)
{
    long long oldpos = m_tabs->m_active_tab->m_docmgr->m_nVScrollPos;

    switch (nSBCode) {
    case SB_TOP:
        m_tabs->m_active_tab->m_docmgr->m_nVScrollPos = 0;
        m_editview->RefreshWindow();
        break;

    case SB_BOTTOM:
        m_tabs->m_active_tab->m_docmgr->m_nVScrollPos = m_tabs->m_active_tab->m_docmgr->m_nVScrollMax;
        m_editview->RefreshWindow();
        break;

    case SB_LINEUP:
        Scroll(0, -1);
        break;

    case SB_LINEDOWN:
        Scroll(0, 1);
        break;

    case SB_PAGEDOWN:
        Scroll(0, m_tabs->m_active_tab->m_docmgr->m_nWindowLines);
        break;

    case SB_PAGEUP:
        Scroll(0, -m_tabs->m_active_tab->m_docmgr->m_nWindowLines);
        break;

    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        m_tabs->m_active_tab->m_docmgr->m_nVScrollPos = GetTrackPos32(m_hWnd_txtedit, SB_VERT);
        m_editview->RefreshWindow();

        break;
    }

    if (oldpos != m_tabs->m_active_tab->m_docmgr->m_nVScrollPos) {
        SetupScrollbars();
        m_tabs->m_active_tab->m_docmgr->OnScroll();
    }

    return 0;
}
LONG TextEditor::OnHScroll(UINT nSBCode, UINT nPos)
{
    int oldpos = m_tabs->m_active_tab->m_docmgr->m_nHScrollPos;

    switch (nSBCode) {
    case SB_LEFT:
        m_tabs->m_active_tab->m_docmgr->m_nHScrollPos = 0;
        m_editview->RefreshWindow();
        break;

    case SB_RIGHT:
        m_tabs->m_active_tab->m_docmgr->m_nHScrollPos = m_tabs->m_active_tab->m_docmgr->m_nHScrollMax;
        m_editview->RefreshWindow();
        break;

    case SB_LINELEFT:
        Scroll(-1, 0);
        break;

    case SB_LINERIGHT:
        Scroll(1, 0);
        break;

    case SB_PAGELEFT:
        Scroll(-m_tabs->m_active_tab->m_docmgr->m_nWindowColumns, 0);
        break;

    case SB_PAGERIGHT:
        Scroll(m_tabs->m_active_tab->m_docmgr->m_nWindowColumns, 0);
        break;

    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        m_tabs->m_active_tab->m_docmgr->m_nHScrollPos = GetTrackPos32(m_hWnd_txtedit, SB_HORZ);
        m_editview->RefreshWindow();
        break;
    }

    if (oldpos != m_tabs->m_active_tab->m_docmgr->m_nHScrollPos) {
        SetupScrollbars();
        m_tabs->m_active_tab->m_docmgr->RepositionCaret();
    }

    return 0;
}

LONG TextEditor::OnMouseWheel(int nDelta)
{
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES 104
#endif

    int nScrollLines;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);

    if (nScrollLines <= 1)
        nScrollLines = 3;

    Scroll(0, (-nDelta / 120) * nScrollLines);
    m_tabs->m_active_tab->m_docmgr->RepositionCaret();

    return 0;
}
