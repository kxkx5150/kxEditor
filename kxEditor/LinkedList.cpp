#define _CRT_SECURE_NO_WARNINGS

#include "LinkedList.h"
#include "Unicode.h"
#include "u16chars.h"
#include <algorithm>
#include <string>

_BOM_LOOKUP BOMLOOK[] = {
    { 0xBFBBEF, 3, NCP_UTF8BOM },
    { 0, 0, NCP_UTF8 },
};

LinkedList::LinkedList()
{
    init();
}
LinkedList::~LinkedList()
{
    delete_all();
    delete m_sbuffer;
    delete m_sbuffer_sub;
    delete m_dbuffer;
}
void LinkedList ::init()
{
    node* temp = new node();
    temp->data = NULL;
    temp->prev = NULL;
    front = temp;

    node* etemp = new node();
    etemp->data = NULL;
    etemp->next = NULL;
    end = etemp;

    front->next = end;
    end->prev = front;
    pointer = front;
    pointer_lineno = -1;

    m_sbuffer = new TCHAR[MAX_SIZE];
    m_sbuffer_sub = new TCHAR[MAX_SIZE];
    m_dbuffer = new TCHAR[MAX_SIZE];
}
void LinkedList::create_empty()
{
    const long long blen = 0;
    BYTE* buftmp = new BYTE[100];
    append_node(buftmp, blen);
    pointer = front;
    pointer_lineno = -1;
    //check_link(true);
}
void LinkedList::clear()
{
    pointer = front;
    m_nLineCount = 0;
    m_fileformat = NCP_UTF8;
    m_CRLF = 2;
    m_headersize = 0;
    pointer_lineno = -1;
    delete_all();
    init();
}
int LinkedList ::delete_all()
{
    node* trav;
    trav = front;
    node* tmp;

    while (trav != NULL) {
        tmp = trav;
        trav = trav->next;

        if (tmp->data) {
            //print_node_data(tmp);
            delete tmp->data;
        }

        delete tmp;
    }
    return 0;
}
bool LinkedList::open_file(TCHAR* filename, int tabwidth)
{
    m_filename = filename;
    m_tabwidth = tabwidth;

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    long long bytelen = 0;
    if ((bytelen = GetFileSize(hFile, 0)) == 0) {
        CloseHandle(hFile);
        return false;
    }

    BYTE* buffer = nullptr;
    if ((buffer = new BYTE[bytelen]) == 0) {
        CloseHandle(hFile);
        return false;
    }

    ULONG numread;
    BOOL rflg = ReadFile(hFile, buffer, bytelen, &numread, 0);
    if (!rflg) {
        CloseHandle(hFile);
        return false;
    }

    clear();
    m_fileformat = detect_file_format(buffer, bytelen, &m_headersize);

    init_linebuffer(buffer, bytelen);

    CloseHandle(hFile);
    delete[] buffer;
    return true;
}
int LinkedList::detect_file_format(BYTE* buf, long long bytelen, int* hdr)
{
    int len = (sizeof(BOMLOOK) / sizeof(*BOMLOOK));
    for (int i = 0; i < len; i++) {
        if (bytelen >= BOMLOOK[i].len && memcmp(buf, &BOMLOOK[i].bom, BOMLOOK[i].len) == 0) {
            *hdr = BOMLOOK[i].len;
            return BOMLOOK[i].type;
        }
    }

    *hdr = 0;
    return NCP_UTF8;
}
bool LinkedList::init_linebuffer(BYTE* buf, long long bytelength)
{
    long long ccount = 0;
    long long bytelen = 0;
    long long buflen = bytelength - m_headersize;
    m_nLineCount = 0;
    LONG cpstat = -1;
    int tabs = 0;

    long long ch32 = 0;
    for (bytelen = 0; bytelen < buflen;) {
        if (cpstat == -1)
            cpstat = bytelen;

        long long len = getbyte(buf, bytelen, buflen - bytelen, &ch32);
        bytelen += len;
        ccount++;

        if (ch32 == '\t') {
            tabs++;
        } else if (ch32 == '\r') {
            len = getbyte(buf, bytelen, buflen - bytelen, &ch32);
            bytelen += len;
            ccount++;
            if (ch32 == '\n') {
                buffer_to_node(buf, cpstat, bytelen, ccount, tabs);
                cpstat = -1;
                ccount = 0;
                tabs = 0;
            }
        } else if (ch32 == '\n') {
            buffer_to_node(buf, cpstat, bytelen, ccount, tabs);
            cpstat = -1;
            ccount = 0;
            tabs = 0;
        }
    }

    if (ch32 == '\n') {
        BYTE* buftmp = new BYTE[0];
        append_node(buftmp, 0);
    } else {
        buffer_to_node(buf, cpstat, bytelen, ccount, tabs);
    }

    //forward_traverse(true);
    check_CRLF_mode();
    _RPTN(_CRT_WARN, "m_CRLF  : %lu\n", m_CRLF);
    _RPTN(_CRT_WARN, "m_nLineCount  : %lu\n", m_nLineCount);
    _RPTN(_CRT_WARN, "m_nLongestLine     : %lu\n\n", longestline());

    return true;
}
void LinkedList::buffer_to_node(BYTE* buf, long long start, long long end, long long ccount, int tabs)
{
    const long long blen = end - start;
    BYTE* buftmp = new BYTE[blen];
    memcpy(buftmp, &buf[start], blen);
    append_node(buftmp, blen);

    long long charwidth = ccount + tabs * m_tabwidth - tabs;
    if (charwidth > m_nLongestLine)
        m_nLongestLine = charwidth;
}
node* LinkedList::append_node(BYTE* d, long long len)
{
    node* tail = end->prev;

    node* temp = new node();
    temp->data = d;
    temp->datalen = len;

    temp->prev = tail;
    temp->next = end;
    tail->next = temp;
    end->prev = temp;

    m_nLineCount++;
    return temp;
}

node* LinkedList::insert_befor_node(node* point, BYTE* d, long long len)
{
    if (0 == point)
        return 0;

    node* temp = new node();
    temp->data = d;
    temp->datalen = len;

    point->prev->next = temp;
    temp->prev = point->prev;
    temp->next = point;

    point->prev = temp;

    m_nLineCount++;
    pointer = temp;
    return temp;
}
node* LinkedList::insert_node(long long lineno, BYTE* d, long long len)
{
    node* point = search_node(lineno);
    if (0 == point)
        return 0;

    node* temp = new node();
    temp->data = d;
    temp->datalen = len;

    temp->prev = point;
    temp->next = point->next;
    point->next = temp;
    temp->next->prev = temp;

    m_nLineCount++;
    return temp;
}
node* LinkedList::insert_node(node* point, BYTE* d, long long len)
{
    if (0 == point)
        return 0;

    node* temp = new node();
    temp->data = d;
    temp->datalen = len;

    temp->prev = point;
    temp->next = point->next;
    point->next = temp;
    temp->next->prev = temp;

    m_nLineCount++;
    return temp;
}
node* LinkedList::replace_node_data(node* _node, int u16len, int* u8len)
{
    BYTE* u8str = new BYTE[*u8len];
    utf16_to_utf8(m_dbuffer, u16len, u8str, (size_t*)u8len);

    delete _node->data;
    _node->data = u8str;
    _node->datalen = *u8len;

    check_longest_line(u8str, *u8len, u16len);
    return _node;
}
node* LinkedList::set_pointer(long long idx)
{
    long long stat = 0;
    node* trav;
    trav = front;

    while (trav != NULL) {
        if (trav->next == NULL) {
            return 0;
        } else {
            if (trav->data) {
                if (stat == idx) {
                    pointer = trav;
                    return pointer;
                }
                stat++;
            }
            trav = trav->next;
        }
    }

    return 0;
}
node* LinkedList::get_pointer()
{
    return pointer;
}
node* LinkedList::pre(long long step)
{
    long long cnt = 0;
    while (pointer != front) {
        if (step == cnt) {
            return pointer;
        }
        if (pointer->prev->data) {
            pointer = pointer->prev;
        } else {
            return 0;
        }
        cnt++;
    }
    return 0;
}
node* LinkedList::next(long long step)
{
    long long cnt = 0;
    while (pointer != end) {
        if (step == cnt) {
            return pointer;
        }
        if (pointer->next->data) {
            pointer = pointer->next;
        } else {
            return 0;
        }
        cnt++;
    }
    return 0;
}
int LinkedList ::delete_node(node* n)
{
    if (n == front || n == end)
        return 0;

    if (n->prev == NULL) {
        front = n->next;
        front->prev = NULL;
    } else if (n->next == NULL) {
        end = n->prev;
        end->next = NULL;
    } else {
        n->prev->next = n->next;
        n->next->prev = n->prev;
    }

    if (pointer == n) {
        pointer_lineno--;
        pointer = pointer->prev;
    }

    delete n;
    m_nLineCount--;

    if (m_nLineCount == 0) {
        //OutputDebugString(L"---delete_node\n");
        create_empty();
        //forward_traverse(true);
        return 0;
    }

    return 0;
}
void LinkedList::check_longest_line(BYTE* u8str, int u8len, int u16len)
{
    int tabs = count_tabs(u8str, u8len);
    long long charwidth = u16len + tabs * m_tabwidth - tabs;
    if (charwidth > m_nLongestLine)
        m_nLongestLine = charwidth;
}
int LinkedList::count_tabs(BYTE* buf, long long buflen)
{
    long long bytelen = 0;
    int tabs = 0;
    long long ch32 = 0;

    for (bytelen = 0; bytelen < buflen;) {
        long long len = getbyte(buf, bytelen, buflen - bytelen, &ch32);
        bytelen += len;

        if (ch32 == '\t') {
            tabs++;
        }
    }
    return tabs;
}

void LinkedList::convert_utf16_to_utf8(TCHAR* buff, int buflen, BYTE* u8buf, int* u8len)
{
    utf16_to_utf8(buff, buflen, u8buf, (size_t*)u8len);
}

int LinkedList::gettext(node* trav, TCHAR* buff, int* buflen)
{
    if (!trav || !trav->data)
        return 0;

    BYTE* tmp = trav->data;
    long long len = trav->datalen;
    return gettext(tmp, len, buff, buflen);
}
node* LinkedList::iterater(long long lineno, TCHAR* buf, int* buflen)
{
    node* rval = search_node(lineno);
    if (rval == 0) {
        *buflen = 0;
        return 0;
    }

    int len = gettext(pointer->data, pointer->datalen, buf, buflen);
    if (len == 0) {
        *buflen = 0;
    }
    return pointer;
}
node* LinkedList::search_node(long long lineno)
{
    node* rval = nullptr;
    if (pointer_lineno == -1) {
        rval = set_pointer(lineno);
    } else if (lineno < pointer_lineno) {
        rval = pre(pointer_lineno - lineno);
    } else if (lineno > pointer_lineno) {
        rval = next(lineno - pointer_lineno);
    } else if (lineno == pointer_lineno) {
        rval = pointer;
    } else {
        rval = set_pointer(lineno);
    }

    if (rval == 0) {
        pointer_lineno = -1;
        pointer = front;
        return 0;
    }

    pointer_lineno = lineno;
    return rval;
}
int LinkedList::getbyte(BYTE* buf, long long offset, long long lenBYTEs, long long* pch32)
{
    BYTE* rawdata = (BYTE*)(buf + offset);
    size_t ch32len = 1;

    switch (m_fileformat) {
    case NCP_UTF8:
    case NCP_UTF8BOM:
        return utf8_to_utf32(rawdata, lenBYTEs, pch32);
    default:
        return 0;
    }
}
int LinkedList::gettext(BYTE* start, long long blen, TCHAR* buff, int* buflen)
{
    BYTE* rawdata = (BYTE*)(start);

    switch (m_fileformat) {
    case NCP_UTF8:
    case NCP_UTF8BOM:
        return utf8_to_utf16(rawdata, blen, buff, (size_t*)buflen);
    default:
        return 0;
    }
}
int LinkedList::strindex_to_bufferpos(BYTE* buff, int buflen, int stridx)
{
    long long ccount = 0;
    long long bytelen = 0;
    long long ch32 = 0;

    _RPTN(_CRT_WARN, "stridx : %d\n", stridx);

    for (bytelen = 0; bytelen < buflen;) {
        long long len = getbyte((BYTE*)buff, bytelen, buflen - bytelen, &ch32);
        bytelen += len;
        _RPTN(_CRT_WARN, "bytelen : %d\n", bytelen);

        ccount++;

        if (stridx == ccount) {
            return bytelen;
        }
    }

    return 0;
}
long long LinkedList::linecount()
{
    return m_nLineCount;
}
long long LinkedList::longestline()
{
    return m_nLongestLine;
}
int LinkedList::getformat()
{
    return m_fileformat;
}
void LinkedList::check_CRLF_mode()
{
    int u16len = MAX_SIZE;
    node* _node = iterater(0, m_sbuffer, &u16len);
    int crlf = CRLF_size(m_sbuffer, u16len);
    if (crlf == 0)
        crlf = 2;

    m_CRLF = crlf;
}
int LinkedList::CRLF_size(TCHAR* szText, int nLength)
{
    if (nLength >= 2) {
        if (szText[nLength - 2] == '\r' && szText[nLength - 1] == '\n')
            return 2;
    }

    if (nLength >= 1) {
        if (szText[nLength - 1] == '\r' || szText[nLength - 1] == '\n' || szText[nLength - 1] == '\x0b'
            || szText[nLength - 1] == '\x0c' || szText[nLength - 1] == '\x85' || szText[nLength - 1] == 0x2028 || szText[nLength - 1] == 0x2029)
            return 1;
    }

    return 0;
}
int LinkedList::CRLF_size(BYTE* szText, int nLength)
{
    BYTE* _szText = (BYTE*)szText;

    if (nLength >= 2) {
        if (_szText[nLength - 2] == '\r' && _szText[nLength - 1] == '\n')
            return 2;
    } else if (nLength >= 1) {
        if (szText[nLength - 1] == '\r' || szText[nLength - 1] == '\n' || szText[nLength - 1] == '\x0b'
            || szText[nLength - 1] == '\x0c' || szText[nLength - 1] == '\x85' || szText[nLength - 1] == 0x2028 || szText[nLength - 1] == 0x2029)
            return 1;
    }

    return 0;
}

int LinkedList::insert_char(TCHAR* szText, ULONG nLength, node* _node, int u16len, int stridx)
{
    int _u8len = _node->datalen;
    wcsncpy(m_dbuffer, m_sbuffer, stridx);
    wcsncpy(m_dbuffer + stridx, szText, nLength);
    wcsncpy(m_dbuffer + stridx + nLength, m_sbuffer + stridx, u16len - stridx);

    int u8len = _u8len + nLength * 8;
    replace_node_data(_node, u16len + nLength, &u8len);
    return 0;
}
int LinkedList::remove_char(node* _node, int u16len, int stridx, int count, int direction, BOOL surrocheck)
{
    int move = 0;
    int crlflen = 0;

    while (count--) {
        if (surrocheck) {
            bool pair = u16_surrogatepair::is_surrogatepair(m_sbuffer[stridx + (move * direction) + direction]);
            move += pair ? 2 : 1;
            //if (move == 1) {
            //    pair = u16_combining_characters::is_combining_characters(m_sbuffer[stridx + (move * direction) + direction]);
            //    move = pair ? 2 : 1;
            //}
        } else {
            move++;
        }
    }

    if (direction == -1) {
        if (move > stridx)
            move = stridx;

        wcsncpy(m_dbuffer, m_sbuffer, stridx - move);
        wcsncpy(m_dbuffer + stridx - move, m_sbuffer + stridx, u16len - stridx);

    } else {
        if (move > u16len - stridx) {
            crlflen = CRLF_size(m_sbuffer, u16len);
            move = u16len - stridx;
        }

        wcsncpy(m_dbuffer, m_sbuffer, stridx);
        wcsncpy(m_dbuffer + stridx, m_sbuffer + stridx + move - crlflen, u16len - stridx - move + crlflen);
    }

    int u8len = _node->datalen;
    replace_node_data(_node, u16len - move + crlflen, &u8len);
    return move;
}
node* LinkedList::insert_line(node* _node, int u16len, int stridx)
{
    long long blen = 0;
    BYTE* buftmp = new BYTE[blen];
    node* nnode = insert_node(_node, buftmp, blen);
    insert_char(m_sbuffer + stridx, u16len - stridx, nnode, blen, 0);
    return nnode;
}
int LinkedList::remove_line(node* _node)
{
    return delete_node(_node);
}
int LinkedList::insert_CRLF(node* _node, int u16len, int stridx)
{
    int crlen = CRLF_size(m_sbuffer, u16len);

    if (u16len == stridx) {
        if (m_CRLF == 2)
            insert_char((TCHAR*)L"\r\n", 2, _node, u16len, stridx);
        else
            insert_char((TCHAR*)L"\n", 1, _node, u16len, stridx);

        long long blen = 0;
        BYTE* buftmp = new BYTE[blen];
        node* _node2 = append_node(buftmp, blen);

        //OutputDebugString(L"-------------\n");
        //print_node_data(_node);
        //print_node_data(_node2);
        return 0;

    } else {
        remove_char(_node, u16len, stridx, u16len - stridx, 1);
        if (m_CRLF == 2)
            insert_char((TCHAR*)L"\r\n", 2, _node, stridx, stridx);
        else
            insert_char((TCHAR*)L"\n", 1, _node, stridx, stridx);

        node* _node2 = insert_line(_node, u16len, stridx);

        //OutputDebugString(L"-------------\n");
        //print_node_data(_node);
        //print_node_data(_node2);

        return 0;
    }

    return -1;
}
int LinkedList::remove_CRLF(node* _nodep, node* _node, int u16lenp, int u16len)
{
    int _u8len = _node->datalen;
    int _u8lenp = _nodep->datalen;

    int crlflen = CRLF_size(m_sbuffer_sub, u16lenp);
    int charlen = crlflen == 2 ? 2 : 1;
    wcsncpy(m_dbuffer, m_sbuffer_sub, u16lenp - charlen);
    wcsncpy(m_dbuffer + u16lenp - charlen, m_sbuffer, u16len);

    int u8len = _u8len + _u8lenp;
    BYTE* u8str = new BYTE[u8len];

    replace_node_data(_nodep, u16len + u16lenp - charlen, &u8len);
    delete_node(_node);

    return u16lenp - charlen;
}
