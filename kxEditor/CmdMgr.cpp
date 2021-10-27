#include "CmdMgr.h"
#include <nlohmann/json.hpp>
#include <windows.h>

CmdMgr::CmdMgr(ContMgr* contmgr)
{
    m_contmgr = contmgr;
}

CmdMgr::~CmdMgr()
{
}
void CmdMgr::set_hwnd(HWND hwnd)
{
    m_hwnd = hwnd;
}
LONG CmdMgr::on_keydown(int contno, UINT nKeyCode, UINT nFlags)
{
    bool ctrl = GetKeyState(VK_CONTROL) < 0 ? true : false;
    bool shift = GetKeyState(VK_SHIFT) < 0 ? true : false;
    bool alt = GetKeyState(VK_MENU) < 0 ? true : false;

    switch (nKeyCode) {
    case VK_SHIFT:
    case VK_CONTROL:
        break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
        if (ctrl || alt)
            send_keycode_json(contno, "keydown", nKeyCode, ctrl, shift, alt);

        break;
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_BACK:
    case VK_DELETE:
        send_keycode_json(contno, "keydown", nKeyCode, ctrl, shift, alt);
        break;
    }
    return 0;
}
LONG CmdMgr::OnChar(int contno, UINT nChar, UINT nFlags)
{
    TCHAR ch = (TCHAR)nChar;

    if (nChar == '\r') {
        OnChar(contno, (UINT)'\n', nFlags);

    } else if (nChar == '\n') {
        m_contmgr->m_containers[contno].tabs->m_active_tab->m_docmgr->keydown_enter(&ch, 1);

    } else if (nChar > 31 || nChar == '\t') {
        m_contmgr->m_containers[contno].tabs->m_active_tab->m_docmgr->keydown_text(&ch, 1);
    }

    return 0;
}
void CmdMgr::send_keycode_json(int contno, std::string type, UINT nKeyCode, BOOL ctrl, BOOL shift, BOOL alt)
{
    nlohmann::json j;
    j["contno"] = contno;
    j["tabno"] = m_contmgr->m_containers[contno].tabs->m_active_tab_no;
    j["mode"] = m_contmgr->m_containers[contno].tabs->m_active_tab->m_docmgr->m_editor_mode;
    j["type"] = type;
    j["keycode"] = nKeyCode;
    j["ctrl"] = ctrl;
    j["shift"] = shift;
    j["alt"] = alt;
    m_nodemgr->beast_ws_write(j);
}
LONG CmdMgr::exec(std::string message)
{
    nlohmann::json j = nlohmann::json::parse(message);

    for (int i = 0; i < j["commands"].size(); ++i) {
        if (j["commands"][i]["type"] == "caret") {

            if (j["commands"][i]["command"] == "right") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->move_caret(VK_RIGHT);

            } else if (j["commands"][i]["command"] == "left") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->move_caret(VK_LEFT);

            } else if (j["commands"][i]["command"] == "up") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->move_caret(VK_UP);

            } else if (j["commands"][i]["command"] == "down") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->move_caret(VK_DOWN);
            }

        } else if (j["commands"][i]["type"] == "clipboard") {

            if (j["commands"][i]["command"] == "paste") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->on_paste();

            } else if (j["commands"][i]["command"] == "copy") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->on_copy();

            } else if (j["commands"][i]["command"] == "cut") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->on_cut();
            }

        } else if (j["commands"][i]["type"] == "input") {

            if (j["commands"][i]["command"] == "backspace") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->keydown_backspace();

            } else if (j["commands"][i]["command"] == "delete") {
                m_contmgr->m_containers[j["contno"]].tabs->m_tabs[j["tabno"]]->m_docmgr->keydown_delete();

            }

        } else if (j["type"] == "none") {
            OutputDebugString(L"none\n");
        }
    }

    return 0;
}
