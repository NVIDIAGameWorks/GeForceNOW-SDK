// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_EXAMPLES_MESSAGE_ROUTER_CLIENT_IMPL_H_
#define CEF_EXAMPLES_MESSAGE_ROUTER_CLIENT_IMPL_H_

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

namespace message_router {

// Implementation of client handlers.
class Client : public CefClient,
               public CefDisplayHandler,
               public CefLifeSpanHandler,
               public CefRequestHandler,
               public CefContextMenuHandler {
 public:
  explicit Client(const CefString& startup_url);

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
  CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }
  CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) OVERRIDE;

  // CefLifeSpanHandler methods:
  bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     const CefString& target_url,
                     const CefString& target_frame_name,
                     CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                     bool user_gesture,
                     const CefPopupFeatures& popupFeatures,
                     CefWindowInfo& windowInfo,
                     CefRefPtr<CefClient>& client,
                     CefBrowserSettings& settings,
                     bool* no_javascript_access) OVERRIDE;
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefRequestHandler methods:
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request,
                      bool user_gesture,
                      bool is_redirect) OVERRIDE;
  CefRefPtr<CefResourceHandler> GetResourceHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request) OVERRIDE;
  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 TerminationStatus status) OVERRIDE;

  // CefContextMenuHandler
  void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefContextMenuParams> params,
                           CefRefPtr<CefMenuModel> model) OVERRIDE;

 private:
  // Handles the browser side of query routing.
  CefRefPtr<CefMessageRouterBrowserSide> message_router_;
  scoped_ptr<CefMessageRouterBrowserSide::Handler> message_handler_;

  const CefString startup_url_;

  // Track the number of browsers using this Client.
  int browser_ct_;

  IMPLEMENT_REFCOUNTING(Client);
  DISALLOW_COPY_AND_ASSIGN(Client);
};

}  // namespace message_router

#endif  // CEF_EXAMPLES_MESSAGE_ROUTER_CLIENT_IMPL_H_
