// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "shared/main.h"

#include <windows.h>
#include <Shlobj.h>

#include "include/cef_sandbox_win.h"

#include "shared/app_factory.h"
#include "shared/client_manager.h"
#include "shared/main_util.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.

namespace shared {

bool TryGetSpecialFolderPath(REFKNOWNFOLDERID rfid, std::wstring& path);
void Launch(int cmsid);

// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance) {
  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();

  void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // Create a temporary CommandLine object.
  CefRefPtr<CefCommandLine> command_line = CreateCommandLine(main_args);

  // Create a CefApp of the correct process type.
  CefRefPtr<CefApp> app;
  switch (GetProcessType(command_line)) {
    case PROCESS_TYPE_BROWSER:
      app = CreateBrowserProcessApp();
      break;
    case PROCESS_TYPE_RENDERER:
      app = CreateRendererProcessApp();
      break;
    case PROCESS_TYPE_OTHER:
      app = CreateOtherProcessApp();
      break;
  }

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Create the singleton manager instance.
  ClientManager manager;

  // Specify CEF global settings here.
  CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif
  settings.remote_debugging_port = 8080;

  // Get application data directory
  std::wstring appDataPath;
  TryGetSpecialFolderPath(FOLDERID_Documents, appDataPath);
  CefString(&settings.log_file).FromWString(appDataPath + L"\\debug.txt");

  // Initialize CEF for the browser process. The first browser instance will be
  // created in CefBrowserProcessHandler::OnContextInitialized() after CEF has
  // been initialized.
  CefInitialize(main_args, settings, app, sandbox_info);

  std::string cmd(command_line->GetCommandLineString().ToString());
  LOG(INFO) << "Command line: " << cmd.c_str();
  std::size_t pos = cmd.find_first_of('=');
  if (pos != std::string::npos) {
      int cmsid = std::stoi(cmd.substr(pos + 1, cmd.length()));
      LOG(INFO) << "CMSID: " << cmsid;
      Launch(cmsid);
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}

#include <map>
std::map<int, std::wstring> cmsidMap = { {16032111, L"C:\\Program Files (x86)\\NVIDIA_Grid_Bundle\\nvidia_vxgi_apollo\\start.bat"} };

void Launch(int cmsid)
{
    std::map<int, std::wstring>::iterator it;
    it = cmsidMap.find(cmsid);
    if (it != cmsidMap.end()) {
        std::wstring cmdline = it->second;
        LOG(INFO) << "Starting game process: " << cmdline.c_str();

        STARTUPINFO startupInfo = {};
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION procInfo = {};
        bool cp = TRUE == CreateProcess(nullptr, const_cast<LPWSTR>(cmdline.c_str()),
            nullptr, nullptr, FALSE, 0,
            nullptr, nullptr, &startupInfo, &procInfo);

        if (!cp)
        {
            LOG(ERROR) << "Failed to start game process";
            return;
        }

        CloseHandle(procInfo.hThread);
        CloseHandle(procInfo.hProcess);
    }
}

bool TryGetSpecialFolderPath(REFKNOWNFOLDERID rfid, std::wstring& path)
{
    wchar_t * pPath = nullptr;
    auto result = SHGetKnownFolderPath(rfid, KF_FLAG_DEFAULT, nullptr, &pPath);
    if (FAILED(result))
    {
        LOG(ERROR) << "Failed to get known folder path. HR: " << result;
        CoTaskMemFree(pPath);
        return false;
    }

    // Make a copy to the path string before we release it.
    path = pPath;
    CoTaskMemFree(pPath);

    return SUCCEEDED(result);
}



}  // namespace shared
