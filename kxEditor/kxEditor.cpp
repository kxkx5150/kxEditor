#include "kxEditor.h"
#include "kxEditor_define.h"
#include <Commdlg.h>
#include <Shlwapi.h>
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

ATOM InitMainView(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KXEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KEDITOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return (RegisterClassEx(&wcex));
}
HWND CreateMainView(HINSTANCE hInstance, int nCmdShow, TCHAR* classname)
{
    HWND hWnd = CreateWindowW(classname, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1000, 800, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return hWnd;
}
BOOL InitTextView()
{
    WNDCLASSEX wcx;
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_DBLCLKS;
    wcx.lpfnWndProc = TextViewWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = sizeof(TextEditor*);
    wcx.hInstance = GetModuleHandle(0);
    wcx.hIcon = 0;
    wcx.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wcx.hbrBackground = (HBRUSH)0;
    wcx.lpszMenuName = 0;
    wcx.lpszClassName = TEXTVIEW_CLASS;
    wcx.hIconSm = 0;

    return RegisterClassEx(&wcx) ? TRUE : FALSE;
}
HWND CreateTextView(HWND hwndParent)
{
    LONG opt = 0;
    if (g_fShowScrollH) {
        opt = WS_VSCROLL | WS_HSCROLL | WS_CHILD | WS_VISIBLE;
    } else {
        opt = WS_VSCROLL | WS_CHILD | WS_VISIBLE;
    }
    return CreateWindowEx(WS_EX_CLIENTEDGE,
        TEXTVIEW_CLASS, _T(""),
        opt,
        0, 0, 0, 0,
        hwndParent,
        0,
        0,
        0);
}
BOOL InitWebView()
{
    WNDCLASSEX wcx;
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_DBLCLKS;
    wcx.lpfnWndProc = WebViewWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = sizeof(WebView*);
    wcx.hInstance = GetModuleHandle(0);
    wcx.hIcon = 0;
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)0;
    wcx.lpszMenuName = 0;
    wcx.lpszClassName = WEBVIEW_CLASS;
    wcx.hIconSm = 0;

    return RegisterClassEx(&wcx) ? TRUE : FALSE;
}
HWND CreateWebView(HWND hwndParent)
{
    LONG opt = 0;
    opt = WS_CHILD | WS_VISIBLE;
    return CreateWindowEx(WS_EX_CLIENTEDGE,
        WEBVIEW_CLASS, _T(""),
        opt,
        0, 0, 0, 0,
        hwndParent,
        0,
        0,
        0);
}
HWND CreateTabControl(HWND hWnd)
{
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccx.dwICC = ICC_TAB_CLASSES;
    if (!InitCommonControlsEx(&iccx))
        return FALSE;

    RECT rc;
    GetClientRect(hWnd, &rc);
    HWND hTab = CreateWindowEx(0, WC_TABCONTROL, 0,
        TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE,
        rc.left, rc.top, rc.right, rc.bottom,
        hWnd, (HMENU)IDC_TAB, hInst, 0);

    SendMessage(hTab, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hTab);
    //defEditWndProc = (WNDPROC)SetWindowLongPtr(hTab, GWLP_WNDPROC, (LONG_PTR)EditWindowProc);
    return hTab;
}

void SetWindowFileName(HWND hwnd, TCHAR* szFileName, BOOL fModified)
{
    TCHAR ach[MAX_PATH + 4];
    TCHAR mod[4] = _T("");

    if (fModified)
        lstrcpy(mod, _T(" *"));

    wsprintf(ach, _T("%s - %s"), szFileName, mod);
    SetWindowText(hwnd, ach);
}
void InitOpenFile(HWND hwnd, int fmt)
{
    TCHAR g_szFileName[MAX_PATH];
    TCHAR g_szFileTitle[MAX_PATH];
    g_szFileTitle[0] = '\0';
    g_szFileName[0] = '\0';

    switch (fmt) {
    case ID_FILE_NEW:
        DoOpenFile(hwnd, g_szFileName, g_szFileTitle);
        break;

    default:
        if (ShowOpenFileDlg(hwnd, g_szFileName, g_szFileTitle)) {
            DoOpenFile(hwnd, g_szFileName, g_szFileTitle);
        }
        break;
    }
}
BOOL ShowOpenFileDlg(HWND hwnd, TCHAR* pstrFileName, TCHAR* pstrTitleName)
{
    static OPENFILENAME ofn;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = hInst;
    ofn.lpstrFilter = (TCHAR*)_T("All Files (*.*)\0*.*\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = pstrFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = pstrTitleName;
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | 0; //OFN_ENABLEHOOK			;
    return GetOpenFileName(&ofn);
}
BOOL DoOpenFile(HWND hwndMain, TCHAR* szFileName, TCHAR* szFileTitle)
{
    m_contmgrs[hwndMain]->open_file_container(m_contmgrs[hwndMain]->m_active_cont_no, szFileName);
    if (true) {
        if (_tcslen(szFileName) == 0)
            SetWindowFileName(hwndMain, (TCHAR*)L"Untitled", FALSE);
        else
            SetWindowFileName(hwndMain, szFileTitle, FALSE);

        return TRUE;
    } else {
    }
    return FALSE;
}
void check_node_nodules(TCHAR* path)
{
    if (!PathFileExists(path)) {
        system("start /wait ..\\npm_install.bat");
    }
    return;
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hpins, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    hInst = hInstance;
    lpCmd = nCmdShow;
    check_node_nodules((TCHAR*)L"..\\node\\node_modules\\express\\index.js");
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KEDITOR, szWindowClass, MAX_LOADSTRING);
    InitMainView(hInstance);
    InitTextView();
    InitWebView();

    CreateMainView(hInst, lpCmd, szWindowClass);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEDITOR));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}
void SetWindSize(HWND hwnd, int width, int height)
{
    m_contmgrs[hwnd]->send_resize_msg_containers(width, height, 0, 0);
}
void OnSelChange(HWND mainhwnd, HWND hwnd)
{
    m_contmgrs[mainhwnd]->on_select_tab(hwnd);
}
void create_manager(HWND hWnd)
{
    if (!m_nodemgr)
        m_nodemgr = new NodeMgr();

    m_contmgrs[hWnd] = new ContMgr(m_nodemgr);
    m_contmgrs[hWnd]->create_editor_container(hWnd);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_COMMAND: {
        WndCommandProc(hWnd, message, wParam, lParam);
        break;
    }
    case WM_CREATE: {
        create_manager(hWnd);
        break;
    }
    case WM_DESTROY: {
        delete m_contmgrs[hWnd];
        m_contmgrs.erase(hWnd);
        if (m_contmgrs.size() < 1) {
            delete m_nodemgr;
            PostQuitMessage(0);
        }
        break;
    }
    case WM_SIZE: {
        SetWindSize(hWnd, (short)LOWORD(lParam), (short)HIWORD(lParam));
        break;
    }
    case WM_SETFOCUS: {
        m_contmgrs[hWnd]->set_focus_container(m_contmgrs[hWnd]->m_active_cont_no);
        break;
    }
    case WM_NOTIFY: {
        switch (((LPNMHDR)lParam)->code) {
        case TCN_SELCHANGE:
            OnSelChange(hWnd, ((LPNMHDR)lParam)->hwndFrom);
            break;
        }
    } break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
LRESULT CALLBACK WndCommandProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {

    case ID_HTTP_GET: {
        m_nodemgr->http_get();
        return 0;
    }
    case ID_HTTP_POST: {
        m_nodemgr->http_post();
        return 0;
    }
    case ID_HTTP_WEBSOCKET: {
        //m_nodemgr->bhttp_ws();
        return 0;
    }
    case ID_WEBVIEW_OPEN: {
        m_contmgrs[hWnd]->change_webview();
        return 0;
    }
    case ID_MODE_TERMINAL: {
        m_contmgrs[hWnd]->change_cmdview();
        return 0;
    }
    case ID_TEXTVIEW_OPEN: {
        m_contmgrs[hWnd]->change_txtview();
        return 0;
    }
    case ID_FILE_NEW:
    case ID_FILE_OPEN:
        InitOpenFile(hWnd, LOWORD(wParam));
        return 0;

    case ID_FILE_NEWWINDOW:
        CreateMainView(hInst, lpCmd, szWindowClass);
        return 0;

    case IDM_EXIT:
        DestroyWindow(hWnd);
        return 0;

    case ID_SPLIT_VERTICAL: {
        m_contmgrs[hWnd]->split_vertical();
        //RECT rect;
        //GetClientRect(hWnd, &rect);
        //int width = rect.right - rect.left;
        //int height = rect.bottom - rect.top;
        //SetWindSize(hWnd, width, height);
        return 0;
    }

    case ID_SPLIT_HORIZONTAL: {

        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
LRESULT WINAPI TextViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_NCCREATE:
        return TRUE;

    case WM_NCDESTROY:
        break;

    case WM_SIZE: {
        TextEditor* pte = (TextEditor*)GetWindowLongPtr(hwnd, 0);
        if (pte) {
            pte->m_tabs->m_active_tab->resize_textview();
        }
        break;
    }
    default:
        TextEditor* pte = (TextEditor*)GetWindowLongPtr(hwnd, 0);
        if (pte)
            pte->WndProc(pte->m_contno, hwnd, msg, wParam, lParam);
    }

    return 0;
}
LRESULT CALLBACK WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_NCCREATE:
        return TRUE;

    case WM_NCDESTROY:
        return 0;

    case WM_GETMINMAXINFO: {

    } break;

    case WM_DPICHANGED: {

    } break;

    case WM_CLOSE: {

    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;

    case WM_SETFOCUS: {
        OutputDebugString(L"focus\n");
        //WebView* wbv = (WebView*)GetWindowLongPtr(hWnd, 0);
        //m_contmgrs[hWnd]->set_focus_container(m_contmgrs[hWnd]->m_active_cont_no);
        break;
    }
    case WM_SIZE: {
        WebView* wbv = (WebView*)GetWindowLongPtr(hWnd, 0);
        if (wbv)
            m_contmgrs[wbv->m_hWnd]->send_resize_msg_webview(hWnd);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
LRESULT CALLBACK EditWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //switch (message) {
    //default:
    //    return CallWindowProc(defEditWndProc, hWnd, message, wParam, lParam);
    //}

    return 0;
}
