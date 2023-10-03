//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Replacement for min and max macros from Windows headers
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include <type_traits>

// Utility functions to replace min/max macros from minwindef.h Windows header.
// These macros are disabled with NOMINMAX preprocessor definition.
// In general, these macros should be replaced by std::min/std::max.
// However, std:: functions only work if both arguments have the same type.
// Use automin/automax as a temporary (TM) solution if T1 != T2 and result type is unclear/nonintuitive.
// Compiler will determine result type automatically like with min/max macros.

// WARNING: These functions do NOT attempt to solve inconsistencies in signed/unsigned comparisons.
// To solve those, consider explicitly casting values and/or using std::cmp_* in C++20 and up.

namespace MaxSDK
{
	// Returns a copy of minimum value between a and b.
	template <typename T1, typename T2>
	[[nodiscard]] inline constexpr auto min(const T1& a, const T2& b) noexcept
	{
		static_assert(!std::is_same<T1, T2>::value, "T1 == T2. Use std::min instead");
		static_assert(std::is_scalar<T1>::value && std::is_scalar<T2>::value, "T1 and T2 must be scalar types");
		return a < b ? a : b;
	}

	// Returns a copy of maximum value between a and b.
	template <typename T1, typename T2>
	[[nodiscard]] inline constexpr auto max(const T1& a, const T2& b) noexcept
	{
		static_assert(!std::is_same<T1, T2>::value, "T1 == T2. Use std::max instead");
		static_assert(std::is_scalar<T1>::value && std::is_scalar<T2>::value, "T1 and T2 must be scalar types");
		return b < a ? a : b;
	}
}
