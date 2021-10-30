#pragma once
#define WIN32_LEAN_AND_MEAN
#include "resource.h"
#include "targetver.h"
#include "TextEditor.h"
#include "ContMgr.h"
#include "NodeMgr.h"
#include "CmdMgr.h"
#include <wil/com.h>
#include <windows.h>
#include <wrl.h>
#include "WebView.h"
#include "WebView2.h"

TCHAR g_szAppName[] = APP_TITLE;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

HINSTANCE hInst;
int lpCmd;
NodeMgr* m_nodemgr = nullptr;
std::map<HWND, ContMgr*> m_contmgrs;

extern BOOL g_fShowScrollH = false;
extern BOOL g_fShowTabControl = true;

void InitOpenFile(HWND hwnd, int fmt);
BOOL ShowOpenFileDlg(HWND hwnd, TCHAR* pstrFileName, TCHAR* pstrTitleName);
BOOL DoOpenFile(HWND hwndMain, TCHAR* szFileName, TCHAR* szFileTitle);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndCommandProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI TextViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
