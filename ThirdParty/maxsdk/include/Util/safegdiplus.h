//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Include gdiplus.h without introducing min and max macros
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

// gdiplustypes.h relies on min and max macros from windows.h.
// These macros are explicitly disabled with NOMINMAX.
// Provide min and max macros just for that header.
#include <minmax.h>
#include <gdiplus.h>

#undef min
#undef max
