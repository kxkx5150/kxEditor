#pragma once

#define MAX_LOADSTRING 100
#define APP_TITLE _T("kxEditor")

#define TXM_BASE (WM_USER)
#define TXM_OPENFILE (TXM_BASE + 0)
#define TXM_CLEAR (TXM_BASE + 1)
#define TXM_SETLINESPACING (TXM_BASE + 2)
#define TXM_ADDFONT (TXM_BASE + 3)
#define TXM_SETCOLOR (TXM_BASE + 4)
#define TXM_GETCOLOR (TXM_BASE + 5)
#define TXM_SETSTYLE (TXM_BASE + 6)
#define TXM_GETSTYLE (TXM_BASE + 7)
#define TXM_SETCARETWIDTH (TXM_BASE + 8)
#define TXM_SETIMAGELIST (TXM_BASE + 9)
#define TXM_SETLONGLINE (TXM_BASE + 10)
#define TXM_SETLINEIMAGE (TXM_BASE + 11)
#define TXM_GETFORMAT (TXM_BASE + 12)
#define TXM_UNDO (TXM_BASE + 13)
#define TXM_REDO (TXM_BASE + 14)
#define TXM_CANUNDO (TXM_BASE + 15)
#define TXM_CANREDO (TXM_BASE + 16)
#define TXM_GETSELSIZE (TXM_BASE + 17)
#define TXM_SETSELALL (TXM_BASE + 18)
#define TXM_GETCURPOS (TXM_BASE + 19)
#define TXM_GETCURLINE (TXM_BASE + 20)
#define TXM_GETCURCOL (TXM_BASE + 21)
#define TXM_SETEDITMODE (TXM_BASE + 22)
#define TXM_GETEDITMODE (TXM_BASE + 23)
#define TXM_SETCONTEXTMENU (TXM_BASE + 24)

#define MAX_FONTS 32
#define COURIERNEW 1
#define LUCIDACONS 2
#define REGLOC _T("SOFTWARE\\kxkx5150\\kx_editor")
#define TEXTVIEW_CLASS _T("TextView")
#define WEBVIEW_CLASS _T("WebView")

#define TXC_FOREGROUND 0 // normal foreground colour
#define TXC_BACKGROUND 1 // normal background colour
#define TXC_HIGHLIGHTTEXT 2 // normal text highlight colour
#define TXC_HIGHLIGHT 3 // normal background highlight colour
#define TXC_HIGHLIGHTTEXT2 4 // inactive text highlight colour
#define TXC_HIGHLIGHT2 5 // inactive background highlight colour
#define TXC_SELMARGIN1 6 // selection margin colour#1
#define TXC_SELMARGIN2 7 // selection margin colour#2
#define TXC_LINENUMBERTEXT 8 // line number text
#define TXC_LINENUMBER 9 // line number background
#define TXC_LONGLINETEXT 10 // long-line text
#define TXC_LONGLINE 11 // long-line background
#define TXC_CURRENTLINETEXT 12 // active line text
#define TXC_CURRENTLINE 13 // active line background
#define TXC_MAX_COLOURS 14 // keep this updated!

#define TXL_LF 1 // line-feed
#define TXL_CR 2 // carriage-return
#define TXL_CRLF 4 // carriage-return, line-feed (default)
#define TXL_ALL 7 // allow all forms regardless

#define LINENO_FORMAT _T(" %2d ")

#define MODE_READONLY 0
#define MODE_INSERT 1
#define MODE_OVERWRITE 2

#define SYSCOL(COLOR_IDX) (0x80000000 | COLOR_IDX)
//
#define TextView_OpenFile(szFile) \
    SendMessage(g_hwndTextView, TXM_OPENFILE, 0, (LPARAM)(TCHAR*)(szFile))

