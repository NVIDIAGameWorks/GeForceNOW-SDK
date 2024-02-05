// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "gfn_sdk_demo/client_impl.h"
#include "shared/app_factory.h"
#include "shared/browser_util.h"
#include "shared/resource_util.h"
#include "shared/defines.h"
#include "shared/main.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

namespace message_router {

namespace {

    // Function to find a free port from a list of given ports
    bool GetFreePort(std::string & freePort)
    {
        int sockfd;
        struct sockaddr_in serv_addr;

        // Initialize socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return false;
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        for (size_t i = 0; i < sizeof(shared::kTCPPorts) / sizeof(shared::kTCPPorts[0]); i++)
        {
            serv_addr.sin_port = htons(std::stoi(shared::kTCPPorts[i]));
            
            // Try to bind the port
            if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
                // Successfully bound, this port is free
                close(sockfd);  // Important to release the port
                freePort = shared::kTCPPorts[i];
                return true;
            }
        }

        close(sockfd);
        return false;
    }

    std::string GetEmbeddedStartupURL() {
        if (!GetFreePort(shared::g_activePort)) {
            return std::string();
        }
        return shared::kTestOrigin + std::string(":") + shared::g_activePort + std::string("/") +  shared::kResourceHTML;
    }

    std::string GetStartupURL() {
        return GetEmbeddedStartupURL();
    }

}  // namespace

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
  }

  void OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) OVERRIDE {

    command_line->AppendSwitch("disable-gpu");
    command_line->AppendSwitch("disable-gpu-compositing");
    command_line->AppendSwitch("disable-accelerated-compositing");
    command_line->AppendSwitch("disable-webgl");
  }

 private:
  IMPLEMENT_REFCOUNTING(BrowserApp);
  DISALLOW_COPY_AND_ASSIGN(BrowserApp);
};

}  // namespace message_router

namespace shared {

CefRefPtr<CefApp> CreateBrowserProcessApp() {
  return new message_router::BrowserApp();
}

}  // namespace shared
