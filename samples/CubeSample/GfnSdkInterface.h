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

#ifndef GFN_SDK_CUBE_APP_INTERFACE_H
#define GFN_SDK_CUBE_APP_INTERFACE_H

#include "GfnRuntimeSdk_CAPI.h"
#include "GfnRuntimeSdk_Wrapper.h"
#include "cube.h"

void gfnsdk_decreaseSpin(struct SpinState *spin_state);
void gfnsdk_increaseSpin(struct SpinState *spin_state);
void gfnsdk_reverseSpin(struct SpinState *spin_state);
void gfnsdk_togglePauseState(struct SpinState *spin_state);
void gfnsdk_init(struct SpinState *spin_state);
void gfnsdk_shutdown();
void gfnsdk_handleButtonClick(UINT uMsg, int xPos, int yPos, struct SpinState *spin_state, int width);

#ifdef _WIN32
void gfnsdk_handleTouch(HWND hWnd, UINT cInputs, HTOUCHINPUT hTouchInput, struct SpinState *spin_state, int width);
#endif

#endif // GFN_SDK_CUBE_APP_INTERFACE_H
