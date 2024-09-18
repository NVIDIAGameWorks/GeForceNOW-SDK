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
 
#ifdef  _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #define MOUSEEVENTF_FROMTOUCH (0xFF515700)
#else
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>
    #include <X11/Xutil.h>
    #define max(x, y) (((x) > (y)) ? (x) : (y))
    #define min(x, y) (((x) < (y)) ? (x) : (y))
#endif
#include "GfnSdkInterface.h"

void ackSpinChange(struct SpinState *spin_state)
{
    char ackMessage[100];
    size_t outLength = snprintf(ackMessage, 100, "spin %0.2f", spin_state->pause ? 0 : spin_state->spin_angle);
    printf("updated %s\n", ackMessage);
    if (gfnSuccess == GfnSendMessage(ackMessage, (unsigned int)outLength))
    {
        printf("Sent message to the client: %s\n", ackMessage);
    }
    else
    {
        printf("Failed to send a communication message to the client.\n");
    }
}

void gfnsdk_decreaseSpin(struct SpinState *spin_state)
{
    if (!spin_state->pause)
    {
        if (spin_state->spin_angle > 0)
        {
            spin_state->spin_angle = max(spin_state->spin_angle - spin_state->spin_increment, 0);
        }
        else
        {
            spin_state->spin_angle = min(spin_state->spin_angle + spin_state->spin_increment, 0);
        }
    }
    ackSpinChange(spin_state);
}

void gfnsdk_increaseSpin(struct SpinState *spin_state)
{
    if (!spin_state->pause)
    {
        if (spin_state->spin_angle > 0)
        {
            spin_state->spin_angle += spin_state->spin_increment;
        }
        else
        {
            spin_state->spin_angle -= spin_state->spin_increment;
        }
    }
    ackSpinChange(spin_state);
}

void gfnsdk_togglePauseState(struct SpinState *spin_state)
{
    static float original_spin_angle = 0;
    if (spin_state->pause)
    {
        spin_state->spin_angle = original_spin_angle;
        original_spin_angle = 0;
    }
    else
    {
        original_spin_angle = spin_state->spin_angle;
        spin_state->spin_angle = 0;
    }
    spin_state->pause = !spin_state->pause;
    ackSpinChange(spin_state);
}

void gfnsdk_reverseSpin(struct SpinState *spin_state)
{
    if (!spin_state->pause)
    {
        spin_state->spin_angle = -1 * spin_state->spin_angle;
    }
    ackSpinChange(spin_state);
}

void gfnsdk_handleButtonClick(UINT uMsg, int xPos, int yPos, struct SpinState *spin_state, int width)
{
#ifdef _WIN32
    if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH)
    {
        // Do not process duplicate Mouse event during touch input
        return;
    }
#endif

    switch (uMsg)
    {
#ifdef _WIN32
    case WM_RBUTTONUP:
#else
    case Button3: // Right button
#endif
        gfnsdk_reverseSpin(spin_state);
        break;

#ifdef _WIN32
    case WM_LBUTTONUP:
#else
    case Button1: // Left button
#endif
        if (xPos < width / 2)
        {
            gfnsdk_decreaseSpin(spin_state);
        }
        else
        {
            gfnsdk_increaseSpin(spin_state);
        }
        break;

    default:
        printf("ERROR: unhandled button click %X\n", uMsg);
        return;
    }
    ackSpinChange(spin_state);
}

#ifdef _WIN32
void gfnsdk_handleTouch(HWND hWnd, UINT uMsg, UINT cInputs, HTOUCHINPUT hTouchInput, struct SpinState *spin_state, int width)
{
    PTOUCHINPUT pInputs = (PTOUCHINPUT)malloc(sizeof(TOUCHINPUT) * cInputs);
    if (pInputs)
    {
        if (GetTouchInputInfo(hTouchInput, cInputs, pInputs, sizeof(TOUCHINPUT)))
        {
            // Consult only first touch
            TOUCHINPUT touchInput = pInputs[0];
            if ((touchInput.dwFlags & TOUCHEVENTF_DOWN) == TOUCHEVENTF_DOWN)
            {
                POINT point;
                point.x = TOUCH_COORD_TO_PIXEL(touchInput.x);
                point.y = TOUCH_COORD_TO_PIXEL(touchInput.y);
                ScreenToClient(hWnd, &point);
                if (point.x < width / 2)
                {
                    gfnsdk_decreaseSpin(spin_state);
                }
                else
                {
                    gfnsdk_increaseSpin(spin_state);
                }
            }
            CloseTouchInputHandle(hTouchInput);
        }
        free(pInputs);
    }
}
#endif

GfnApplicationCallbackResult GFN_CALLBACK MessageCallback(const GfnString* pMessage, void* pContext)
{
    printf("Message from client: '%s' length=%u\n", pMessage->pchString, pMessage->length);

    if (pContext == NULL)
    {
        printf("ERROR: Message Callback has no context");
        return crCallbackFailure;
    }

    struct SpinState *spin_state = (struct SpinState *)pContext; 

    char ackMessage[100];
    size_t outLength = 0;
    bool spinUpdated = true;
    if (strncmp(pMessage->pchString, "togglePause", pMessage->length) == 0)
    {
        gfnsdk_togglePauseState(spin_state);
    }
    else if (strncmp(pMessage->pchString, "exit", pMessage->length) == 0)
    {
        spin_state->quit = true;
        outLength = snprintf(ackMessage, 100, "exiting");
        spinUpdated = false;
    }
    else if (strncmp(pMessage->pchString, "spin+", pMessage->length) == 0)
    {
        gfnsdk_increaseSpin(spin_state);
    }
    else if (strncmp(pMessage->pchString, "spin-", pMessage->length) == 0)
    {
        gfnsdk_decreaseSpin(spin_state);
    }
    else if (strncmp(pMessage->pchString, "respin", pMessage->length) == 0)
    {
        gfnsdk_reverseSpin(spin_state);
    }
    else
    {
        outLength = snprintf(ackMessage, 100, "unrecognised message");
        spinUpdated = false;
    }
    ackSpinChange(spin_state);

    return crCallbackSuccess;
}

void gfnsdk_init(struct SpinState *spin_state) {
    GfnRuntimeError err = GfnInitializeSdk(gfnDefaultLanguage);
    if (GFNSDK_FAILED(err))
    {
        // Initialization errors generally indicate a flawed environment. Check error code for details.
        // See GfnError in GfnSdk.h for error codes.
        printf("Error initializing the sdk: %d\n", err);
        return;
    }

    bool bIsCloudEnvironment = false;
    GfnIsRunningInCloud(&bIsCloudEnvironment);
    if (bIsCloudEnvironment)
    {
        // Register any implemented callbacks capable of serving requests from the SDK.
        err = GfnRegisterMessageCallback(MessageCallback, spin_state);
        if (err != gfnSuccess)
        {
            printf("Error registering MessageCallback: %d\n", err);
        }
    }
    else
    {
        printf("GFNSDK: Not running in cloud environment!\n");
        return;
    }
}

void gfnsdk_shutdown() {
    GfnRuntimeError err = GfnShutdownSdk();
    if (GFNSDK_FAILED(err))
    {
        printf("Error shutting down the sdk: %d\n", err);
    }
}
