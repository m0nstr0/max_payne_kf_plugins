//
// Copyright [2010] Autodesk, Inc.  All rights reserved. 
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//
//
#pragma once

#include "../3dsmaxconfig.h"

namespace MaxSDK
{
	namespace Util
	{
		//! \deprecated This has been deprecated in 3ds Max 2024, please use builtin static_assert(b, "msg") instead
		template <bool b> struct MAX_DEPRECATED StaticAssert
		{
			static_assert(b, "MaxSDK::Util::StaticAssert failed");
		};
	}
}
