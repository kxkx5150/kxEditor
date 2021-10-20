#pragma once
#include "LinkedList.h"
#include "TextEditor.h"

#define SEL_REMOVE 0
#define SEL_COPY 1

const int END_OF_LINE = -1;
const int END_OF_LINE_CRLF = -2;

#define NORMAL_MODE 0
#define INSERT_MODE 1
#define VISUAL_MODE 2
#define COMMAND_MODE 3

class TextEditor;
class LinkedList;
class EditView;

enum class SELMODE {
    END,
    START,
    SELECTED,
};
struct XY {
    int x;
    int y;
};

class DocMgr {

private:
    HWND m_hWnd;
    TextEditor* m_txteditr = nullptr;
    LinkedList* m_Document = nullptr;

    EditView* m_editview = nullptr;

private:
    POS stattmp, endtmp;
    SELMODE m_select_mode = SELMODE::END;

public:
    int m_editor_mode = NORMAL_MODE;

    BOOL m_show_caret = false;
    POS m_caret_pos;

    POS m_stat_sel;
    POS m_end_sel;

    TCHAR* m_buffer;

    int m_nWindowLines = 0;
    int m_nWindowColumns = 0;

    long long m_nVScrollMax = 0;
    int m_nHScrollMax = 0;

    long long m_nVScrollPos = 0;
    int m_nHScrollPos = 0;


public:
    DocMgr(LinkedList* doc, TextEditor* txtee, EditView* editview, HWND hwnd);
    ~DocMgr();

    void init();
    void clear();

    void create_caret();
    void destroy_caret();
    void show_caret();
    void hide_caret();
    void set_caret(POS pos);
    void set_caret(long long lineno, int stridx, int arrow = 0);
    void set_caret(int x, int y);
    void set_caret(int x, int y, POS ps);
    long long move_caret(int key);
    void RepositionCaret();

    void clear_select();
    void set_select_start(int x, int y, POS ps);
    SELMODE set_select_move(int mx, int my);
    void set_select_end(int mx, int my);
    void set_select(POS stat, POS end, BOOL backward);
    SELMODE get_selmode();
    BOOL select_paint_mode();
    BOOL check_selected();
    int parse_selected(int mode);
    int remove_selected_area();
    void pos_to_refresh_area(POS stat, POS end, BOOL backward);
    void pos_to_refresh_area_single(POS mstat, POS mend, BOOL backward);
    void pos_to_refresh_area_multiple(POS mstat, POS mend, BOOL backward);

    int XPosToBufferIndex(int x, long long lineno, int* px);
    long long YPosToLineIndex(int y);
    BOOL XYPosToBufferIndex(int x, int y, long long* lineno, int* stridx, int* px, int* py);
    XY buffer_to_absolute_XYpos(long long lineno, int stridx);
    XY buffer_to_offset_XYpos(long long lineno, int stridx);
    int charindex_to_Xpos(int lineno, int stridx);
    BOOL is_painted_area(long long lineno);

    POS insert_text_parser(long long lineno, int stridx, TCHAR* szText, ULONG nLength);
    void remove_text_parser(long long slineno, int sstridx, long long elineno, int estridx);
    void remove_text_parser(POS stat, POS end);
    int get_text_parser(POS stat, POS end, BOOL rm);
    int copy_text_parser(POS stat, POS end, BOOL rm);

    void insert_text(long long lineno, int stridx, TCHAR* szText, ULONG nLengt);
    void remove_text(long long lineno, int stridx, int count, int direction, BOOL surrocheck = true);
    void remove_line(long long lineno);
    int copy_text(long long lineno, int stridx, int count, int direction, int bufferidx = 0, BOOL surrocheck = true);

    void mouse_down(int mx, int my);
    SELMODE mouse_move(int mx, int my);
    void mouse_up(int mx, int my);

    long long keydown_text(TCHAR* szText, ULONG nLength);
    long long keydown_enter(TCHAR* szText, ULONG nLength);
    long long keydown_backspace();
    long long keydown_delete();

    long long on_paste();
    long long on_copy();
    long long on_cut();

    void OnScroll();

private:
};
