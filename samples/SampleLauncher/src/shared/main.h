// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_EXAMPLES_SHARED_MAIN_H_
#define CEF_EXAMPLES_SHARED_MAIN_H_

#include "include/base/cef_build.h"
#include <string>

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace shared {

// The port used for the browser window. Global to provide simple access to query handler.
extern std::string g_activePort;

// Entry point function shared by executable targets.
#if defined(OS_WIN)
int APIENTRY wWinMain(HINSTANCE hInstance);
#else
int main(int argc, char* argv[]);
#endif

}  // namespace shared

#endif  // CEF_EXAMPLES_SHARED_MAIN_H_
