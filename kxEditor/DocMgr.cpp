#define _CRT_SECURE_NO_WARNINGS
#include "DocMgr.h"
#include "u16chars.h"

DocMgr::DocMgr(LinkedList* doc, TextEditor* txtee, EditView* editview, HWND hwnd)
{
    m_hWnd = hwnd;
    m_txteditr = txtee;
    m_Document = doc;
    m_editview = editview;
    m_buffer = new TCHAR[MAX_SIZE];
    init();
}
DocMgr::~DocMgr()
{
    delete m_buffer;
}
void DocMgr::init()
{
    clear();
}
void DocMgr::clear()
{
    POS ps;
    ps.lineno = 0;
    ps.stridx = 0;
    m_caret_pos = ps;
    m_show_caret = false;
    hide_caret();
    m_Document->clear();
    m_editview->reset_usp_cache();
    clear_select();
}
void DocMgr::create_caret()
{
    DWORD nWidth = 0;
    SystemParametersInfo(SPI_GETCARETWIDTH, 0, &nWidth, 0);
    nWidth = 2;
    CreateCaret(m_hWnd, (HBITMAP)NULL, nWidth, m_txteditr->m_nLineHeight);
    set_caret(0, 0);
    ShowCaret(m_hWnd);
}
void DocMgr::destroy_caret()
{
    hide_caret();
    DestroyCaret();
}
void DocMgr::show_caret()
{
    if (!m_show_caret) {
        ShowCaret(m_hWnd);
        m_show_caret = true;
    }
}
void DocMgr::hide_caret()
{
    if (m_show_caret) {
        m_show_caret = false;
        HideCaret(m_hWnd);
    }
}
void DocMgr::set_caret(POS pos)
{
    set_caret(pos.lineno, pos.stridx);
}
void DocMgr::set_caret(long long lineno, int stridx, int arrow)
{
    int buflen = MAX_SIZE;
    node* _node = nullptr;
    int crlf = 0;
    long long linecnt = m_txteditr->GetLineCount();

    if (lineno < 0) {
        lineno = 0;
    } else if (linecnt <= lineno) {
        lineno = linecnt - 1;
    }

    if (arrow == VK_LEFT && stridx < 0 && 0 < lineno) {
        _node = m_Document->iterater(lineno - 1, m_buffer, &buflen);
        crlf = m_Document->CRLF_size(m_buffer, buflen);
        stridx = buflen - crlf;
        --lineno;
    } else {
        _node = m_Document->iterater(lineno, m_buffer, &buflen);
        crlf = m_Document->CRLF_size(m_buffer, buflen);

        if (arrow == VK_RIGHT && stridx > _node->datalen - crlf && lineno < linecnt - 1) {
            ++lineno;
            stridx = 0;
        }

        bool lpair = u16_surrogatepair::is_surrogatepair(m_buffer[stridx]);
        if (lpair) {
            if (arrow == VK_LEFT) {
                stridx--;
            } else if (arrow == VK_RIGHT) {
                stridx++;
            }
        }
    }

    if (stridx < 0) {
        stridx = 0;
    } else if (buflen - crlf < stridx) {
        stridx = buflen - crlf;
    }

    m_caret_pos.lineno = lineno;
    m_caret_pos.stridx = stridx;
    RepositionCaret();
}
void DocMgr::set_caret(int x, int y)
{
    int px = x + m_editview->LeftMarginWidth();
    SetCaretPos(px, y);
    show_caret();
}
void DocMgr::set_caret(int x, int y, POS ps)
{
    m_caret_pos = ps;
    set_caret(x, y);
}
void DocMgr::RepositionCaret()
{
    if (m_txteditr->GetLineCount() == 0)
        return;

    long long scrtop = m_nVScrollPos;
    if (is_painted_area(m_caret_pos.lineno)) {
        RECT rect;
        GetClientRect(m_hWnd, &rect);
        int scrleft = m_nHScrollPos * m_txteditr->m_nFontWidth;
        XY xy = buffer_to_offset_XYpos(m_caret_pos.lineno, m_caret_pos.stridx);
        set_caret(xy.x, xy.y);

        if (xy.x < 0) {
            hide_caret();
        } else if (rect.right + scrleft < xy.x) {
            hide_caret();
        }
    } else {
        hide_caret();
    }
}
void DocMgr::set_select_start(int x, int y, POS ps)
{
    clear_select();
    stattmp = ps;
    m_select_mode = SELMODE::START;
}
SELMODE DocMgr::set_select_move(int mx, int my)
{
    long long lineno = 0;
    int stridx = 0;
    int px = 0;
    int py = 0;
    XYPosToBufferIndex(mx, my, &lineno, &stridx, &px, &py);
    POS ps;
    ps.lineno = lineno;
    ps.stridx = stridx;
    endtmp = ps;

    if (stattmp == endtmp) {
        return m_select_mode;
    }

    if ((stattmp.stridx > endtmp.stridx && stattmp.lineno >= endtmp.lineno)
        || stattmp.lineno > endtmp.lineno) {

        set_select(endtmp, stattmp, true);
    } else {
        set_select(stattmp, endtmp, false);
    }

    return m_select_mode;
}
void DocMgr::set_select_end(int mx, int my)
{
    if (m_stat_sel == m_end_sel) {
        m_select_mode = SELMODE::END;
    } else {
        m_select_mode = SELMODE::SELECTED;
    }
}
void DocMgr::set_select(POS stat, POS end, BOOL backward)
{
    m_stat_sel = stat;
    m_end_sel = end;

    pos_to_refresh_area(stat, end, backward);

    if (backward)
        set_caret(m_stat_sel);
    else
        set_caret(m_end_sel);
}
void DocMgr::clear_select()
{
    m_select_mode = SELMODE::END;

    POS ps2;
    ps2.lineno = 0;
    ps2.stridx = -1;
    m_stat_sel = ps2;

    POS ps3;
    ps3.lineno = 0;
    ps3.stridx = -1;
    m_end_sel = ps3;

    m_editview->clear_select_pos();
    m_editview->RefreshWindow();
}
SELMODE DocMgr::get_selmode()
{
    return m_select_mode;
}
BOOL DocMgr::select_paint_mode()
{
    return m_select_mode != SELMODE::END;
}
BOOL DocMgr::check_selected()
{
    return m_select_mode == SELMODE::SELECTED;
}
int DocMgr::parse_selected(int mode)
{
    switch (mode) {
    case SEL_REMOVE:
        remove_selected_area();
        break;

    case SEL_COPY:
        return copy_text_parser(m_stat_sel, m_end_sel, false);
    }

    return 0;
}
int DocMgr::remove_selected_area()
{
    set_caret(m_stat_sel.lineno, m_stat_sel.stridx);
    remove_text_parser(m_stat_sel, m_end_sel);
    clear_select();
    return 0;
}
void DocMgr::insert_text(long long lineno, int stridx, TCHAR* szText, ULONG nLength)
{
    POS pos = insert_text_parser(lineno, stridx, szText, nLength);
    m_editview->reset_usp_cache();
    m_txteditr->UpdateMetrics();
    set_caret(pos);
}
void DocMgr::remove_text(long long lineno, int stridx, int count, int direction, BOOL surrocheck)
{
    int u16len = MAX_SIZE;
    int cnttmp = count;
    node* _node = m_Document->iterater(lineno, m_Document->m_sbuffer, &u16len);
    int crlflen = m_Document->CRLF_size(m_Document->m_sbuffer, u16len);

    if (count == END_OF_LINE || count == END_OF_LINE_CRLF)
        cnttmp = u16len - stridx - crlflen;

    int move = m_Document->remove_char(_node, u16len, stridx, cnttmp, direction, surrocheck);

    if (count == END_OF_LINE_CRLF) {
        node* _node = m_Document->iterater(lineno, m_Document->m_sbuffer_sub, &u16len);
        int u16lenb = MAX_SIZE;
        node* _nodeb = m_Document->iterater(lineno + 1, m_Document->m_sbuffer, &u16lenb);
        m_Document->remove_CRLF(_node, _nodeb, u16len, u16lenb);
    }
}
void DocMgr::remove_line(long long lineno)
{
    int u16len = MAX_SIZE;
    node* _node = m_Document->iterater(lineno, m_Document->m_sbuffer, &u16len);
    m_Document->remove_line(_node);
}
int DocMgr::copy_text(long long lineno, int stridx, int count, int direction, int bufferidx, BOOL surrocheck)
{
    int u16len = MAX_SIZE;
    int cnttmp = count;
    node* _node = m_Document->iterater(lineno, m_Document->m_sbuffer, &u16len);
    int crlflen = m_Document->CRLF_size(m_Document->m_sbuffer, u16len);

    if (count == END_OF_LINE) {
        cnttmp = u16len - stridx - crlflen;
    } else if (count == END_OF_LINE_CRLF) {
        cnttmp = u16len - stridx;
    }

    wcsncpy(bufferidx + m_Document->m_dbuffer, m_Document->m_sbuffer + stridx, cnttmp);
    return cnttmp;
}

int DocMgr::copy_text_parser(POS stat, POS end, BOOL rm)
{
    int len = get_text_parser(stat, end, false);
    return len;
}
int DocMgr::get_text_parser(POS stat, POS end, BOOL rm)
{
    TCHAR* str = nullptr;
    int len = 0;

    if (stat.lineno == end.lineno) {
        len = copy_text(stat.lineno, stat.stridx, end.stridx - stat.stridx, 1, 0, false);
    } else {
        for (size_t i = stat.lineno; i <= end.lineno; i++) {
            if (stat.lineno == i) {
                len = copy_text(stat.lineno, stat.stridx, END_OF_LINE_CRLF, 1, len, false);
            } else if (end.lineno == i) {
                len += copy_text(end.lineno, 0, end.stridx, 1, len, false);
            } else {
                len += copy_text(i, 0, END_OF_LINE_CRLF, 1, len, false);
            }
        }
    }

    return len;
}
void DocMgr::remove_text_parser(POS stat, POS end)
{
    if (stat.lineno == end.lineno) {
        remove_text(stat.lineno, stat.stridx, end.stridx - stat.stridx, 1, false);
        m_editview->reset_usp_cache();
    } else {
        for (long long i = end.lineno; stat.lineno <= i; i--) {
            if (stat.lineno == i) {
                remove_text(stat.lineno, stat.stridx, END_OF_LINE_CRLF, 1, false);
            } else if (end.lineno == i) {
                remove_text(end.lineno, 0, end.stridx, 1, false);
            } else {
                remove_line(i);
            }
        }

        m_editview->reset_usp_cache();
    }
}
POS DocMgr::insert_text_parser(long long lineno, int stridx, TCHAR* szText, ULONG nLength)
{
    POS pos;
    pos.lineno = -1;
    pos.stridx = -1;

    long long linenotmp = lineno;
    int stridxtmp = stridx;
    int lentmp = 0;
    int sztxtoffset = 0;
    int stat = 0;

    for (size_t i = 0; i < nLength; i++) {
        lentmp++;
        if (nLength == 1 && szText[0] == '\n') {
            int u16len = MAX_SIZE;
            node* _node = m_Document->iterater(lineno, m_Document->m_sbuffer, &u16len);
            pos.stridx = m_Document->insert_CRLF(_node, u16len, stridx);
            linenotmp++;
            stridxtmp = 0;

        } else if (szText[i] == '\n') {
            int crlflen = m_Document->CRLF_size(szText + stat, lentmp);
            int u16len = MAX_SIZE;

            node* _node = m_Document->iterater(linenotmp, m_Document->m_sbuffer, &u16len);
            m_Document->insert_char(szText + stat, lentmp - crlflen, _node, u16len, stridxtmp);
            u16len = MAX_SIZE;
            _node = m_Document->iterater(linenotmp, m_Document->m_sbuffer, &u16len);
            m_Document->insert_CRLF(_node, u16len, lentmp + stridxtmp - crlflen);

            stat = i + 1;
            lentmp = 0;
            stridxtmp = 0;
            linenotmp++;

        } else if (i == nLength - 1) {
            int u16len = MAX_SIZE;
            node* _node = m_Document->iterater(linenotmp, m_Document->m_sbuffer, &u16len);
            m_Document->insert_char(szText + stat, lentmp, _node, u16len, stridxtmp);

            stridxtmp += lentmp;
        }
    }
    //m_Document->forward_traverse(true);
    pos.lineno = linenotmp;
    pos.stridx = stridxtmp;

    return pos;
}

void DocMgr::pos_to_refresh_area(POS stat, POS end, BOOL backward)
{
    POS mstat = m_editview->m_paint_sel_stat < stat ? m_editview->m_paint_sel_stat : stat;
    POS mend = m_editview->m_paint_sel_end > end ? m_editview->m_paint_sel_end : end;

    if (mstat.lineno == mend.lineno) {
        m_editview->RefreshWindow();
        //slow?
        //pos_to_refresh_area_single(mstat, mend, backward);
    } else {
        m_editview->RefreshWindow();
        //slow?
        //pos_to_refresh_area_multiple(mstat, mend, backward);
    }
}
void DocMgr::pos_to_refresh_area_single(POS mstat, POS mend, BOOL backward)
{
    if (!is_painted_area(mstat.lineno))
        return;

    RECT rect;
    XY fxy, lxy;
    int left = m_editview->LeftMarginWidth();
    fxy = buffer_to_offset_XYpos(mstat.lineno, mstat.stridx);
    lxy = buffer_to_offset_XYpos(mend.lineno, mend.stridx);

    SetRect(&rect,
        fxy.x + left,
        fxy.y,
        lxy.x + left,
        lxy.y + m_txteditr->m_nLineHeight);
    InvalidateRect(m_hWnd, &rect, FALSE);
}
void DocMgr::pos_to_refresh_area_multiple(POS mstat, POS mend, BOOL backward)
{
    RECT rect, client;
    XY fxy, lxy;
    int left = m_editview->LeftMarginWidth();

    BOOL statp = is_painted_area(mstat.lineno);
    BOOL endp = is_painted_area(mend.lineno);

    if (statp) {
        fxy = buffer_to_offset_XYpos(mstat.lineno, mstat.stridx);
        node* _node = m_Document->search_node(mstat.lineno);
        lxy = buffer_to_offset_XYpos(mstat.lineno, _node->datalen);
        SetRect(&rect,
            fxy.x + left,
            fxy.y,
            lxy.x + left,
            lxy.y + m_txteditr->m_nLineHeight);
        InvalidateRect(m_hWnd, &rect, FALSE);
    }

    if (endp) {
        fxy = buffer_to_offset_XYpos(mend.lineno, mend.stridx);
        lxy = buffer_to_offset_XYpos(mend.lineno, 0);
        SetRect(&rect,
            fxy.x + left,
            fxy.y,
            lxy.x + left,
            lxy.y + m_txteditr->m_nLineHeight);
        InvalidateRect(m_hWnd, &rect, FALSE);
    }

    if ((!statp && !endp) || mstat.lineno + 1 == mend.lineno)
        return;

    GetClientRect(m_hWnd, &client);
    int statline = mstat.lineno + 1;
    int endline = mend.lineno - statline;

    if (!statp && endp) {
        statline = m_nVScrollPos;
    } else if (statp && !endp) {
        endline = m_nVScrollPos + m_nWindowLines;
    }

    fxy = buffer_to_offset_XYpos(statline, 0);
    SetRect(&rect,
        client.left + left,
        fxy.y,
        client.right,
        fxy.y + m_txteditr->m_nLineHeight * endline);
    InvalidateRect(m_hWnd, &rect, FALSE);
}
int DocMgr::XPosToBufferIndex(int x, long long lineno, int* px)
{
    int curx = 0;
    int off_chars = 0;
    RECT rect;

    GetClientRect(m_hWnd, &rect);
    rect.bottom -= rect.bottom % m_txteditr->m_nLineHeight;

    x -= m_editview->LeftMarginWidth();

    if (x < 0)
        x = 0;
    if (x >= rect.right)
        x = rect.right - 1;

    int tmpx = x + m_nHScrollPos * m_txteditr->m_nFontWidth;
    USPDATA* uspData = m_editview->GetUspData(0, (ULONG)lineno);
    UspSnapXToOffset(uspData, tmpx, &x, &off_chars, 0);
    *px = x;
    return off_chars;
}
long long DocMgr::YPosToLineIndex(int y)
{
    long long nLineNo = 0;
    RECT rect;

    GetClientRect(m_hWnd, &rect);
    rect.bottom -= rect.bottom % m_txteditr->m_nLineHeight;

    if (y < 0)
        y = 0;
    if (y >= rect.bottom)
        y = rect.bottom - 1;

    long long linecnt = m_txteditr->GetLineCount();
    nLineNo = (y / m_txteditr->m_nLineHeight) + m_nVScrollPos;
    if (nLineNo >= linecnt) {
        nLineNo = linecnt ? linecnt - 1 : 0;
    }

    return nLineNo;
}
BOOL DocMgr::XYPosToBufferIndex(int x, int y, long long* lineno, int* stridx, int* px, int* py)
{
    *lineno = YPosToLineIndex(y);
    *stridx = XPosToBufferIndex(x, *lineno, px);
    *py = (int)((*lineno - m_nVScrollPos) * m_txteditr->m_nLineHeight);
    *px -= m_nHScrollPos * m_txteditr->m_nFontWidth;

    return 0;
}
XY DocMgr::buffer_to_absolute_XYpos(long long lineno, int stridx)
{
    XY xy;
    xy.y = lineno * (long long)m_txteditr->m_nLineHeight;
    xy.x = charindex_to_Xpos(lineno, stridx);
    return xy;
}
XY DocMgr::buffer_to_offset_XYpos(long long lineno, int stridx)
{
    XY xy = buffer_to_absolute_XYpos(lineno, stridx);
    int tmpx = xy.x - m_nHScrollPos * m_txteditr->m_nFontWidth;
    xy.x = tmpx;
    long long tmpy = xy.y - m_nVScrollPos * m_txteditr->m_nLineHeight;
    xy.y = tmpy;
    return xy;
}
int DocMgr::charindex_to_Xpos(int lineno, int stridx)
{
    int xpos = 0;
    USPDATA* uspData;
    if ((uspData = m_editview->GetUspData(NULL, lineno)) != 0) {
        UspOffsetToX(uspData, stridx, FALSE, &xpos);
    }

    return xpos;
}
BOOL DocMgr::is_painted_area(long long lineno)
{
    long long scrtop = m_nVScrollPos;
    if (scrtop <= lineno
        && lineno < scrtop + m_nWindowLines) {
        return true;
    }

    return false;
}
//events
void DocMgr::OnScroll()
{
    RepositionCaret();
}
