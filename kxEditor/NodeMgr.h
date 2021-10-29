#pragma once
#include "ContMgr.h"
#include "CmdMgr.h"
#include <string>
#include <tchar.h>
#include <nlohmann/json.hpp>

class NodeMgr {


public:
    TCHAR* m_geturl;
    BOOL m_ws_receive = false;

public:
    NodeMgr();
    ~NodeMgr();

    void init_node();
    void terminate_node();

    std::wstring GetFullPathFor(LPCWSTR relativePath);

    void http_get();
    void http_get_response(TCHAR* resp);
    void http_post();
    void http_post_response(TCHAR* resp);
    
    int beast_ws_start();
    int beast_ws_close();
    int beast_ws_write(ContMgr* contmgr, nlohmann::json j);


private:
    PROCESS_INFORMATION pInfo = {};


};
