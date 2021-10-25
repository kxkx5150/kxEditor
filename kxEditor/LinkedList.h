#pragma once
#include <crtdbg.h>
#include <windows.h>

#define NCP_UTF8 0
#define NCP_UTF8BOM 1
#define NCP_UTF16 2
#define NCP_UTF16BE 3

#define MAX_SIZE 100000

struct POS {
    long long lineno;
    int stridx;
    bool operator==(POS a) const
    {
        if (a.lineno == lineno && a.stridx == stridx)
            return true;
        else
            return false;
    }
    bool operator!=(POS a) const
    {
        if (a.lineno != lineno || a.stridx != stridx)
            return true;
        else
            return false;
    }
    bool operator<(POS a) const
    {
        if (lineno < a.lineno) {
            return true;
        } else if (a.lineno == lineno) {
            if (stridx < a.stridx) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    bool operator>(POS a) const
    {
        if (lineno > a.lineno) {
            return true;
        } else if (lineno == a.lineno) {
            if (stridx > a.stridx) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};
struct _BOM_LOOKUP {
    DWORD bom;
    long long len;
    int type;
};
struct node {
    BYTE* data; // Data
    long long datalen;
    node* prev;
    node* next;
};

class LinkedList {

public:
    TCHAR* m_filename;

    TCHAR* m_sbuffer;
    TCHAR* m_sbuffer_sub;
    TCHAR* m_dbuffer;

private:
    node* front;
    node* end;
    node* pointer;
    long long pointer_lineno = -1;

    long long m_nLineCount = 0;
    long long m_nLongestLine = 0;

    int m_CRLF = 2;
    int m_fileformat = NCP_UTF8;
    int m_headersize = 0;
    int m_tabwidth = 4;

private:
public:
    LinkedList();
    ~LinkedList();

    void init();
    void clear();
    void create_empty();
    bool open_file(TCHAR* filename, int tabwidth);
    bool init_linebuffer(BYTE* buf, long long BYTElength);
    void buffer_to_node(BYTE* buf, long long start, long long end, long long ccount, int tabs);
    int detect_file_format(BYTE* buf, long long BYTElen, int* hdr);

    template <typename T>
    int CRLF_size(T szText, int nLength)
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
    int CRLF_size(TCHAR* szText, int nLength);
    int CRLF_size(BYTE* szText, int nLength);

    node* append_node(BYTE* d, long long len);
    node* insert_node(node* point, BYTE* d, long long len);
    node* insert_node(long long lineno, BYTE* d, long long len);
    node* insert_befor_node(node* point, BYTE* d, long long len);
    node* replace_node_data(node* _node, int u16len, int* u8len);
    int delete_node(node*);
    int delete_all();

    node* set_pointer(long long idx);
    node* get_pointer();
    node* pre(long long step);
    node* next(long long step);
    node* search_node(long long lineno);
    int strindex_to_bufferpos(BYTE* buff, int buflen, int stridx);

    node* iterater(long long lineno, TCHAR* buf, int* buflen);
    int gettext(node* trav, TCHAR* buff, int* buflen);
    int gettext(BYTE* start, long long blen, TCHAR* buff, int* buflen);
    int getbyte(BYTE* buf, long long offset, long long lenBYTEs, long long* pch32);
    void convert_utf16_to_utf8(TCHAR* buff, int buflen, BYTE* u8buf, int* u8len);

    int insert_char(TCHAR* szText, ULONG nLength, node* _node, int u16len, int stridx);
    int remove_char(node* _node, int u16len, int stridx, int count, int direction, BOOL surrocheck = true);
    node* insert_line(node* _node, int u16len, int stridx);
    int remove_line(node* _node);
    int insert_CRLF(node* _node, int u16len, int stridx);
    int remove_CRLF(node* _nodep, node* _node, int u16lenp, int u16len);

    void check_longest_line(BYTE* u8str, int u8len, int u16len);
    int count_tabs(BYTE* buf, long long buflen);

    POS keydown_text(TCHAR* szText, ULONG nLength, long long lineno, int stridx);
    POS keydown_backspace(long long lineno, int stridx, int count);
    POS keydown_delete(long long lineno, int stridx, int count);

    void check_link(BOOL printnd);
    void print_node_data(node* trav);
    long long forward_traverse(BOOL printnd);
    long long backward_traverse(BOOL printnd);

    int getformat();
    void check_CRLF_mode();
    long long linecount();
    long long longestline();
};
