#pragma once
#define WIN32_LEAN_AND_MEAN
#include "resource.h"
#include "targetver.h"
#include "TextEditor.h"
#include "ContMgr.h"
#include "NodeMgr.h"
#include "CmdMgr.h"
#include "StatusBar.h"
#include <wil/com.h>
#include <windows.h>
#include <wrl.h>
#include "WebView2.h"

HINSTANCE hInst;
HWND g_hwndStatusbar;
HWND g_hwndwebview;

extern NodeMgr* m_nodemgr = nullptr;
ContMgr* m_contmgr = nullptr;
CmdMgr* m_cmdmgr = nullptr;

BOOL g_fShowStatusbar = true;
extern BOOL g_fShowScrollH = false;

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
TCHAR g_szAppName[] = APP_TITLE;
TCHAR g_szFileName[MAX_PATH];
TCHAR g_szFileTitle[MAX_PATH];

void InitOpenFile(HWND hwnd, int fmt);
BOOL ShowOpenFileDlg(HWND hwnd, TCHAR* pstrFileName, TCHAR* pstrTitleName);
BOOL DoOpenFile(HWND hwndMain, TCHAR* szFileName, TCHAR* szFileTitle);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndCommandProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI TextViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);