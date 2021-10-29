#define _CRT_SECURE_NO_WARNINGS
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cpprest/http_client.h>
#include "NodeMgr.h"
#include "Unicode.h"
#include "Shellapi.h"
#include <crtdbg.h>

extern NodeMgr* m_nodemgr;

using namespace web;
using namespace web::http;
using namespace web::http::client;
pplx::task<void> Get();
pplx::task<int> Post();

namespace beast = boost::beast;        
namespace bhttp = beast::http;         
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;      
net::io_context ioc;
tcp::resolver resolver { ioc };
websocket::stream<tcp::socket> b_ws { ioc };


NodeMgr::NodeMgr()
{
    SetConsoleOutputCP(CP_UTF8);
    init_node();
    Sleep(300);
    beast_ws_start();
}

NodeMgr::~NodeMgr()
{
    beast_ws_close();
    //terminate_node();
}
DWORD RunSilent(TCHAR* strFunct, TCHAR* strstrParams)
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    TCHAR Args[4096];
    TCHAR* pEnvCMD = NULL;
    TCHAR pDefaultCMD[] = L"CMD.EXE";
    ULONG rc;

    memset(&StartupInfo, 0, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(STARTUPINFO);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;
    Args[0] = 0;

    pEnvCMD = _wgetenv(L"COMSPEC");

    if (pEnvCMD) {
        wcscpy(Args, pEnvCMD);
    } else {
        wcscpy(Args, pDefaultCMD);
    }

    wcscat(Args, L" /c ");
    wcscat(Args, strFunct);
    wcscat(Args, L" ");
    wcscat(Args, strstrParams);


    if (!CreateProcess(NULL, Args, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInfo)) {
        return GetLastError();
    }

    //WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
    if (!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
        rc = 0;

    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);

    return rc;
}
void NodeMgr::init_node()
{
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pInfo, sizeof(pInfo));
    TCHAR cmd[100] = L"..\\node\\node.exe ..\\node\\index.js";
    STARTUPINFO sInfo = {};
    GetStartupInfo(&sInfo);
    BOOL bResult = CreateProcess(NULL, cmd, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, L"..\\node", &si, &pInfo);
    if (!bResult) {
        OutputDebugString(L"CreateProcess Error\n");
        ::CloseHandle(pInfo.hThread);
        ::CloseHandle(pInfo.hProcess);
        return;
    }
}
std::wstring NodeMgr::GetFullPathFor(LPCWSTR relativePath)
{
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(GetModuleHandle(0), path, MAX_PATH);
    std::wstring pathName(path);
    std::size_t index = pathName.find_last_of(L"\\") + 1;
    pathName.replace(index, pathName.length(), relativePath);
    return pathName;
}
void NodeMgr::terminate_node()
{
    TerminateProcess(pInfo.hProcess, 0);
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
}

void NodeMgr::http_get()
{
    m_geturl = (TCHAR*)L"http://localhost:3000";
    //m_geturl = (TCHAR*)L"https://www.yahoo.co.jp";
    Get();
}
void NodeMgr::http_get_response(TCHAR* resp)
{
    int len = _tcsclen(resp);
    //m_contmgr->m_containers[m_contmgr->m_active_cont_no].tabs->m_active_tab->m_docmgr->insert_text(0, 0, resp, len);
}
void NodeMgr::http_post()
{
    m_geturl = (TCHAR*)L"http://localhost:3000";
    Post();
}
void NodeMgr::http_post_response(TCHAR* resp)
{
    int len = _tcsclen(resp);
    //m_contmgr->m_containers[m_contmgr->m_active_cont_no].tabs->m_active_tab->m_docmgr->insert_text(0, 0, resp, len);
}
pplx::task<void> Get()
{
    return pplx::create_task([] {
        http_client client(m_nodemgr->m_geturl);
        return client.request(methods::GET);
    }).then([](http_response response) {
        if (response.status_code() == status_codes::OK) {
            OutputDebugString(L"------http get ok-------\n");
            std::wstring json_bom = response.extract_string().get().c_str();
            const TCHAR* wcs = json_bom.c_str();
            m_nodemgr->http_get_response((TCHAR*)wcs);
            //return response.extract_json();
        }
    });
    //    .then([](json::value json) {
    //    //std::wstring json_bom = json[L"title"].as_string();
    //    //const wchar_t* wcs = json_bom.c_str();
    //    //OutputDebugString(wcs);
    //});
}
pplx::task<int> Post()
{
    return pplx::create_task([] {
        json::value postData;
        auto sex = utility::conversions::to_string_t("man");
        postData[L"user_info"][L"name"] = json::value::string(L"userあいうえお");
        postData[L"user_info"][L"sex"] = json::value::string(sex);
        postData[L"user_info"][L"age"] = json::value::number(20);
        http_client client(m_nodemgr->m_geturl);
        return client.request(methods::POST, L"", postData.serialize(), L"application/json");
    }).then([](http_response response) {
        if (response.status_code() == status_codes::OK) {
            OutputDebugString(L"------http post ok-------\n");
            std::wstring json_bom = response.extract_string().get().c_str();
            const TCHAR* wcs = json_bom.c_str();
            m_nodemgr->http_post_response((TCHAR*)wcs);
            //auto body = response.extract_string();
            //std::wcout << body.get().c_str() << std::endl;
            //std::cout << response.extract_json() << std::endl;
            //return response.extract_json();
            return 0;
        }
    });
    //    .then([](json::value json) {
    //    // リザルトコードを返す
    //    return json[L"result"].as_integer();
    //});
}

int NodeMgr::beast_ws_start()
{
    std::string host = "localhost";
    const char port[] = "3001";

    try {
        auto const results = resolver.resolve(host, port);
        auto ep = net::connect(b_ws.next_layer(), results);
        host += ':' + std::to_string(ep.port());
        b_ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(bhttp::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
            }));

        b_ws.handshake(host, "/");
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int NodeMgr::beast_ws_close()
{
    b_ws.close(websocket::close_code::normal);
    return EXIT_SUCCESS;
}
int NodeMgr::beast_ws_write(ContMgr* contmgr,nlohmann::json j)
{
    std::string s = j.dump();
    b_ws.write(net::buffer(std::string(s)));
    std::string str;
    auto buf = boost::asio::dynamic_buffer(str);
    b_ws.read(buf);
    contmgr->m_cmdmgr->exec(str);

    return EXIT_SUCCESS;
}

