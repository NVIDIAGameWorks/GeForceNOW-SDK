// Not a contribution
// Changes made by NVIDIA CORPORATION & AFFILIATES enabling GFN SDK Cube Sample or otherwise documented as
// NVIDIA-proprietary are not a contribution and subject to the following terms and conditions:
/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-NvidiaProprietary
 *
 * NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
 * property and proprietary rights in and to this material, related
 * documentation and any modifications thereto. Any use, reproduction,
 * disclosure or distribution of this material and related documentation
 * without an express license agreement from NVIDIA CORPORATION or
 * its affiliates is strictly prohibited.
 */

#ifndef GFN_SDK_CUBE_APP_CUBE_H
#define GFN_SDK_CUBE_APP_CUBE_H

#include <stdbool.h>

#ifndef  _WIN32
#define UINT uint32_t
#endif

struct SpinState
{
    float spin_angle;
    float spin_increment;
    bool pause;
    bool quit;
};

#if _WIN32
int WINAPI WinMain_internal(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow);
#else
int main_internal(int argc, char **argv);
#endif

#endif // GFN_SDK_CUBE_APP_CUBE_H
