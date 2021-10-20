#include "StatusBar.h"
#include <crtdbg.h>



DWORD dwStatusBarStyles = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NOMOVEY | //| CCS_NORESIZE
    SBT_NOBORDERS | SBARS_SIZEGRIP;

//	Process WM_MENUSELECT message to display menu-item hints in statusbar
//
int StatusBarMenuSelect(HWND hwnd, HWND hwndSB, WPARAM wParam, LPARAM lParam)
{
    //UINT lookup[] = { 0, 0 };

    //// Display helpful text in status bar
    //MenuHelp(WM_MENUSELECT, wParam, lParam, GetMenu(hwnd), g_hResourceModule,
    //	hwndSB, (UINT *)lookup);

    return 0;
}
void SetStatusBarParts(HWND hwndSB)
{
    RECT r;
    HWND hwndParent = GetParent(hwndSB);
    int parts[MAX_STATUS_PARTS];
    int parentwidth;

    GetClientRect(hwndParent, &r);

    parentwidth = r.right < 400 ? 400 : r.right;
    parts[0] = parentwidth - 320;
    parts[1] = parentwidth - 120;
    parts[2] = parentwidth; //-1;

    SendMessage(hwndSB, SB_SETPARTS, MAX_STATUS_PARTS, (LPARAM)parts);
}
void SetStatusBarText(HWND hwndSB, UINT nPart, UINT uStyle, TCHAR* fmt, ...)
{
    TCHAR tmpbuf[100];
    va_list args;
    va_start(args, fmt);
    vswprintf(tmpbuf,100, fmt, args);
    va_end(args);
    SendMessage(hwndSB, SB_SETTEXT, (WPARAM)(nPart | uStyle), (LPARAM)(LPCWSTR)tmpbuf);
}
HWND CreateStatusBar(HWND hwndParent)
{
    HWND hwndSB;
    hwndSB = CreateStatusWindow(dwStatusBarStyles, _T(""), hwndParent, 2);
    SetStatusBarParts(hwndSB);
    SetStatusBarText(hwndSB, 0, 1, (TCHAR*)_T(""));
    SetStatusBarText(hwndSB, 1, 0, (TCHAR*)_T(" Ln %d, Col %d"), 1, 1);
    SetStatusBarText(hwndSB, 2, 0, (TCHAR*)_T(" UTF-8"));
    if (!hwndSB) {
        OutputDebugString(L"Error CreateStatusWindow\n");
    }
    _RPTN(_CRT_WARN, "------- statusbar handle : %p\n", hwndSB);
    return hwndSB;
}
