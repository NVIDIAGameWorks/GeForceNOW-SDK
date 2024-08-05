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

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <cube.h>

FILE *log_fp = NULL;

void startLog()
{
    // redirect log to file
    log_fp = freopen("CubeSample.log", "a", stdout);
    printf("-------------------------------\n");
}

void closeLog()
{
    if (log_fp)
        fclose(log_fp);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    startLog();
    int ret = WinMain_internal(hInstance, hPrevInstance, pCmdLine, nCmdShow);
    closeLog();
    return ret;
}
#else
int main(int argc, char **argv)
{
    startLog();
    int ret = main_internal(argc, argv);
    closeLog();
    return ret;
}
#endif

