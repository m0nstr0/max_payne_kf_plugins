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
#include "..\strclass.h"
#include <memory>
#include "ImageLayoutInfo.h"
#include "ColorConverter.h"

namespace MaxSDK::ColorManagement {

class IColorEngine;
class IParameter;

/** Interface class that defines color conversions from source color space to
 * target color space. User can get an instance by calling one of the
 * GetXXXPipeline() functions of the class IColorPipelineMgr. Actual color
 * conversion will be done by a ColorConverter object which can be created by
 * calling IColorPipeline::GetColorConverter() templated factory functions with
 * desired source and destination image formats.
 *
 * \see     \ref ColorManagementOverview, ColorConverter, ImageLayoutInfo  
 * \warning Please note that none of the functions of this class are thread safe
 *          and should ideally be called from the main thread.
 */
class IColorPipeline : public MaxHeapOperators
{
public:
	/**
	 *  Dynamic parameters for the IColorPipeline objects. Use
	 *  IColorPipeline::GetParameter() to receive the pointers to the parameters the
	 *  pipeline may have.
	 */
	class IParameter : public MaxHeapOperators
	{
	public:
		virtual ~IParameter() = default;

		//! Returns the name of the dynamic parameter. Each dynamic parameter has a unique name.
		virtual MSTR GetName() const = 0;
		
		//! Returns the value of the dynamic parameter.
		virtual float GetValue() = 0;

		//! Sets the value of the dynamic parameter.
		virtual bool SetValue(float val) = 0;
	};

	//! ColorConverter Flags for future expansion.
	enum Flags : uint32_t
	{
		FLAG_NONE	= 0,
	};

public:
	IColorPipeline() = default;
	virtual ~IColorPipeline() = default;

	/** \name Dynamic Parameters */
	///@{ 
	//! Returns the number of dynamic parameters this pipeline has, can be zero.
	virtual size_t		GetNumParameters() const = 0;

	//! Retrieves the pointer to the dynamic parameter object by 0-based index number.
	virtual IParameter* GetParameter(size_t idx) const = 0;

	//! Retrieves the pointer to the dynamic parameter by the parameter name. Each parameter has a unique name.
	virtual IParameter* GetParameter(const MSTR& paramName) const = 0;
	///@}

	/** This function returns an optimized color converter object for the given
	 *  source and destination pixel data types. Returned ColorConverter objects
	 *  are  stateless, thus can safely be used to process colors in multiple
	 *  threads in parallel. Returned value is unique_ptr which means caller
	 *  owns the object.
	 *
	 *  Calling IColorPipeline::GetColorConverter() for the first time for a
	 *  complex pipeline may take some time, thus for time-critical tasks get a
	 *  ColorConverter pointer beforehand and reuse it when needed. The system
	 *  caches color converter objects for a given TSRC, TDST pair, so
	 *  subsequent calls to this function with the same TSRC, TDST values may be
	 *  faster than the first call.  
	 *  \sa     ImageLayoutInfo  
	 *  \tparam TSRC: Source pixel data type. ImageLayoutInfo should have a
	 *	        specialization for this this type, otherwise you'll get a
	 *	        compile-time error.  
	 *  \tparam TDST: Destination pixel data type. ImageLayoutInfo should have a
	 *	        specialization for this this type, otherwise you'll get a
	 *	        compile-time error.  
	 *  \param  [in] flags: currently not used. Always pass FLAG_NONE.  
	 *  \return unique_ptr to optimized ColorConverter object for the specified
	 *	        TSRC and TDST pixel types.  
	*/
	template <class TSRC, class TDST>
	std::unique_ptr<ColorConverter<TSRC, TDST>> GetColorConverter(uint32_t flags=FLAG_NONE)
	{
		constexpr ImageLayoutInfo::ChannelType eTypeSrc = ImageLayoutInfo::ChanType<TSRC>();
		constexpr ImageLayoutInfo::ChannelType eTypeDst = ImageLayoutInfo::ChanType<TDST>();

		auto engine = std::unique_ptr<IColorEngine>(GetEngine(eTypeSrc, eTypeDst, flags));
		auto pCPUConverter = std::make_unique<ColorConverter<TSRC, TDST>>(std::move(engine));
		return pCPUConverter;
	}

	/** Returns list of operations this pipeline will apply.  
	 * This function can be used for diagnostic purposed to check and verify the
	 * list of operations the pipeline will apply to transform colors. 
	 *
	 * \return  List of operations both in non-optimized and optimized form.
	 *			Operations will be listed in CTF (color transform format).
	 */
	virtual MSTR GetDiagnosticString() = 0;

protected:
	//! Internal function.
	virtual IColorEngine* GetEngine(ImageLayoutInfo::ChannelType eTypeSrc, ImageLayoutInfo::ChannelType eTypeDst, uint32_t flags) = 0;
};

} // namespace MaxSDK::ColorManagement