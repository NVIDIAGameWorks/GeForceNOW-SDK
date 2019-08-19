// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "gfn_sdk_demo/client_impl.h"
#include "shared/app_factory.h"
#include "shared/browser_util.h"
#include "shared/resource_util.h"

namespace message_router {

namespace {

std::string GetStartupURL() {
  return shared::kTestOrigin + std::string("gfn_sdk.html");
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
