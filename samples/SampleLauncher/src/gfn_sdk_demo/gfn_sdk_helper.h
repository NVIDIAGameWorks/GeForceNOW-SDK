// Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef GFN_SDK_HELPER_H_
#define GFN_SDK_HELPER_H_

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

bool GfnSdkHelper(CefRefPtr<CefBrowser> browser,
                  CefRefPtr<CefFrame> frame,
                  int64 query_id,
                  const CefString& request,
                  bool persistent,
                  CefRefPtr<CefMessageRouterBrowserSide::Callback> callback);

#endif