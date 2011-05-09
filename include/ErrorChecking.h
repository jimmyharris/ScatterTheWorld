/*
 *  ErrorChecking.h
 *  ScatterTheWorld
 *
 *  Created by James Harris on 5/9/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

//----------------------------
// Macros
//----------------------------
#define CHECK_RC(rc, what)                               \
if (rc != XN_STATUS_OK)                                  \
{                                                        \
printf("%s failed: %s\n", what, xnGetStatusString(rc));\
exit(rc);                                             \
}
#define CHECK_ERRORS(rc, errors, what)  \
if (rc == XN_STATUS_NO_NODE_PRESENT)	\
{										\
XnChar strError[1024];              \
errors.ToString(strError, 1024);	\
printf("%s\n", strError);			\
exit(rc);						    \
}
