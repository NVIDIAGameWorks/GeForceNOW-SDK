// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2024 NVIDIA Corporation. All rights reserved.

#include <stdio.h>
#include <math.h>
#include <string.h>

// Sample will use the Helper Wrapper sources to auto-manage SDK library handling
#include "GfnRuntimeSdk_Wrapper.h"

// Max url on most browsers is 2048, so max encoded string + null character is 2733
#define MAX_URL_LEN_WITH_NULL 2048
#define MAX_BASE64_URL_WITH_NULL 2733

#ifdef _WIN32
#   include <conio.h>
#   include <windows.h>
#elif __linux__
#   include <termios.h>
#   include <unistd.h>
// GFN on linux doesn't open up a cmd prompt by default, so we have to create a display window of our own.
#   include <X11/Xlib.h>
#   include <X11/keysym.h>
#endif

#if __linux__
typedef struct x11_context
{
    Display* display;
    Window window;
    GC gc;
} x11_context;

x11_context g_context;
unsigned int g_consoleLine;
#endif

// Keyboard input helper function
static char getKeyPress()
{
#ifdef _WIN32
    return _getch();
#elif __linux__
    XEvent event;
    while (true)
    {
        XNextEvent(g_context.display, &event);
        if (event.type == KeyPress) {
            KeySym key = XLookupKeysym(&event.xkey, 0);
            return key;
        }
    }
#else
    return ' ';
#endif
}

// Loop exit helper function that waits for spacebar press
static char waitForInput()
{
    char c;
    do
    {
        c = getKeyPress();
    } while (c != ' ' && (c < '1' || c > '7'));
    return c;
}

void ShowText(const char* text)
{
#ifdef _WIN32
    printf(text);
    printf("\n");
#elif __linux__
    XDrawString(g_context.display, g_context.window, g_context.gc, 10, 20 * (g_consoleLine++), text, strlen(text));
#endif
}

#if __linux__
void SetupDisplay()
{
    g_context.display = XOpenDisplay(NULL);
    if (!g_context.display) {
        printf("Error: Unable to open X display.\n");
        return;
    }

    int screen = DefaultScreen(g_context.display);
    Window rootWindow = RootWindow(g_context.display, screen);

    // Create a new window
    g_context.window = XCreateSimpleWindow(g_context.display, rootWindow, 0, 0, 800, 400, 0, 0, 0);
    XSelectInput(g_context.display, g_context.window, ExposureMask | KeyPressMask);
    XMapWindow(g_context.display, g_context.window);

    // Set the color attributes
    XColor grayColor;
    Colormap colormap = DefaultColormap(g_context.display, screen);
    XParseColor(g_context.display, colormap, "gray", &grayColor);
    XAllocColor(g_context.display, colormap, &grayColor);

    XColor blackColor;
    XParseColor(g_context.display, colormap, "black", &blackColor);
    XAllocColor(g_context.display, colormap, &blackColor);

    // Create a graphics context
    g_context.gc = XCreateGC(g_context.display, g_context.window, 0, NULL);
    XSetForeground(g_context.display, g_context.gc, grayColor.pixel);
    XSetBackground(g_context.display, g_context.gc, blackColor.pixel);
}

void ClearDisplay()
{
    XClearWindow(g_context.display, g_context.window);
    g_consoleLine = 0;
}

void Draw()
{
    XFlush(g_context.display);
}

void CleanupDisplay()
{
    // Clean up
    XFreeGC(g_context.display, g_context.gc);
    XDestroyWindow(g_context.display, g_context.window);
    XCloseDisplay(g_context.display);
}
#endif

void ShowStatus(const char* text, bool shouldWait)
{
    ShowText(text);
    if (shouldWait)
    {
        ShowText("Press space to continue");
    }
#if __linux__
    Draw();
#endif
    if (shouldWait)
    {
        waitForInput();
    }
}

void ShowError(const char* text, GfnError error, bool shouldWait)
{
    char buffer[256];
    sprintf(buffer, text, error, GfnErrorToString(error));
    ShowStatus(buffer, shouldWait);
}

void DoSleep(int secs)
{
#ifdef _WIN32
    Sleep(secs * 1000);
#elif __linux__
    sleep(secs);
#endif
}

// Example application initialization method with a call to initialize the Geforce NOW Runtime SDK.
GfnError SDKInitialize()
{
    // Using gfnDefaultLanguage to tell the SDK to use the default system language for any
    // UI it might show, which won't be the case in this sample.
    printf("\n\nInitializing GFN SDK...\n");
    GfnError result = GfnInitializeSdk(gfnDefaultLanguage);
    if (GFNSDK_FAILED(result))
    {
        // Initialization errors generally indicate a flawed environment. Check error code for details.
        // See GfnError in GfnSdk.h for error codes.
        ShowError("Error initializing the SDK: %d(%s)", result, false);
    }
    else
    {
        // Initalization success, which the return value hints at the environment detected
        switch (result)
        {
        case gfnInitSuccessClientOnly:
            // Detected this looks like a client system (outside GFN cloud).
            // Only client-related API methods should be called.
            printf("SDK initalization success, detected client environment.\n");
            break;
        case gfnInitSuccessCloudOnly:
            // Detected this looks like a GFN cloud system (GFN server).
            // Only GFN cloud-related API methods should be called.
            printf("SDK initalization success, detected cloud environment.\n");
            break;
        default:
            // Generic success case
            printf("SDK initalization success.\n");
            break;
        }
    }

    return result;
}

// Example application shutdown method with a call to shut down the Geforce NOW Runtime SDK
void SDKShutdown()
{
    printf("\n\nShutting down GFN SDK...\n");

    // Shut down the Geforce NOW Runtime SDK. Note that it's safe to call
    // gfnShutdownRuntimeSdk even if the SDK was not initialized.
    // If a failure is returned, its worth noting, but nothing can be done about it.
    GfnError result = GfnShutdownSdk();
    printf("Shutdown call for SDK result: %d\n", result);
}

// Example method to call the basic *insecure* Cloud Check APIs
bool BasicCloudCheck()
{
    bool bIsCloudEnvironment = false;
    GfnError result = gfnSuccess;

    printf("\n\nPerforming Basic Secure Cloud Check via GfnCloudCheck without Challenge and Response data...\n");
    result = GfnCloudCheck(NULL, NULL, &bIsCloudEnvironment);

    if (GFNSDK_FAILED(result))
    {
        // Failure case, do not rely on bIsCloudEnvironment result
        ShowError("Cloud Check API call returned error: %d(%s)", result, false);
        return false;
    }
    else
    {
        printf("Application %s executing in Geforce NOW Cloud environment.\n", (bIsCloudEnvironment) ? "is" : "is not");
    }
    return bIsCloudEnvironment;
}

// Example method to call the GfnOpenURLOnClient API method. This will send a request to the GFN
// client to open the provided URL on their local web browser.
void OpenClientBrowser(const char* url)
{
    if (url == NULL) {
        printf("Something went wrong. A NULL URL was passed in!\n");
        // Continue for the sake of experiencing a similar error from the SDK.
    }

    // Call the SDK function
    GfnError result = GfnOpenURLOnClient(url);
    if (GFNSDK_FAILED(result))
    {
#if __linux__
        ShowError("Unable to send request to client. %d(%s).", result, true);
#else
        ShowError("Unable to send request to client. %d(%s).", result, false);
#endif
    }
    else
    {
#if __linux__
        ShowStatus("The client has been notified that this software wishes to open a URL locally.", true);
#else
        ShowStatus("The client has been notified that this software wishes to open a URL locally.", false);
#endif
    }

#if __linux__
    Draw();
#endif
}

// There is a limit of 5 calls per second, 25 calls per minute. This function tests both.
void DoStressTest(const char* url)
{
    if (url == NULL) {
        printf("Something went wrong. A NULL URL was passed in!\n");
        return;
    }

    GfnError result;
    for (int i = 0; i < 5; i++)
    {
        result = GfnOpenURLOnClient(url);
        if (result != gfnSuccess)
        {
            char buffer[256];
            sprintf(buffer, "Unexpected error %%d(%%s) received when sending request %d within one second", i);
            ShowError(buffer, result, false);
        }

    }
    result = GfnOpenURLOnClient(url);
    if (result == gfnThrottled) {
        ShowText("The 6th message in a second was correctly throttled.");
    }
    else
    {
        char buffer[256];
        sprintf(buffer, "Error, the 6th message within a second should have been throttled. %d(%%s) was returned instead.", result);
        ShowError(buffer, result, false);
    }

    // We've already sent 5, and the 6th didn't count, so now we need to do 21 more to trigger the next threshold limit.
    for (int i = 0; i < 4; i++)
    {
        DoSleep(2);
        for (int j = 0; j < 5; j++)
        {
            result = GfnOpenURLOnClient(url);
            if (result != gfnSuccess)
            {
                char buffer[256];
                sprintf(buffer, "Unexpected error %%d(%%s) received when sending request %d within one minute", 5 + (5 * i) + j);
                ShowError(buffer, result, false);
            }
        }
    }
    result = GfnOpenURLOnClient(url);
    if (result == gfnThrottled) {
        ShowText("The 26th message in a minute was correctly throttled.");
    }
    else
    {
        char buffer[256];
        sprintf(buffer, "Error, the 26th message within a minute should have been throttled. %d(%%s) was returned instead.", result);
        ShowError(buffer, result, false);
    }
}

void DisplayPrompt()
{
    ShowText(" ");
    ShowText("Press 1 to open https://www.nvidia.com/en-us/geforce-now/");
    ShowText("Press 2 to open https://developer.geforcenow.com/learn/guides/offerings-sdk");
    ShowText("Press 3 to attempt to open an invalid URL");
    ShowText("Press 4 to attempt to open a URL that's too long");
    ShowText("Press 5 to attempt to send a NULL URL");
    ShowText("Press 6 to attempt to send a URL with invalid characters");
    ShowText("Press 7 to send TOO MANY URLs in a short timeframe");
    ShowText(" ");
    ShowText("Press space bar to exit...");

#if __linux__
    Draw();
#endif
}

// Example application main
int main(int argc, char* argv[])
{
#if __linux__
    SetupDisplay();
    ClearDisplay();
#endif
    // First step: Initialize the Geforce NOW Runtime SDK before any other SDK method calls.
    GfnError result;
    result = SDKInitialize();
    if (GFNSDK_FAILED(result))
    {
        // Initialization failure, exit now, no need to call GfnShutdownSdk()
        // Calling any SDK methods in this state will return indeterministic results that should not be trusted.
        ShowError("SDK Initialize failed with %d(%s)! This does not appear to be a GFN Game seat. Exiting...", result, true);
        return -1;
    }

    // GfnOpenURLOnClient can only be called in the cloud environment, and so that must be checked first.
    if (!BasicCloudCheck())
    {
        ShowStatus("Not running in Geforce NOW Cloud environment, exiting early without calling Cloud APIs.", true);
    }
    else
    {
        char c;
        do {
#if __linux__
            ClearDisplay();
#endif
            BasicCloudCheck();
            DisplayPrompt();
            c = waitForInput();
            char str_input[2];
            sprintf(str_input, "%c", c);
            ShowText(str_input);
            ShowText(" ");
#if __linux__
            Draw();
#endif
            switch (c)
            {
            case '1':
                OpenClientBrowser("https://www.nvidia.com/en-us/geforce-now/");
                break;
            case '2':
                OpenClientBrowser("https://developer.geforcenow.com/learn/guides/offerings-sdk");
                break;
            case '3':
                OpenClientBrowser("httpz://www.nvidia.com/en-us/geforce-now/");
                break;
            case '4':
            {
                char dataThatsWayTooBig[2049] = "https://www.nvidia.com/123456789"; // \0 is at index 32
                for (size_t i = 2; i < (2048 / 16); i++)
                {
                    memcpy(&(dataThatsWayTooBig[i * 16]), "0123456789ABCDEF", 16);
                }
                dataThatsWayTooBig[2048] = '\0';
                OpenClientBrowser(dataThatsWayTooBig);
            }
            break;
            case '5':
                OpenClientBrowser(NULL);
                break;
            case '6':
                OpenClientBrowser("https://www.nvidia.com/\">< ^`{|");
                break;
            case '7':
                DoStressTest("https://www.nvidia.com/en-us/geforce-now/");
                break;
            default:
                break;
            }
        } while (c != ' ');
    }

    // GFN SDK Shutdown. It's safe to call ShutdownSDK even if the SDK was not initialized.
    SDKShutdown();

#if __linux__
    CleanupDisplay();
#endif

    return 0;
}
