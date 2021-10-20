#define _CRT_SECURE_NO_WARNINGS
#include "DocMgr.h"

void DocMgr::mouse_down(int mx, int my)
{
    long long lineno = 0;
    int stridx = 0;
    int px = 0;
    int py = 0;

    XYPosToBufferIndex(mx, my, &lineno, &stridx, &px, &py);
    POS ps;
    ps.lineno = lineno;
    ps.stridx = stridx;

    set_caret(px, py, ps);
    set_select_start(px, py, ps);
}
SELMODE DocMgr::mouse_move(int mx, int my)
{
    if (m_select_mode != SELMODE::START) {
        return m_select_mode;
    }

    return set_select_move(mx, my);
}
void DocMgr::mouse_up(int mx, int my)
{
    set_select_end(mx, my);
}
long long DocMgr::move_caret(int key)
{
    if (check_selected())
        clear_select();

    switch (key) {
    case VK_LEFT:
        set_caret(m_caret_pos.lineno, --m_caret_pos.stridx, VK_LEFT);
        break;
    case VK_RIGHT:
        set_caret(m_caret_pos.lineno, ++m_caret_pos.stridx, VK_RIGHT);
        break;
    case VK_UP:
        if (!is_painted_area(m_caret_pos.lineno)) {
            m_txteditr->Scroll(0, m_caret_pos.lineno - 1 - m_nVScrollPos);
        } else if (m_nVScrollPos == m_caret_pos.lineno) {
            m_txteditr->Scroll(0, -1);
        }
        set_caret(--m_caret_pos.lineno, m_caret_pos.stridx);
        break;
    case VK_DOWN:
        if (!is_painted_area(m_caret_pos.lineno)) {
            m_txteditr->Scroll(0, m_caret_pos.lineno - 1 - m_nVScrollPos);
        } else if (m_nVScrollPos + m_nWindowLines - 1 == m_caret_pos.lineno) {
            m_txteditr->Scroll(0, 1);
        }
        set_caret(++m_caret_pos.lineno, m_caret_pos.stridx);
        break;
    }
    return 0;
}
long long DocMgr::keydown_text(TCHAR* szText, ULONG nLength)
{
    if (check_selected()) {
        parse_selected(SEL_REMOVE);
    }

    if (m_caret_pos.lineno < 0 || m_caret_pos.stridx < 0 || nLength < 0)
        return 0;

    POS pos = insert_text_parser(m_caret_pos.lineno, m_caret_pos.stridx, szText, nLength);
    if (pos.lineno < 0)
        return 0;

    m_editview->reset_usp_cache();
    m_txteditr->UpdateMetrics();
    set_caret(pos);
    return 0;
}
long long DocMgr::keydown_enter(TCHAR* szText, ULONG nLength)
{
    if (check_selected()) {
        parse_selected(SEL_REMOVE);
        return 0;
    }

    POS pos = insert_text_parser(m_caret_pos.lineno, m_caret_pos.stridx, szText, nLength);
    if (pos.lineno < 0)
        return 0;

    m_editview->reset_usp_cache();
    m_txteditr->UpdateMetrics();
    set_caret(pos);

    return 0;
}
long long DocMgr::keydown_backspace()
{
    if (check_selected()) {
        parse_selected(SEL_REMOVE);
        return 0;
    }

    POS pos;
    pos.lineno = -1;
    pos.stridx = -1;

    if (m_caret_pos.lineno == 0 && m_caret_pos.stridx == 0)
        return 0;
    else if (m_caret_pos.lineno < 0 || m_caret_pos.stridx < 0)
        return 0;

    int u16len = MAX_SIZE;
    node* _node = m_Document->iterater(m_caret_pos.lineno, m_Document->m_sbuffer, &u16len);

    if (m_caret_pos.stridx == 0) {
        int u16lenp = MAX_SIZE;
        node* _nodep = m_Document->iterater(m_caret_pos.lineno - 1, m_Document->m_sbuffer_sub, &u16lenp);
        pos.stridx = m_Document->remove_CRLF(_nodep, _node, u16lenp, u16len);
        pos.lineno = --m_caret_pos.lineno;
    } else {
        int move = m_Document->remove_char(_node, u16len, m_caret_pos.stridx, 1, -1);
        pos.lineno = m_caret_pos.lineno;
        pos.stridx = m_caret_pos.stridx - move;
    }

    if (pos.lineno < 0)
        return 0;

    m_editview->reset_usp_cache();
    m_txteditr->UpdateMetrics();
    set_caret(pos);
    return 0;
}
long long DocMgr::keydown_delete()
{
    if (check_selected()) {
        parse_selected(SEL_REMOVE);
        return 0;
    }

    POS pos;
    pos.lineno = -1;
    pos.stridx = -1;

    int u16len = MAX_SIZE;
    node* _node = m_Document->iterater(m_caret_pos.lineno, m_Document->m_sbuffer_sub, &u16len);
    int crlen = m_Document->CRLF_size(m_Document->m_sbuffer_sub, u16len);

    if (m_caret_pos.lineno == m_Document->linecount() - 1 && m_caret_pos.stridx == u16len - crlen) {
        return 0;
    } else if (m_caret_pos.lineno < 0 || m_caret_pos.stridx < 0) {
        return 0;
    }

    if (m_caret_pos.stridx == u16len - crlen) {
        if (m_caret_pos.lineno != m_Document->linecount() - 1) {
            int u16lenp = MAX_SIZE;
            node* _nodep = m_Document->iterater(m_caret_pos.lineno + 1, m_Document->m_sbuffer, &u16lenp);
            pos.stridx = m_Document->remove_CRLF(_node, _nodep, u16len, u16lenp);
            pos.lineno = m_caret_pos.lineno;
        }

    } else {
        wcsncpy(m_Document->m_sbuffer, m_Document->m_sbuffer_sub, u16len);
        int move = m_Document->remove_char(_node, u16len, m_caret_pos.stridx, 1, 1);
        pos.lineno = m_caret_pos.lineno;
        pos.stridx = m_caret_pos.stridx;
    }

    m_editview->reset_usp_cache();
    m_txteditr->UpdateMetrics();
    return 0;
}
long long DocMgr::on_paste()
{
    TCHAR* szText = nullptr;

    if (OpenClipboard(m_hWnd)) {
        HANDLE hMem = GetClipboardData(CF_UNICODETEXT);
        if (hMem != NULL) {
            szText = (TCHAR*)GlobalLock(hMem);
            GlobalUnlock(hMem);
        }
        CloseClipboard();

        if (szText) {
            ULONG textlen = lstrlen(szText);
            if (textlen < 1)
                return 0;

            POS pos = insert_text_parser(m_caret_pos.lineno, m_caret_pos.stridx, szText, textlen);
            m_editview->reset_usp_cache();
            m_txteditr->UpdateMetrics();
            set_caret(pos);
        }
    }

    return 0;
}
long long DocMgr::on_copy()
{
    if (check_selected()) {
        if (OpenClipboard(m_hWnd)) {
            HANDLE hMem;
            TCHAR* ptr;
            int len = parse_selected(SEL_COPY);

            if ((hMem = GlobalAlloc(GPTR, (len + 1) * sizeof(TCHAR))) != 0) {
                if ((ptr = (TCHAR*)GlobalLock(hMem)) != 0) {
                    EmptyClipboard();
                    wcsncpy(ptr, m_Document->m_dbuffer, len);
                    ptr[len] = 0;
                    SetClipboardData(CF_UNICODETEXT, hMem);
                    GlobalUnlock(hMem);
                }
            }

            CloseClipboard();
        }
    }

    return 0;
}
long long DocMgr::on_cut()
{
    if (check_selected()) {
        on_copy();
        parse_selected(SEL_REMOVE);
        return 0;
    }
}
