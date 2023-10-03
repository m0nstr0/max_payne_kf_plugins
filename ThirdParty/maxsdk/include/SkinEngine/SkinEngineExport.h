//**************************************************************************/
// Copyright (c) 1998-2021 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once
#ifdef BUILD_SKINENGINE
#define SEEXPORT __declspec( dllexport )
#else
#define SEEXPORT __declspec( dllimport )
#endif
