// This header file contains methods used in CloudCheck Utils methods.
// Application devs can replace this functionality with their own memory allocation and logger implementation.

#ifndef __GFN_CLOUD_CHECK_APP_ADAPTER_H__
#define __GFN_CLOUD_CHECK_APP_ADAPTER_H__

#include <stdio.h>
#include <stdlib.h>

// Define for debugging.
#define ENABLE_CLOUDCHECK_LOGGING

#ifdef ENABLE_CLOUDCHECK_LOGGING
#define GFN_CC_LOG printf
#else
#define CC_LOG(...)
#endif

#define GFN_CC_MALLOC(size) malloc(size)
#define GFN_CC_FREE(ptr) free(ptr)

#endif //__GFN_CLOUD_CHECK_APP_ADAPTER_H__
