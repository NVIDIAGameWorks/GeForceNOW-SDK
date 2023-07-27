// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "shared/resource_util.h"

#include "gfn_sdk_demo/resources/win/resource.h"

namespace shared {

int GetResourceId(const std::string& resource_path) {
  if (resource_path == "gfn_sdk.html")
    return IDS_MESSAGE_ROUTER_HTML;
  if (resource_path == "apollo11.jpg")
    return IDS_MESSAGE_ROUTER_IMG;
  return 0;
}

}  // namespace shared
