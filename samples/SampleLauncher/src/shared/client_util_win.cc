// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "shared/client_util.h"

#include <windows.h>
#include <string>
#include <ShlObj.h>

#include "include/cef_browser.h"

namespace shared {

void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                         const CefString& title) {
  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  SetWindowText(hwnd, std::wstring(title).c_str());
}

bool TryGetSpecialFolderPath(SpecialDirectory sd, std::wstring& path)
{
  KNOWNFOLDERID knownId;
  switch(sd)
  {
    case SD_LOCALAPPDATA:
      knownId = FOLDERID_LocalAppData;
      break;
    case SD_COMMONAPPDATA:
      knownId = FOLDERID_ProgramData;
      break;
    default:
      LOG(ERROR) << "Invalid special directory: " << sd;
      return false;
  }

  wchar_t *pPath = NULL;
  if (SHGetKnownFolderPath(knownId, KF_FLAG_DEFAULT, NULL, &pPath) != S_OK)
  {
    LOG(ERROR) << "Failed to get Special Directory: " << GetLastError();
    return false;
  }

  path = pPath;
  CoTaskMemFree(pPath);

  return true;
}

}  // namespace shared
