#pragma once
#include "TextEditor.h"
#include <string>

class ContMgr;
class NodeMgr;
struct EditorContainer;
extern NodeMgr* m_nodemgr;

class CmdMgr {
private:
    HWND m_hwnd = nullptr;
    ContMgr* m_contmgr = nullptr;

public:
    CmdMgr(ContMgr* contmgr);
    ~CmdMgr();
    void set_hwnd(HWND hwnd);

    LONG on_keydown(int contno, UINT nKeyCode, UINT nFlags);
    LONG OnChar(int contno, UINT nChar, UINT nFlags);
    LONG exec(std::string message);
    void send_keycode_json(int contno, std::string type, UINT nKeyCode, BOOL ctrl, BOOL shift, BOOL alt);


private:
};
