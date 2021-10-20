#include "DocMgr.h"
#include "TextEditor.h"

LONG TextEditor::OnMouseActivate(HWND hwndTop, UINT nHitTest, UINT nMessage)
{
    SetFocus(m_hWnd_txtedit);
    return MA_ACTIVATE;
}
LONG TextEditor::OnSetFocus(HWND hwndOld)
{
    m_tabs->m_active_tab->m_docmgr->create_caret();
    m_editview->RefreshWindow();
    return 0;
}
LONG TextEditor::OnKillFocus(HWND hwndNew)
{
    m_tabs->m_active_tab->m_docmgr->destroy_caret();
    m_editview->RefreshWindow();
    return 0;
}
LONG TextEditor::OnLButtonDown(UINT nFlags, int mx, int my)
{
    SetCapture(m_hWnd_txtedit);
    m_tabs->m_active_tab->m_docmgr->mouse_down(mx, my);
    return 0;
}
LONG TextEditor::OnLButtonUp(UINT nFlags, int mx, int my)
{
    ReleaseCapture();
    m_tabs->m_active_tab->m_docmgr->mouse_up(mx, my);
    return 0;
}
LONG TextEditor::OnMouseMove(UINT nFlags, int mx, int my)
{
    SELMODE selmode = m_tabs->m_active_tab->m_docmgr->mouse_move(mx, my);
    RECT rect;
    POINT pt = { mx, my };
    GetClientRect(m_hWnd_txtedit, &rect);
    rect.bottom -= rect.bottom % m_nLineHeight;
    rect.left += m_editview->LeftMarginWidth();

    if (PtInRect(&rect, pt) || SELMODE::START != selmode) {
        if (m_nScrollTimer != 0) {
            KillTimer(m_hWnd_txtedit, m_nScrollTimer);
            m_nScrollTimer = 0;
        }
    } else {
        if (m_nScrollTimer == 0) {
            m_nScrollCounter = 0;
            m_nScrollTimer = SetTimer(m_hWnd_txtedit, 1, 10, 0);
        }
    }
    
    return 0;
}
int ScrollDir(int counter, int distance)
{
    int amt;
    if (abs(distance) < 16)
        amt = 8;
    else if (abs(distance) < 48)
        amt = 3;
    else
        amt = 1;

    if (counter % amt == 0)
        return distance < 0 ? -1 : 1;
    else
        return 0;
}
LONG TextEditor::OnTimer(UINT nTimerId)
{
    int dx = 0, dy = 0;
    RECT rect;
    POINT pt;

    GetClientRect(m_hWnd_txtedit, &rect);
    rect.bottom -= rect.bottom % m_nLineHeight;
    rect.left += m_editview->LeftMarginWidth();

    GetCursorPos(&pt);
    ScreenToClient(m_hWnd_txtedit, &pt);

    if (pt.y < rect.top)
        dy = ScrollDir(m_nScrollCounter, pt.y - rect.top);
    else if (pt.y >= rect.bottom)
        dy = ScrollDir(m_nScrollCounter, pt.y - rect.bottom);

    if (pt.x < rect.left)
        dx = ScrollDir(m_nScrollCounter, pt.x - rect.left);
    else if (pt.x > rect.right)
        dx = ScrollDir(m_nScrollCounter, pt.x - rect.right);

    HRGN hrgnUpdate = Scroll(dx, dy, true);

    if (hrgnUpdate != NULL) {
        OnMouseMove(0, pt.x, pt.y);
        InvalidateRgn(m_hWnd_txtedit, hrgnUpdate, FALSE);
        DeleteObject(hrgnUpdate);
        UpdateWindow(m_hWnd_txtedit);
    }

    m_nScrollCounter++;

    return 0;
}
