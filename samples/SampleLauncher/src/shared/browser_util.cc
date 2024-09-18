// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "shared/browser_util.h"

#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 900

namespace shared {

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class WindowDelegate : public CefWindowDelegate {
public:
  explicit WindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);
    window->Show();

    // Give keyboard focus to the browser view.
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
    browser_view_ = NULL;
  }

  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
      return browser->GetHost()->TryCloseBrowser();
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) OVERRIDE {
    // Preferred window size.
    return CefSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  }

  CefSize GetMinimumSize(CefRefPtr<CefView> view) OVERRIDE {
    // Minimum window size.
    return CefSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  }

  bool CanResize(CefRefPtr<CefWindow> window) OVERRIDE {
    return false;
  }

  bool CanMaximize(CefRefPtr<CefWindow> window) OVERRIDE {
    return false;
  }

  bool CanMinimize(CefRefPtr<CefWindow> window) OVERRIDE {
    return false;
  }

private:
  CefRefPtr<CefBrowserView> browser_view_;

  IMPLEMENT_REFCOUNTING(WindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(WindowDelegate);
};

class PopupBrowserViewDelegate : public CefBrowserViewDelegate {
public:
  explicit PopupBrowserViewDelegate() {}

  CefSize GetPreferredSize(CefRefPtr<CefView> view) OVERRIDE {
    return CefSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  }

private:
  IMPLEMENT_REFCOUNTING(PopupBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(PopupBrowserViewDelegate);
};

class BrowserViewDelegate : public CefBrowserViewDelegate {
public:
  explicit BrowserViewDelegate() {}

  // CefBrowserViewDelegate methods:
  CefRefPtr<CefBrowserViewDelegate> GetDelegateForPopupBrowserView(
    CefRefPtr<CefBrowserView> browser_view,
    const CefBrowserSettings& settings,
    CefRefPtr<CefClient> client,
    bool is_devtools) OVERRIDE {
    return new PopupBrowserViewDelegate();
  }

private:
  IMPLEMENT_REFCOUNTING(BrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(BrowserViewDelegate);
};

}  // namespace

void CreateBrowser(CefRefPtr<CefClient> client,
                   const CefString& startup_url,
                   const CefBrowserSettings& settings) {
  CEF_REQUIRE_UI_THREAD();

#if defined(OS_WIN) || defined(OS_LINUX)
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // Create the browser using the Views framework if "--use-views" is specified
  // via the command-line. Otherwise, create the browser using the native
  // platform framework. The Views framework is currently only supported on
  // Windows and Linux.
  const bool use_views = false; // !command_line->HasSwitch("disable_views");
#else
  const bool use_views = false;
#endif

  if (use_views) {
    // Create the BrowserView.
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        client, startup_url, settings, NULL, new BrowserViewDelegate());

    // Create the Window. It will show itself after creation.
    CefWindow::CreateTopLevelWindow(new WindowDelegate(browser_view));
  } else {
    // Information used when creating the native window.
    CefWindowInfo window_info;

#if defined(OS_WIN)
    // On Windows we need to specify certain flags that will be passed to
    // CreateWindowEx().
    window_info.SetAsPopup(NULL, "GFN SDK Sample Launcher");
    // Adjust window dimensions for system DPI settings
    HDC screen = GetDC(0);
    int systemDpi = GetDeviceCaps(screen, LOGPIXELSX);
    // We can't programmatically know the side of the rendered HTML without
    // resizing windows after the fact. Size this to fit the HTML.
    int dpiAdjustedWidth = MulDiv(WINDOW_WIDTH, systemDpi, 96);
    int dpiAdjustedHeight = MulDiv(WINDOW_HEIGHT, systemDpi, 96);
    window_info.width = dpiAdjustedWidth;
    window_info.height = dpiAdjustedHeight;
#else
    window_info.width = WINDOW_WIDTH;
    window_info.height = WINDOW_HEIGHT;
#endif

    // Create the browser window.
    CefBrowserHost::CreateBrowser(window_info, client, startup_url, settings, NULL);
  }
}

}  // namespace shared
