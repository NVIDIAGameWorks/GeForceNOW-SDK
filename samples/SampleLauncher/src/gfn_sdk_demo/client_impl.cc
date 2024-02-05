// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "gfn_sdk_demo/client_impl.h"
#include "gfn_sdk_demo/gfn_sdk_helper.h"

#include "include/wrapper/cef_helpers.h"

#include "shared/client_util.h"
#include "shared/resource_util.h"

#include "GfnRuntimeSdk_Wrapper.h"

#ifdef WIN32
#include "shared/resources/win/resource.h"
#include <ShellScalingApi.h>
#endif

namespace message_router {

namespace {

typedef enum {
    DEV_TOOLS_HEIGHT = 600,
    DEV_TOOLS_WIDTH = 900
} DevTools;

// HiDPI The default logical DPI when scaling is applied in windows. see.
// https://msdn.microsoft.com/en-us/library/ms701681(v=vs.85).aspx
#define DEFAULT_WINDOWS_DPI 96

// Handle messages in the browser process.
class MessageHandler : public CefMessageRouterBrowserSide::Handler {
 public:
  explicit MessageHandler(const CefString& startup_url)
      : startup_url_(startup_url) {}

  // Called due to cefQuery execution in message_router.html.
  bool OnQuery(CefRefPtr<CefBrowser> browser,
               CefRefPtr<CefFrame> frame,
               int64 query_id,
               const CefString& request,
               bool persistent,
               CefRefPtr<Callback> callback) OVERRIDE {
    // Only handle messages from the startup URL.
    const std::string& url = frame->GetURL();
    if (url.find(startup_url_) != 0)
      return false;

    if (GfnSdkHelper(browser, frame, query_id, request, persistent, callback)) {
      return true;
    }

    return false;
  }

 private:
  const CefString startup_url_;

  DISALLOW_COPY_AND_ASSIGN(MessageHandler);
};

}  // namespace

Client::Client(const CefString& startup_url)
    : startup_url_(startup_url), browser_ct_(0) {}

void Client::OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
  // Call the default shared implementation.
  shared::OnTitleChange(browser, title);
}

bool Client::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                      CefProcessId source_process,
                                      CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  return message_router_->OnProcessMessageReceived(browser, source_process,
                                                   message);
}

bool Client::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           const CefString& target_url,
                           const CefString& target_frame_name,
                           CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                           bool user_gesture,
                           const CefPopupFeatures& popupFeatures,
                           CefWindowInfo& windowInfo,
                           CefRefPtr<CefClient>& client,
                           CefBrowserSettings& settings,
                           bool* no_javascript_access) {
  CEF_REQUIRE_UI_THREAD();

  return shared::OnBeforePopup(browser, frame, target_url, target_frame_name, target_disposition, user_gesture, popupFeatures, windowInfo, client, settings, no_javascript_access);
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (!message_router_) {
    // Create the browser-side router for query handling.
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterBrowserSide::Create(config);

    // Register handlers with the router.
    message_handler_.reset(new MessageHandler(startup_url_));
    message_router_->AddHandler(message_handler_.get(), false);
    shared::g_browserHost = browser->GetHost();
  }

  browser_ct_++;

#ifdef WIN32
  HWND windowHandle = static_cast<HWND>(browser->GetHost()->GetWindowHandle());
  HICON iconHandle = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_GFNICON));
  SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)iconHandle);
  SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)iconHandle);
#endif // WIN32

  shared::g_browserHost = browser->GetHost();

  // Call the default shared implementation.
  shared::OnAfterCreated(browser);
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::DoClose(browser);
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (--browser_ct_ == 0) {
    // Free the router when the last browser is closed.
    message_router_->RemoveHandler(message_handler_.get());
    message_handler_.reset();
    message_router_ = NULL;
   
    GfnShutdownSdk();
    shared::g_browserHost = NULL;
  }

  // Call the default shared implementation.
  shared::OnBeforeClose(browser);
}

bool Client::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            bool user_gesture,
                            bool is_redirect) {
  CEF_REQUIRE_UI_THREAD();

  message_router_->OnBeforeBrowse(browser, frame);
  return false;
}

CefRefPtr<CefResourceHandler> Client::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  const std::string& url = request->GetURL();

  // This is a minimal implementation of resource loading. For more complex
  // usage (multiple files, zip archives, custom handlers, etc.) you might want
  // to use CefResourceManager. See the "resource_manager" target for an
  // example implementation.
  const std::string& resource_path = shared::GetResourcePath(url);
  if (!resource_path.empty())
    return shared::GetResourceHandler(resource_path);

  return NULL;
}

void Client::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                       TerminationStatus status) {
  CEF_REQUIRE_UI_THREAD();

  message_router_->OnRenderProcessTerminated(browser);
}

void Client::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefContextMenuParams> params,
                                 CefRefPtr<CefMenuModel> model) {
    CEF_REQUIRE_UI_THREAD();

#if 1
    model->AddSeparator();
    // Add a "Show DevTools" item to all context menus.
    model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, "&Show DevTools");
    model->AddItem(CLIENT_ID_INSPECT_ELEMENT, "Inspect Element");
#else
    // Disable right-click context menu since we don't need it for anything
    model->Clear();
#endif
}

bool Client::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefContextMenuParams> params, int command_id,
                                  CefContextMenuHandler::EventFlags event_flags)
{

    switch (command_id)
    {
    case CLIENT_ID_SHOW_DEVTOOLS:
        ShowDevTools(browser, CefPoint());
        return true;
    case CLIENT_ID_INSPECT_ELEMENT:
        ShowDevTools(browser, CefPoint(params->GetXCoord(), params->GetYCoord()));
        return true;
    default: // Allow default handling, if any.
        return true;
    }
}

void Client::ShowDevTools(CefRefPtr<CefBrowser> browser, const CefPoint &inspect_element_at)
{
    CefWindowInfo windowInfo;
    CefBrowserSettings settings;

#ifdef WIN32
    windowInfo.SetAsPopup(NULL, "");
    windowInfo.width = MulDiv(DEV_TOOLS_WIDTH, DEFAULT_WINDOWS_DPI, DEFAULT_WINDOWS_DPI);
    windowInfo.height = MulDiv(DEV_TOOLS_HEIGHT, DEFAULT_WINDOWS_DPI, DEFAULT_WINDOWS_DPI);
#endif

    browser->GetHost()->ShowDevTools(windowInfo, browser->GetHost()->GetClient(), settings, inspect_element_at);
}

}  // namespace message_router
