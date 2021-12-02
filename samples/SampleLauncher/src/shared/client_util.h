// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_EXAMPLES_SHARED_CLIENT_BASE_H_
#define CEF_EXAMPLES_SHARED_CLIENT_BASE_H_

#include "include/cef_client.h"

namespace shared {
    extern CefRefPtr<CefBrowserHost> g_browserHost;

// This file provides functionality common to all CefClient example
// implementations.

// Called from CefDisplayHandler methods:
void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);

// Called from CefLifeSpanHandler methods:
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
                   bool* no_javascript_access);
void OnAfterCreated(CefRefPtr<CefBrowser> browser);
bool DoClose(CefRefPtr<CefBrowser> browser);
void OnBeforeClose(CefRefPtr<CefBrowser> browser);

// Platform-specific implementation.
void PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);

// Returns the contents of |request| as a string.
std::string DumpRequestContents(CefRefPtr<CefRequest> request);

typedef enum
{
    SD_UNDEFINED = 0x00000000,
    SD_LOCALAPPDATA = 0x00000001,   //(Ex: CSIDL_LOCAL_APPDATA)
    SD_COMMONAPPDATA = 0x00000002   //(Ex: CSIDL_COMMON_APPDATA )
} SpecialDirectory;
bool TryGetSpecialFolderPath(SpecialDirectory sd, std::wstring& path);

}  // namespace shared

#endif  // CEF_EXAMPLES_SHARED_CLIENT_BASE_H_
