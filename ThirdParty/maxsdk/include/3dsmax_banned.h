//**************************************************************************/
// Copyright (c) 2018 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Common banned.h wrapper header for 3ds Max projects
//
// PURPOSE:
// - Pre-include system or 3rd-party library headers before banned.h,
//   to suppress C4995 function deprecation errors in external code.
// - Suppress side-effect errors (e.g. warnings, macro conflicts) that may arise
//   from force-including (/FI) this banned.h wrapper header on the project.
//***************************************************************************/

#pragma once

//
// PRE-INCLUDES
//

#ifndef STRICT
#define STRICT  // Strict type checking
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Faster builds with smaller headers
#endif
#ifndef NOMINMAX
#define NOMINMAX // Macros min(a, b) and max(a, b)
#endif

#if defined(_AFXDLL) && defined(__cplusplus)
// If MFC project, do not include <windows.h> directly; instead, include <afxwin.h> which internally includes <windows.h>.
// Notes:
// - MFC requires C++ compilation. Exclude this check for C files.
// - Checking for defined _AFXDLL only works if project is configured to "use MFC in a Shared DLL".
//   If the project is configured to "use MFC in a Shared Library", a different check/solution would be needed.
#ifndef __AFXWIN_H__
#error Include <afxwin.h> and other MFC-related includes & defines, before including 3dsmax_banned.h, in a project-specific banned.h wrapper header.
#endif
#else
#include <windows.h>
#endif // _AFXDLL

#include <tchar.h>

// Conditionally pre-include a subset of Qt headers
// (only those that contain banned.h C4995 errors, which so far were only found in QtCore)
#ifdef QT_CORE_LIB
// Pre-include <xlocnum>, which is nested under <QtCore/qstring.h> --> <string> --> ... --> <ostream> --> ... -->  <xlocnum>.
// <xlocnum> takes care of several nested C4995 errors in <cstdio>, <stdio.h>, <cstring>, <cwchar>, <iosfwd>, <xlocale>, etc.
// It is explicitly pre-included here for info/clarity/example, although just pre-including <QtCore/qstring.h>
// or <QtCore/qstringbuilder.h> as done below would suffice.
#include <xlocnum>
// Also pre-include Qt headers that directly contain banned.h C4995 errors, apart from those also found in nested standard headers.
// Specifying the Qt .h headers, e.g. <QtCore/qstringbuilder.h>, instead of the non-.h headers, e.g. <QtCore/QStringBuilder>,  to skip the extra level of indirection.
#pragma warning(push)
#pragma warning(disable: 4127 4251 4275)
#include <QtCore/QtCore>
#pragma warning (pop)
#endif // QT_CORE_LIB

#ifdef QT_GUI_LIB
#pragma warning(push)
#pragma warning(disable: 4244 4251)
#include <QtGui/qmatrix.h>
#include <QtGui/qtransform.h>
#include <QtGui/qtextobject.h>
#pragma warning (pop)
#endif // QT_GUI_LIB

//
// BANNED FUNCTIONS
//

// Define '_SDL_BANNED_RECOMMENDED' to detect additional API deprecations
//#define _SDL_BANNED_RECOMMENDED

// Include Microsoft's Security Development Lifecycle (SDL) banned.h header file
// Reference: https://blogs.microsoft.com/microsoftsecure/2012/08/30/microsofts-free-security-tools-banned-h/
#include "banned.h"


//
// Additional custom deprecations (deprecated by Microsoft but not necessarily flagged as "unsafe")
//

// Starting with the Visual Studio 2010 CRT library, _CrtIsValidPointer() and _CrtIsValidHeapPointer() are no longer useful.
// References: _CrtIsValidPointer() - https://msdn.microsoft.com/en-us/library/0w1ekd5e.aspx
//             _CrtIsValidHeapPointer() - https://msdn.microsoft.com/en-us/library/ys6cfhhh.aspx
#pragma deprecated (_CrtIsValidPointer, _CrtIsValidHeapPointer)
