// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "shared/main.h"

#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "include/base/cef_logging.h"

#include "shared/app_factory.h"
#include "shared/client_manager.h"
#include "shared/main_util.h"

namespace shared {

  std::string g_activePort;

namespace {

int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X error received: "
               << "type " << event->type << ", "
               << "serial " << event->serial << ", "
               << "error_code " << static_cast<int>(event->error_code) << ", "
               << "request_code " << static_cast<int>(event->request_code)
               << ", "
               << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}

static int mkdirs(std::string path, mode_t mode)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        return 0;  // Directory exists
    }

    size_t lastSlashPos = path.find_last_of('/');
    if (std::string::npos != lastSlashPos)
    {
        // Recursively create parent directory
        int ret = mkdirs(path.substr(0, lastSlashPos), mode);
        if (ret != 0)
        {
            return ret;  // Return error upwards
        }
    }

    // Create directory
    if (mkdir(path.c_str(), mode) != 0) {
        return -1;  // Error
    }

    return 0;
}

// Recursively creates missing directories
static bool MakeDirs(std::string path)
{
    int status = mkdirs(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status == -1)
    {
        std::stringstream ss;
        ss << "Failed to create directory: " << path;
        return false;
    }

    return true;
}

}  // namespace

// Entry point function for all processes.
int main(int argc, char* argv[]) {
  // Provide CEF with command-line arguments.
  CefMainArgs main_args(argc, argv);

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
  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Install xlib error handlers so that the application won't be terminated
  // on non-fatal errors.
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);

  // Create the singleton manager instance.
  ClientManager manager;

  // Specify CEF global settings here.
  CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
        settings.no_sandbox = true;
#endif
  settings.remote_debugging_port = 8080;

  std::stringstream ss;
  ss << getenv("HOME") << "/.nvidia/GfnRuntimeSdk";

  if (!MakeDirs(ss.str()))
  {
    fprintf(stderr, "Could not create logging directory");
  }
  else
  {
    ss << "//GfnSdkSampleLauncher.log";
    CefString(&settings.log_file).FromString(ss.str());
  }

  // Initialize CEF for the browser process. The first browser instance will be
  // created in CefBrowserProcessHandler::OnContextInitialized() after CEF has
  // been initialized.
  CefInitialize(main_args, settings, app, nullptr);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}

}  // namespace shared
