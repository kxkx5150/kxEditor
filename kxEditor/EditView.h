#pragma once
extern "C" {
#include "..\UspLib\usplib.h"
}
#include "DocMgr.h"
#include "LinkedList.h"
#include "TextEditor.h"

#define USP_CACHE_SIZE 300

class TextEditor;
class Tabs;

class DocMgr;
struct FONT;


struct USPCACHE {
    USPDATA* uspData;
    ULONG lineno; // line#
    ULONG offset; // offset (in WCHAR's) of this line
    ULONG usage; // cache-count

    int length; // length in chars INCLUDING CR/LF
    int length_CRLF; // length in chars EXCLUDING CR/LF
};

class EditView {
public:
    USPCACHE* m_uspCache;

    POS m_paint_sel_stat;
    POS m_paint_sel_end;

private:
    HWND m_hWnd;
    TextEditor* m_txteditr = nullptr;
    Tabs* m_tabs = nullptr;

    TCHAR* m_buff = nullptr;
    ATTR* m_attr = nullptr;

private:



public:
    EditView(HWND m_hWnd, TextEditor* txteditr, Tabs* tabs);
    ~EditView();

    USPDATA* GetUspData(HDC hdc, ULONG nLineNo, ULONG* off_chars = 0);
    int ApplySelection(USPDATA* uspData, ULONG nLine, ULONG nOffset, ULONG nTextLen);
    void reset_usp_cache();
    int StripCRLF(TCHAR* szText, ATTR* attr, int nLength, bool fAllow);

    LONG OnPaint();
    void PaintLine(HDC hdc, ULONG nLineNo, int xpos, int ypos, HRGN hrgnUpdate);
    void PaintText(HDC hdc, ULONG nLineNo, int xpos, int ypos, RECT* bounds);
    int set_textatributes(long long nLineNo, long long nOffset, long long& nColumn, TCHAR* szText, int nTextLen, ATTR* attr);
    int NeatTextYOffset(USPFONT* font);
    void PaintRect(HDC hdc, RECT* rect, COLORREF fill);
    void PaintRect(HDC hdc, int x, int y, int width, int height, COLORREF fill);
    void clear_select_pos();

    void RefreshWindow();

    // editor left margin
    int LeftMarginWidth();
    int PaintMargin(HDC hdc, ULONG nLineNo, int xpos, int ypos);
    void DrawCheckedRect(HDC hdc, RECT* rect, COLORREF fg, COLORREF bg);
};
