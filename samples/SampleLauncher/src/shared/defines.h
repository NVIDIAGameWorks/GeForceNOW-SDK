// Copyright (c) 2020, NVIDIA CORPORATION.  All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#pragma once

namespace shared {
    // This array defines the possible ports CEF will use for the browser window. One of these ports
    // must be used for successful redirection of NVIDIA IDM/IDP activities via the "NVIDIA Login"
    // button for this application. Use of any other ports in this sample application will fail 
    // NVIDIA IDM/IDP activities. Your NVIDA IDM/IDP-integrated application may have other ports
    // defined/assigned.
   const char* const kTCPPorts[] = { "15875", "8642", "19636", "30674", "10929", 0 };
   
   // This is the name of the page loaded into the CEF browser for this sample application.
   const char kResourceHTML[] = "gfn_sdk.html";
}
