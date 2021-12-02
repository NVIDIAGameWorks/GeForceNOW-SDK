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
#include "shared/client_util.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.

namespace shared {
    HANDLE g_gameClosedEvent;

    std::string g_activePort;

    // Callback definition used if there is a child game process, and it is closed
    void CALLBACK GameProcessClosedCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

    bool Launch(int cmsid, HANDLE & gameProcHandle);

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
        TryGetSpecialFolderPath(shared::SD_COMMONAPPDATA, appDataPath);
        appDataPath += L"\\NVIDIA Corporation\\GfnRuntimeSdk";

        int err = SHCreateDirectoryEx(NULL, appDataPath.c_str(), NULL);
        if (err != ERROR_SUCCESS && err != ERROR_ALREADY_EXISTS)
        {
            std::wstringstream ss;
            ss << "Could not create log output directory " << appDataPath << " err=" << err;
            fwprintf(stderr, ss.str().c_str());
            OutputDebugStringW(ss.str().c_str());
        }
        CefString(&settings.log_file).FromWString(appDataPath + L"\\GfnSdkSampleLauncher.log");

        // Initialize CEF for the browser process. The first browser instance will be
        // created in CefBrowserProcessHandler::OnContextInitialized() after CEF has
        // been initialized.
        CefInitialize(main_args, settings, app, sandbox_info);

        LOG(INFO) << "Command line: " << command_line->GetCommandLineString();
        if (command_line->HasSwitch("launch")) {
            int cmsid = 0;
            try
            {
                cmsid = std::stoi(command_line->GetSwitchValue("launch").c_str());
            }
            catch(...)
            {
                LOG(ERROR) << "Invalid launch parameter value, defaulting to CMS ID 0!";
            }
            LOG(INFO) << "CMSID: " << cmsid;
            HANDLE gameProcHandle = NULL;
            if (Launch(cmsid, gameProcHandle))
            {
                HANDLE newWaitHandle = NULL;
                RegisterWaitForSingleObject(&newWaitHandle, gameProcHandle, reinterpret_cast<WAITORTIMERCALLBACK>(&GameProcessClosedCallback), reinterpret_cast<void*>(gameProcHandle), INFINITE, WT_EXECUTEONLYONCE);
                g_gameClosedEvent = CreateEvent(NULL, TRUE, FALSE, L"Global\\GFNRuntimeSampleLauncherGameClosed");
            }            
        }

        // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
        CefRunMessageLoop();

        CefShutdown();

        return 0;
    }

#include <map>
    std::map<int, std::wstring> cmsidMap = { {16032111, L"C:\\Program Files (x86)\\NVIDIA_Grid_Bundle\\nvidia_vxgi_apollo\\Lunar\\Binaries\\Win32\\Lunar-Win32-Shipping.exe"} };

    bool Launch(int cmsid, HANDLE & gameProcHandle)
    {
        gameProcHandle = NULL;
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
                return false;
            }

            gameProcHandle = procInfo.hProcess;
            CloseHandle(procInfo.hThread);
        }
        return true;
    }

    void CALLBACK GameProcessClosedCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
    {
        LOG(INFO) << "Child game process has closed, exiting... ";
        HANDLE gameProcHandle = reinterpret_cast<HANDLE>(lpParameter);
        CloseHandle(gameProcHandle);
        SetEvent(g_gameClosedEvent);
    }

}  // namespace shared
