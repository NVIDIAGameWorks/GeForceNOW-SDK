// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <Shlwapi.h>
#include <WinInet.h>
#include <winsock2.h>

#include "gfn_sdk_demo/client_impl.h"
#include "shared/app_factory.h"
#include "shared/browser_util.h"
#include "shared/resource_util.h"
#include "include/base/cef_logging.h"
#include "shared/client_util.h"
#include "shared/defines.h"
#include "shared/main.h"

namespace message_router {

    namespace {
        // Iterates through GfnSdk's shared::kTCPPorts to attempt to find a free one
        bool GetFreePort(std::string & port)
        {
            port.clear();
            char* hostname = nullptr;

            int sockfd = 0;
            struct sockaddr_in serv_addr;
            struct hostent* server;

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                return false;
            }

            server = gethostbyname(hostname);

            if (!server) {
                closesocket(sockfd);
                return false;
            }

            memset((char*)&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            memcpy((char*)server->h_addr,
                (char*)&serv_addr.sin_addr.s_addr,
                server->h_length);

            for (int i = 0; ; i++) {
                const char* testPort = shared::kTCPPorts[i];
                if (!testPort) {
                    closesocket(sockfd);
                    return false;
                }

                serv_addr.sin_port = htons(std::stoi(testPort));
                if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                    // Port is available to use, only the first one is needed
                    closesocket(sockfd);
                    port = testPort;
                    return true;
                }
                
            }
            closesocket(sockfd);
        }

        std::string GetEmbeddedStartupURL() {
            if (!GetFreePort(shared::g_activePort)) {
                return std::string();
            }
            return shared::kTestOrigin + std::string(":") + shared::g_activePort + std::string("/") +  shared::kResourceHTML;
        }

        std::string GetStartupURL() {
            CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
            if (command_line->HasSwitch("startup-uri"))
            {
                std::string startupParam = command_line->GetSwitchValue("startup-uri").ToString();
                if (startupParam.length() > INTERNET_MAX_URL_LENGTH)
                {
                    LOG(ERROR) << "Startup URI parameter too long, falling back to embedded resource!";
                    return GetEmbeddedStartupURL();
                }
                // Check if the path is a relative or absolute path
                if (startupParam.find(':') == std::string::npos)
                {
                    const CefString& programPath = command_line->GetProgram();
                    const std::string& currentPath = programPath.ToString().substr(0, programPath.ToString().find_last_of("/\\") + 1);
                    startupParam = currentPath + startupParam;
                }
                PCSTR pszUnicode = startupParam.c_str();
                CHAR output[INTERNET_MAX_URL_LENGTH];
                DWORD dwDisp = INTERNET_MAX_URL_LENGTH;
                // Convert to a proper file:/// uri value so it matches the CefFrame url after loading
                HRESULT res = UrlCreateFromPathA(pszUnicode, output, &dwDisp, NULL);
                if (res != S_OK && res != S_FALSE)
                {
                    LOG(ERROR) << "Unable to convert startup URI parameter to a valid URL, falling back to embedded resource!";
                    return GetEmbeddedStartupURL();
                }
                std::string startupUri(output);
                return startupUri;
            }
            else
            {
                return GetEmbeddedStartupURL();
            }
        }

    }  // namespace

    void CALLBACK GameProcessClosedBrowserCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

    // Implementation of CefApp for the browser process.
    class BrowserApp : public CefApp, public CefBrowserProcessHandler {
    public:
        BrowserApp() {}

        // CefApp methods:
        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {
            return this;
        }

        // CefBrowserProcessHandler methods:
        void OnContextInitialized() OVERRIDE {
            // Create the browser window.
            const CefString& startup_url = GetStartupURL();
            shared::CreateBrowser(new Client(startup_url), startup_url,
                CefBrowserSettings());

            // Event registration for notification when the child game process is closed)
            HANDLE g_gameClosedEvent = CreateEvent(NULL, TRUE, FALSE, L"Global\\GFNRuntimeSampleLauncherGameClosed");
            HANDLE newWaitHandle = NULL;
            RegisterWaitForSingleObject(&newWaitHandle, g_gameClosedEvent, reinterpret_cast<WAITORTIMERCALLBACK>(&GameProcessClosedBrowserCallback), this, INFINITE, WT_EXECUTEONLYONCE);
        }

        void OnBeforeCommandLineProcessing(
            const CefString& process_type,
            CefRefPtr<CefCommandLine> command_line) {

            command_line->AppendSwitch("disable-gpu");
            command_line->AppendSwitch("disable-gpu-compositing");
            command_line->AppendSwitch("disable-accelerated-compositing");
            command_line->AppendSwitch("disable-webgl");
        }

        void BrowserApp::CloseAllAppBrowsers(void)
        {
            if (!!shared::g_browserHost) {
                shared::g_browserHost->CloseBrowser(true);
            }
            else {
                LOG(ERROR) << "Skipping browser window closure due to invalid host pointer";
            }
        }

    private:
        IMPLEMENT_REFCOUNTING(BrowserApp);
        DISALLOW_COPY_AND_ASSIGN(BrowserApp);
    };

    void CALLBACK GameProcessClosedBrowserCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
    {
        BrowserApp * browserApp = reinterpret_cast<BrowserApp *>(lpParameter);
        if (!browserApp)
        {
            LOG(ERROR) << "Invalid callback data, unable to request application exit";
        }
        CefPostTask(TID_UI, base::Bind(&BrowserApp::CloseAllAppBrowsers, browserApp));
    }

}  // namespace message_router

namespace shared {

    CefRefPtr<CefApp> CreateBrowserProcessApp() {
        return new message_router::BrowserApp();
    }

}  // namespace shared
