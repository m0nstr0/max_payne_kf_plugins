//**************************************************************************/
// Copyright 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <cstdint> 

class AColor;
class Color;
struct Color24;
struct Color48;
struct Color64;
struct BMM_Color_24;
struct BMM_Color_32;
struct BMM_Color_48;
struct BMM_Color_64;
struct BMM_Color_fl;
struct BMM_Color_fl3;

namespace MaxSDK::ColorManagement {

/** Helper template class for providing memory layout information for various
 * pixel data types. IColorPipeline and ColorConverter classes use this to
 * process pixels properly. Specializations of this template class for common
 * 3ds %Max SDK pixel structures are provided by the system, but users can
 * specialize the methods to add support for custom types. All member functions
 * are 'static constexpr' thus all the type information is compile-time
 * evaluated.
 *
 * Specializations for the following types are provided by the system:  
 * Color, AColor, Color24, Color48, Color64, BMM_Color_24, BMM_Color_32,
 * BMM_Color_32, BMM_Color_48, BMM_Color_64 and BMM_Color_fl
*/
class ImageLayoutInfo
{
public:

	/** Data type and bit-depth of each color channel of the pixel*/
	enum class ChannelType : uint8_t
	{
		type_unknown = 0,
		type_uint8,
		type_uint16,
		type_float,
		type_half,
	};

	/** Order of the color channels of the pixels in the memory.  
	 * \note    this value should be in agreement with the value returned by
	 *          NumChan() function.
	 */
	enum class ChannelOrder : uint8_t
	{
		order_unknown = 0,
		order_rgb,
		order_bgr,
		order_rgba,
		order_bgra,
		order_abgr,
	};

	/** Deleted default constructor. This class is a purely static class which
	 * needs to be evaluated in the compile-time.   
	 */
	ImageLayoutInfo() = delete;

	/** Returns the number of color channels, 3 for RGB, 4 for RGBA. 
	 *
	 * \note    this value should be in agreement with the value returned by the
	 *          ChanOrder() function.
	 */
	template <class T>
	static constexpr int NumChan();

	/** Returns the bit-depth and data type of each channel. See
	 * ImageLayoutInfo::ChannelType enum for possible values. */
	template <class T>
	static constexpr ChannelType ChanType();

	/** Returns the channel order in the memory layout. See
	 * ImageLayoutInfo::ChannelOrder enum for possible values. 
	 *
	 * \note    This value should be in agreement with the value returned by the
	 *          NumChan() function. 
	 */
	template <class T>
	static constexpr ChannelOrder ChanOrder();

	/** Returns the number of bytes each pixel occupies in the memory, in other
	 * words pixel-to-pixel distance in bytes. The default implementation  
	 * \code  
	 * return sizeof(T);   
	 * \endcode  
	 * should work for most cases.
	*/
	template <class T>
	static constexpr ptrdiff_t PixelStrideBytes()
	{
		return sizeof(T);
	};
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// specializations for some 3ds %Max built-in pixel formats
// Color specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<Color>()
{
	return 3;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<Color>()
{
	return ChannelType::type_float;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<Color>()
{
	return ChannelOrder::order_rgb;
}

// AColor specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<AColor>()
{
	return 4;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<AColor>()
{
	return ChannelType::type_float;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<AColor>()
{
	return ChannelOrder::order_rgba;
}

// Color24 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<Color24>()
{
	return 3;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<Color24>()
{
	return ChannelType::type_uint8;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<Color24>()
{
	return ChannelOrder::order_rgb;
}

// Color48 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<Color48>()
{
	return 3;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<Color48>()
{
	return ChannelType::type_uint16;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<Color48>()
{
	return ChannelOrder::order_rgb;
}

// Color64 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<Color64>()
{
	return 4;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<Color64>()
{
	return ChannelType::type_uint16;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<Color64>()
{
	return ChannelOrder::order_rgba;
}

// BMM_Color_24 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<BMM_Color_24>()
{
	return 3;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<BMM_Color_24>()
{
	return ChannelType::type_uint8;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<BMM_Color_24>()
{
	return ChannelOrder::order_rgb;
}

// BMM_Color_32 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<BMM_Color_32>()
{
	return 4;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<BMM_Color_32>()
{
	return ChannelType::type_uint8;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<BMM_Color_32>()
{
	return ChannelOrder::order_rgba;
}

// BMM_Color_48 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<BMM_Color_48>()
{
	return 3;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<BMM_Color_48>()
{
	return ChannelType::type_uint16;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<BMM_Color_48>()
{
	return ChannelOrder::order_rgb;
}

// BMM_Color_64 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<BMM_Color_64>()
{
	return 4;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<BMM_Color_64>()
{
	return ChannelType::type_uint16;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<BMM_Color_64>()
{
	return ChannelOrder::order_rgba;
}

// BMM_Color_fl specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<BMM_Color_fl>()
{
	return 4;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<BMM_Color_fl>()
{
	return ChannelType::type_float;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<BMM_Color_fl>()
{
	return ChannelOrder::order_rgba;
}

// BMM_Color_fl3 specialization ========================
template <>
constexpr int ImageLayoutInfo::NumChan<BMM_Color_fl3>()
{
	return 3;
}
template <>
constexpr ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<BMM_Color_fl3>()
{
	return ChannelType::type_float;
}
template <>
constexpr ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<BMM_Color_fl3>()
{
	return ChannelOrder::order_rgb;
}

#endif //DOXYGEN_SHOULD_SKIP_THIS

} // namespace MaxSDK::ColorManagement
