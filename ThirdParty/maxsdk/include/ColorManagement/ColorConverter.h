//**************************************************************************/
// Copyright (c) 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "..\maxheap.h"
#include <memory>
#include <WTypes.h>
#include "..\ColorManagement\ImageLayoutInfo.h"

namespace MaxSDK::ColorManagement {

class IColorEngine;


/** This is a fully header-implemented class wrapping IColorEngine for type-safe
 *  color conversions. Objects of this class are stateless thus the functions
 *  are reentrant and thread safe. You can call
 *  IColorPipeline::GetColorConverter() to acquire an instance for the desired
 *  input and output image formats. 
 *
 *  This class uses \ref ImageLayoutInfo specializations to understand the
 *  memory layout and structure of the pixels for the given type. By adding
 *  specializations for ImageLayoutInfo you can extend the support for custom
 *  types and pixel formats.
 *
 *  \tparam  TSRC Type of the source pixels.  
 *  \tparam  TDST Type of the destination pixels. For in-place conversion TSRC
 *	         and TDST should be identical.  
 *  \see     \ref ColorManagementOverview, IColorPipeline, ImageLayoutInfo */
template <class TSRC, class TDST>
class ColorConverter
{
public:

	/** Constructor, IColorPipeline will create instances, developers should not
	 * instantiate this class directly */
	ColorConverter(std::unique_ptr<IColorEngine> pCPUEngine)
		: pEngine(std::move(pCPUEngine))
	{
	}

	/** Deleted default constructor. */
	ColorConverter() = delete;
	
	/** Destructor. Since IColorPipeline will pass ColorConverter objects via
	 * unique_ptr, life-cycle is auto-managed. */
	~ColorConverter(){};

	/** Converts a single scanline with optional pixel stepping.  
	 * \param  [in] pSrc A pointer to source buffer.  
	 * \param  [out] pDst A pointer to destination buffer.  
	 * \param  [in] W The width of the scanline in pixels.  
	 * \param  [in] stepSrc The source pixel stepping.  
	 * \param  [in] stepDst The destination pixel stepping. */
	void ConvertLine(const TSRC* pSrc, TDST* pDst, size_t W, size_t stepSrc = 1, size_t stepDst = 1)
	{
		constexpr int nChanSrc = ImageLayoutInfo::NumChan<TSRC>();
		constexpr ImageLayoutInfo::ChannelType eTypeSrc = ImageLayoutInfo::ChanType<TSRC>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderSrc = ImageLayoutInfo::ChanOrder<TSRC>();
		ptrdiff_t pixStrSrc = stepSrc * ImageLayoutInfo::PixelStrideBytes<TSRC>();

		constexpr int nChanDst = ImageLayoutInfo::NumChan<TDST>();
		constexpr ImageLayoutInfo::ChannelType eTypeDst = ImageLayoutInfo::ChanType<TDST>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderDst = ImageLayoutInfo::ChanOrder<TDST>();
		ptrdiff_t pixStrDst = stepDst * ImageLayoutInfo::PixelStrideBytes<TDST>();

		pEngine->ConvertBuffers(
			pSrc, nChanSrc, eTypeSrc, eOrderSrc, pixStrSrc, 0, 
			pDst, nChanDst, eTypeDst, eOrderDst, pixStrDst, 0, 
			W, 1);
	};

	/** Converts images with contiguous pixel memory layouts  
	 * \param   [in] pSrc A pointer to source buffer.  
	 * \param   [out] pDst A pointer to destination buffer.  
	 * \param   [in] W The width of the image in pixels.  
	 * \param   [in] H The height of the image in pixels. */
	void ConvertImage(const TSRC* pSrc, TDST* pDst, size_t W, size_t H)
	{
		constexpr int nChanSrc = ImageLayoutInfo::NumChan<TSRC>();
		constexpr ImageLayoutInfo::ChannelType eTypeSrc = ImageLayoutInfo::ChanType<TSRC>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderSrc = ImageLayoutInfo::ChanOrder<TSRC>();
		constexpr ptrdiff_t pixStrSrc = ImageLayoutInfo::PixelStrideBytes<TSRC>();

		constexpr int nChanDst = ImageLayoutInfo::NumChan<TDST>();
		constexpr ImageLayoutInfo::ChannelType eTypeDst = ImageLayoutInfo::ChanType<TDST>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderDst = ImageLayoutInfo::ChanOrder<TDST>();
		constexpr ptrdiff_t pixStrDst = ImageLayoutInfo::PixelStrideBytes<TDST>();

		pEngine->ConvertBuffers(
			pSrc, nChanSrc, eTypeSrc, eOrderSrc, pixStrSrc, 0, 
			pDst, nChanDst, eTypeDst, eOrderDst, pixStrDst, 0, 
			W, H);
	};

	/** Performs in-place color conversion. This function requires the TSRC and
	 * TDST template parameters have the same memory layout. If that's not the
	 * case, calling this function will create a compile-time error.  
	 * \param   [in,out] pImg A pointer to buffer for in-place conversion.  
	 * \param   [in] W The width of the buffer in pixels.  
	 * \param   [in] H The height of the buffer in pixels. */
	void ConvertImage(TSRC* pImg, size_t W, size_t H)
	{
		constexpr int nChanSrc = ImageLayoutInfo::NumChan<TSRC>();
		constexpr ImageLayoutInfo::ChannelType eTypeSrc = ImageLayoutInfo::ChanType<TSRC>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderSrc = ImageLayoutInfo::ChanOrder<TSRC>();
		constexpr ptrdiff_t pixStrSrc = ImageLayoutInfo::PixelStrideBytes<TSRC>();

		constexpr int nChanDst = ImageLayoutInfo::NumChan<TDST>();
		constexpr ImageLayoutInfo::ChannelType eTypeDst = ImageLayoutInfo::ChanType<TDST>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderDst = ImageLayoutInfo::ChanOrder<TDST>();
		constexpr ptrdiff_t pixStrDst = ImageLayoutInfo::PixelStrideBytes<TDST>();

		//make sure this Converter specialization can to in-place conversion.
		static_assert(
			(nChanSrc == nChanDst) && (eTypeSrc == eTypeDst) && (eOrderSrc == eOrderDst) && (pixStrSrc == pixStrDst),   
			"In-place color conversion requires TSRC and TDST have the same memory layout");

		pEngine->ConvertBuffers(
			pImg, nChanSrc, eTypeSrc, eOrderSrc, pixStrSrc, 0, 
			pImg, nChanDst, eTypeDst, eOrderDst, pixStrDst, 0, 
			W, H);
	};

	/** Single color conversion. Please note that calling this function in a
	 * loop will be much slower than using the functions which operate on 1D or
	 * 2D arrays. Unless you really need to convert only a few colors, please
	 * use the functions that operate on arrays.  
	 * \param   [in] cIn source color.  
	 * \return  The converted color. */
	TDST ConvertColor(const TSRC& cIn)
	{
		constexpr int nChanSrc = ImageLayoutInfo::NumChan<TSRC>();
		constexpr ImageLayoutInfo::ChannelType eTypeSrc = ImageLayoutInfo::ChanType<TSRC>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderSrc = ImageLayoutInfo::ChanOrder<TSRC>();
		ptrdiff_t pixStrSrc = ImageLayoutInfo::PixelStrideBytes<TSRC>();

		constexpr int nChanDst = ImageLayoutInfo::NumChan<TDST>();
		constexpr ImageLayoutInfo::ChannelType eTypeDst = ImageLayoutInfo::ChanType<TDST>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderDst = ImageLayoutInfo::ChanOrder<TDST>();
		ptrdiff_t pixStrDst = ImageLayoutInfo::PixelStrideBytes<TDST>();

		TDST cOut;
		pEngine->ConvertBuffers(
			&cIn , nChanSrc, eTypeSrc, eOrderSrc, pixStrSrc, 0, 
			&cOut, nChanDst, eTypeDst, eOrderDst, pixStrDst, 0, 
			1, 1);

		return cOut;
	};

	/** Converts images with pixel stepping and line padding options.
	 *  \param  [in] pSrc source pixel array.  
	 *  \param  [in] stepSrc The source pixel stepping (in pixels). Use 1 to
	 *	        process every pixel.  
	 *  \param  [in] iLinePaddingBytesSrc source buffer line padding in bytes.
	 *	        Use 0 if the buffer is contiguous.  
	 *  \param  [out] pDst pixel array.  
	 *  \param  [in] stepDst pixel stepping (in pixels). Use 1 to
	 *	        process pixels in order.  
	 *  \param  [in] iLinePaddingBytesDst buffer line padding in
	 *	        bytes. Use 0 if the buffer is contiguous.  
	 *  \param  [in] W width of the image.  
	 *  \param  [in] H height of the image. */
	void ConvertImage(
		const TSRC* pSrc, size_t stepSrc, ptrdiff_t iLinePaddingBytesSrc,
		TDST* pDst,       size_t stepDst, ptrdiff_t iLinePaddingBytesDst, 
		size_t W, size_t H)
	{
		constexpr int nChanSrc = ImageLayoutInfo::NumChan<TSRC>();
		constexpr ImageLayoutInfo::ChannelType eTypeSrc = ImageLayoutInfo::ChanType<TSRC>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderSrc = ImageLayoutInfo::ChanOrder<TSRC>();
		ptrdiff_t pixStrSrc = stepSrc * ImageLayoutInfo::PixelStrideBytes<TSRC>();

		constexpr int nChanDst = ImageLayoutInfo::NumChan<TDST>();
		constexpr ImageLayoutInfo::ChannelType eTypeDst = ImageLayoutInfo::ChanType<TDST>();
		constexpr ImageLayoutInfo::ChannelOrder eOrderDst = ImageLayoutInfo::ChanOrder<TDST>();
		ptrdiff_t pixStrDst = stepDst * ImageLayoutInfo::PixelStrideBytes<TDST>();

		pEngine->ConvertBuffers(
			pSrc, nChanSrc, eTypeSrc, eOrderSrc, pixStrSrc, iLinePaddingBytesSrc, 
			pDst, nChanDst, eTypeDst, eOrderDst, pixStrDst, iLinePaddingBytesDst, 
			W, H);
	}

private: 
	std::unique_ptr<IColorEngine> pEngine;
};

/** Interface class for internal color transformation engine, developers should
 *  not use this class directly. Please use \ref ColorConverter class which
 *  provides convenient, type-safe and life-time managing usage by wrapping the
 *  IColorEngine objects.  
 *  \see    \ref ColorManagementOverview, ColorConverter */
class IColorEngine : public MaxHeapOperators
{
public:
	virtual ~IColorEngine() = default;

	virtual void ConvertBuffers(
		const void* pSrc, 
		int nChanSrc, 
		ImageLayoutInfo::ChannelType eChanTypeSrc, 
		ImageLayoutInfo::ChannelOrder eChanOrderSrc, 
		ptrdiff_t iPixelStrideSrc, 
		ptrdiff_t iLinePaddingBytesSrc,

		void* pDst, 
		int nChanDst, 
		ImageLayoutInfo::ChannelType eChanTypeDst, 
		ImageLayoutInfo::ChannelOrder eChanOrderDst,
		ptrdiff_t iPixelStrideDst, 
		ptrdiff_t iLinePaddingBytesDst, 

		size_t W, 
		size_t H) = 0;
};


} // namespace MaxSDK::ColorManagement
