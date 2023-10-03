//**************************************************************************/
// Copyright (c) 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <memory>
#include "..\ifnpub.h"
#include "..\GetCOREInterface.h"
#include "ImageLayoutInfo.h"
#include "InputColorSpaceInfo.h"
#include "ColorConverter.h"
#include "IColorPipeline.h"
#include "IModeSettings.h"

//! The interface ID for class IColorPipelineMgr, use this to get the instance.
#define COLORPIPELINEMGR_INTERFACE Interface_ID(0xba4ab2a1, 0xc010c010)

namespace MaxSDK::ColorManagement {


// ----------------------------------------------------------------------------
/*! \page ColorManagementOverview Color Management Overview 
 *
 * 3ds %Max can work in three different color management modes:
 * - <b>Unmanaged</b>: All the color swatches and the bitmap pixel values are
 *   used as-is, i.e. no physical to perceptual conversions (like gamma
 *   encoding/decoding) will be performed.
 * - <b>Gamma</b>: This is also called "de-gamma/re-gamma work flow". In this
 *   mode all the input colors and textures are converted to linear space using
 *   the input gamma values. Similarly all the output colors are properly
 *   gamma-encoded for the output medium (monitor, file, etc). The color
 *   computations (like rendering) are done in linear (scene-referred) space.
 *   This mode assumes that all of the colors are expressed in the same color
 *   gamut (typically sRGB).
 * - <b>OpenColorIO</b>: There are two OCIO modes
 *   (ColorPipelineMode::kOCIO_DEFAULT and ColorPipelineMode::kOCIO_CUSTOM)
 *   which are the most powerful color management modes 3ds %Max provides. In
 *   these modes the user can use textures and colors from different color
 *   spaces, can pick any linear color space (such as ACEScg) to perform
 *   renderings in, can work on monitors with different color gamuts and can
 *   save images in various color spaces with different view transforms applied.
 *   This mode uses OpenColorIO as the color management engine and the available
 *   color spaces, displays, transforms etc are defined in OCIO config files.
 *
 *  IColorPipelineMgr is the central hub for controlling and querying the active
 *  color management mode, available color spaces, displays, views and and
 *  accessing the other settings. IModeSettings class provides access to
 *  detailed settings of each mode. (see IColorPipelineMgr::Settings() )
 *
 *  If you need to convert colors from one color space to another, you'll need
 *  to create objects of class IColorPipeline. You can create IColorPipeline
 *  objects using various methods of the IColorPipelineMgr class. Although
 *  IColorPipeline defines the operations needed to transform colors, it can not
 *  do the computations by itself. The actual conversions are done by the
 *  ColorConverter class which is a template class and can perform operations in
 *  an optimized way for the specified input and output data types.
 *
 *  Here is an example showing how the classes relate to each other and how to
 *  use them in some simple scenarios.
 *
 *  \code  
 *  // Get the pointer to the IColorPipelineMgr instance.  
 *  IColorPipelineMgr* colorPipelineMgr = IColorPipelineMgr::GetInstance();  
 *
 *  // Get the pipeline for transforming colors from the current rendering space  
 *  // to default display/view pair.  
 *  auto pipeline = colorPipelineMgr->GetDefaultViewingPipeline();  
 *
 *  // Get two converters, one for Color->BMM_Color_32, other for  
 *  // BMM_Color_64->BMM_Color_64 pixel data types.  
 *  auto conv_Flto32 = pipeline->GetColorConverter<Color, BMM_Color_32>();  
 *  auto conv_64to64 = pipeline->GetColorConverter<BMM_Color_64, BMM_Color_64>();  
 *
 *  // Example 1: single color conversion from Color in rendering color space to  
 *  // BMM_Color_32 in default display/view color space.  
 *  BMM_Color_32 out32 = conv_Flto32->ConvertColor(Color(1.0f, 0.5f, 0.25f));
 *
 *  // Example 2: in-place image conversion in parallel  
 *  BMM_Color_64 myImage[1280 * 1024];  
 *  parallel_for(  
 *      ...,  
 *      [&](...)  
 *      {  
 *          BMM_Color_64* bufferSliceForThread = ...  
 *          size_t sliceHeight = ...  
 *          conv_64to64->ConvertImage(bufferSliceForThread, 1280, sliceHeight);  
 *      }  
 *  );  
 *  \endcode  
 *
 * Please See Color Management in the 3ds %Max Developer Guide for more information.
 */
// ----------------------------------------------------------------------------


/** Change bitmask that are passed with color management notification messages.  
 *  This bit mask is passed in NotifyInfo::callParam with the \ref
 *  NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
 *  NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 notification codes. In rare occasions
 *  it's possible to get those notifications with none of the bits set, this
 *  happens during the "load color management settings" operation for example,
 *  where the loaded settings are the same as the previous ones.
 */
enum NotificationChangeMask : uint32_t
{
	ModeChanged				= 1 << 0,	//!< Color management mode has changed
	GammaModeSettingsChanged= 1 << 1,	//!< Settings which are used in the ColorPipelineMode::kGAMMA mode have changed.
	OCIOModeSettingsChanged	= 1 << 2,	//!< Settings which are used in the OCIO modes have changed.
	InputSettingsChanged	= 1 << 3,	//!< Settings that affect the color management settings that are used in bitmap loading have changed.
	OuputSettingsChanged	= 1 << 4,   //!< Settings that affect the color management settings that are used in bitmap saving have changed.
	RenderingSpaceChanged	= 1 << 5,	//!< Rendering (working) color space has changed.
	DisplayViewChanged		= 1 << 6,	//!< One or more of the display/view settings have changed.
	ParameterSpaceChanged	= 1 << 7,	//!< The color space in which the color literals are stored has changed.
	ModeReInitialized		= 1 << 8,	//!< Depending on the situation, re-initialization may change multiple things or may change nothing. Assume everything is changed.
	ModeLockStateChanged	= 1 << 9,	//!< OCIO config path's lock state is changed in the ColorPipelineMode::kOCIO_CUSTOM mode.
};



/** This is the central class for handling system-wide color management related
 *  data and tasks. This class supersedes the GammaMgr and IColorCorrectionMgr
 *  classes, providing unified settings and optimized color processing features
 *  in all color management modes. IModeSettings objects that can be retrieved
 *  using the IColorPipelineMgr::Settings() or
 *  IColorPipelineMgr::GetModeSettings() functions will contain detailed
 *  settings for the color management modes. This class also serves as a factory
 *  for creating IColorPipeline instances. If any active setting changes this
 *  manager will broadcast a set of notifications to notify the listeners.  
 *
 *  \warning Please note that none of the functions of this class are thread safe
 *	        and should ideally be called from the main thread.  
 *  \see    \ref ColorManagementOverview, IColorPipeline, ColorConverter  
 *  \see    NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE,
 *	        NOTIFY_COLOR_MANAGEMENT_POST_CHANGE,
 *	        NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2
 */
class IColorPipelineMgr : public FPStaticInterface
{
public:
	/** Returns a pointer to the system-wide instance of IColorPipelineMgr. This
	 * is a faster alternative to  
	 * \code  
	 * (IColorPipelineMgr*)GetCOREInterface(COLORPIPELINEMGR_INTERFACE)  
	 * \endcode  
	 *
	 * \return  Pointer to the system-wide instance.
	 */
	CoreExport static IColorPipelineMgr* GetInstance();

	
	/** \name Error Reporting Functions 
	*/
	///@{ ----------------------------------------------------------------------
	/** Name of last function that resulted in error. You can use this function
	 * along with GetLastErrorText() to know more about the latest failed call
	 * to IColorPipelineMgr functions.  
	 * 
	 * \return  The string holding the name name of the last function that
	 *          resulted in error. If there were no function calls that failed
	 *          since the last call to ClearLastError then the returned value
	 *          will be an empty string.  
	 * \see     GetLastErrorText, ClearLastError
	 */
	virtual MSTR GetLastErrorFunction() const = 0;

	/** Error string for the last occurring error in any of the function calls.
	 * If any function returns a fail code, you can use this function to get
	 * the details about the failure. Please use GetLastErrorFunction() to
	 * see/verify which function set this error text.  
	 * 
	 * \return  %String holding a function-specific explanation of the failure or
	 *          an empty string if there was no error since the last
	 *          ClearLastError() call.  
	 * \see		GetLastErrorFunction, ClearLastError
	 */
	virtual MSTR GetLastErrorText() const = 0;

	/** Resets the last error text and function to empty string. 
	 * \see		GetLastErrorFunction, ClearLastError
	 */
	virtual void ClearLastError() = 0;
	///@} ----------------------------------------------------------------------


	/** \name Color Management Mode Functions  
	 * In 3ds %Max there are 4 different color management modes (see
	 * \ref ColorPipelineMode enum). These functions will set and get the currently
	 * active mode. 
	 */
	///@{ ----------------------------------------------------------------------

	/** Sets the active color management mode.  
	 * Please note that changing the color management mode may have permanent
	 * effects on the scene data, especially on the input bitmaps and textures,
	 * and it may not be possible to revert. If the operation is successful,
	 * \ref NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 will be broadcast with
	 * NotificationChangeMask::modeChanged mask set in the post messages.
	 *
	 * \param   [in] mode one of the ColorPipelineMode enums. If you set the
	 *          color management mode to ColorPipelineMode::kOCIO_CUSTOM you
	 *          also need to make sure that a valid OCIO configuration file is
	 *          loaded, otherwise colors won't be managed.  
	 * \return  Returns true if the operation was successful. If it returns
	 *          false you can use GetLastErrorText and GetLastErrorFunction to
	 *          get details.  
	 */
	virtual bool SetColorPipelineMode(ColorPipelineMode mode) = 0;

	/** Returns the currently active color management mode. Use Settings
	 * function to access the individual settings of the active mode.
	 *
	 * \return	Currently active color management mode.
	 */
	virtual ColorPipelineMode GetColorPipelineMode() const = 0;
	///@} ----------------------------------------------------------------------

	
	/** \name Settings access functions.  
	 * These functions will return pointers to IModeSettings objects which hold
	 * detailed settings about the mode. Settings() function will return a
	 * pointer to a mutable object that holds the settings for the currently
	 * active mode, GetModeSettings() will return read-only object for the
	 * specified mode for query purposes.
	 */
	///@{ ----------------------------------------------------------------------

	/** Returns a pointer to a mutable object of type IModeSettings holding the
	 * detailed settings of the currently active color management mode. Through
	 * this pointer you can get and set the settings. 
	 *
	 * \return  Pointer to IModeSettings object to set/get settings of the currently active
	 *			color management mode.
	 * \see		GetModeSettings
	 */
	virtual IModeSettings* Settings() const = 0;

	/** Returns an read-only IModeSettings object for the specified mode. This
	 * can be used to query the settings of currently inactive the color
	 * management modes. To access the settings for the currently active mode,
	 * use the Settings() function instead.
	 *
	 * \param   [in] mode One of the ColorPipelineMode enum values specifying the
	 *          mode for which the IModeSettings object will be returned.  
	 * \return  Read-only pointer to the settings object for the specified mode.
	 *          If the specified mode is invalid nullptr will be returned.  
	 * \see		Settings
	 */
	virtual const IModeSettings* GetModeSettings(ColorPipelineMode mode) const = 0;
	///@} ----------------------------------------------------------------------

	
	/** \name Display Gamma Functions (ColorPipelineMode::kGAMMA mode only).  
	 * Display gamma settings are used only in ColorPipelineMode::kGAMMA mode and won't have
	 * any effect in other modes.
	 */
	///@{ ----------------------------------------------------------------------
	
	/** Sets the display gamma. Sets the display gamma value that's used in the
	 * ColorPipelineMode::kGAMMA mode. It won't have any effect in other color
	 * management modes. If the operation is successful, \ref
	 * NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 notifications will be broadcast with
	 * GammaModeSettingsChanged flag set. Display gamma value will always be in
	 * sync with the GammaMgr and IColorCorrectionMgr.
	 *
	 * \param   [in] newGamma This is the display gamma value to be used.  
	 * \return  True if the operation is successful.
	 */
	virtual bool	SetDisplayGamma(float newGamma) = 0;

	/** Retrieves the gamma value used for the display in the
	 * ColorPipelineMode::kGAMMA mode.  
	 * In other modes this value is not used.
	 *
	 * \return  Gamma value set for the display for the
	 *          ColorPipelineMode::kGAMMA mode.
	 */ 
	virtual float	GetDisplayGamma() const = 0;
	///@} ----------------------------------------------------------------------

	
	/** \name File I/O Gamma Functions (ColorPipelineMode::kGAMMA only).  
	 *  Accessor functions for the file I/O gamma settings which are used only
	 *  in ColorPipelineMode::kGAMMA mode. These values won't be used in any
	 *  other color management mode. 
	 */
	///@{ ----------------------------------------------------------------------

	/** Sets the default gamma that's used for the loaded bitmaps in the
	 * ColorPipelineMode::kGAMMA mode. If the operation is successful, \ref
	 * NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 notifications will be broadcast with
	 * GammaModeSettingsChanged and InputSettingsChanged flags set.
	 *
	 * \param   [in] newGamma default gamma value to be used for bitmap load
	 *          operations.  
	 * \return  True if the operation is successful.
	 */
	virtual bool	SetDefaultFileInGamma(float newGamma) = 0;

	/** Returns the default gamma value that will be used in the bitmap load
	 * operations in the ColorPipelineMode::kGAMMA mode.
	 *
	 * \return      Default gamma value for the bitmap load operations.
	 */
	virtual float	GetDefaultFileInGamma() const = 0;
	
	/** Sets the default gamma that's used for the saved bitmaps in the
	 * ColorPipelineMode::kGAMMA mode. If the operation is successful, \ref
	 * NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 notifications will be broadcast with
	 * GammaModeSettingsChanged and OutputSettingsChanged flags set.
	 *
	 * \param   [in] newGamma default gamma value to be used for bitmap save
	 *          operations.  
	 * \return  True if the operation is successful.
	 */

	virtual bool	SetDefaultFileOutGamma(float newGamma) = 0;
	
	/** Returns the default gamma value that will be used in the bitmap save
	 * operations in the ColorPipelineMode::kGAMMA mode.
	 *
	 * \return      Default gamma value for the bitmap save operations.
	 */
	virtual float	GetDefaultFileOutGamma() const = 0;
	///@} ----------------------------------------------------------------------

	
	/** \name Monitor Display Functions  
	 * These functions are provided for supporting OpenColorIO v2 virtual
	 * displays in the future. Currently they are not implemented.
	 */
	///@{ ----------------------------------------------------------------------
	/// This function is provided for future use. Not implemented yet.
	virtual size_t	GetNumSystemMonitorDisplays() const = 0;
	/// This function is provided for future use. Not implemented yet.
	virtual MSTR	GetSystemMonitorDisplayName(size_t idx) const = 0;
	/// This function is provided for future use. Not implemented yet.
	virtual size_t	GetSystemMonitorDisplayIndex(const MSTR& displayName) const = 0; // returns (size_t)(-1) if not found.
	/// This function is provided for future use. Not implemented yet.
	virtual MSTR	GetDisplayForDesktopCoordinate(POINT& p) const = 0;
	/// This function is provided for future use. Not implemented yet.
	virtual MSTR	GetDisplayForWindow(HWND hWnd) const = 0;
	///@} ----------------------------------------------------------------------


	/** \name Factory Functions to Create ColorPipeline objects.  
	 * There functions are used to create IColorPipeline objects which define
	 * transformations that convert colors, usually from one color space to
	 * another color space through a specific set of operations. Although these
	 * functions are designed around OpenColorIO (OCIO) concepts, they are valid
	 * for non-OCIO modes too, you just need to make sure that parameters passed
	 * to those functions are valid for the current mode. (See \ref
	 * IModeSettings for available items in a given color management mode).
	 *
	 * These functions return unique_ptr and thus will provide lifetime
	 * management on the returned objects. 
	 * 
	 * \see     IColorPipeline, Settings, IModeSettings
	 */
	///@{ ----------------------------------------------------------------------
	
	/** Generic pipeline applying transformation from one space to another
	 * space.
	 *
	 * \param   [in] sFromSpace %Name of any color space or role specifying the source
	 *          color space. See Settings() and IModeSettings for querying the
	 *          currently available color spaces.  
	 * \param   [in] sToSpace %Name of any color space or role specifying the target
	 *          color space. See Settings() and IModeSettings for querying the
	 *          currently available color spaces.  
	 * \return  If successful, returns a unique_ptr to the IColorPipeline
	 *          object. If operation fails it returns an empty unique_ptr. You
	 *          can use GetLastErrorText() and GetLastErrorFunction() functions
	 *          to get details about the failure.
	*/
	virtual std::unique_ptr<IColorPipeline> GetPipeline(const MSTR& sFromSpace, const MSTR& sToSpace) = 0;

	/** Creates a generic gamma transformation pipeline.  
	 * Returned pipeline will apply only the gamma transfer function and won't
	 * change color gamut. Optional 4x4 matrix provides post-gamma operation
	 * which can be used for various purposes such as channel swizzling.
	 *
	 * \param   [in] gamma Coefficient for the power function that's used for the
	 *          gamma curve.  
	 * \param   [in] chanTM Pointer to an array of 16 double precision floating point
	 *          numbers forming a 4x4 matrix which can be used for channel
	 *          swizzling or any linear channel mixing/scaling operations.  
	 * \return  If successful, returns a unique_ptr to the IColorPipeline
	 *          object. If operation fails it returns an empty unique_ptr. You
	 *          can use GetLastErrorText() and GetLastErrorFunction() functions
	 *          to get details about the failure.   
	 */
	virtual std::unique_ptr<IColorPipeline> GetGammaPipeline(float gamma, const double* chanTM = nullptr) = 0;

	// Default Viewing Pipeline
	virtual std::unique_ptr<IColorPipeline> GetDefaultViewingPipeline(
			DisplayViewTarget eTarget = DisplayViewTarget::kGLOBAL, bool inverse = false,
			const double* chanTM = nullptr) = 0;

	// color converters for a display pipeline
	virtual std::unique_ptr<IColorPipeline> GetViewingPipeline(const MSTR& sFromSpace, const MSTR& sDisplay,
			const MSTR& sView, bool inverse = false, const double* chanTM = nullptr) = 0;

	// complex viewing pipeline converter with dynamic parameters and channel swizzling option
	virtual std::unique_ptr<IColorPipeline> GetAdvancedViewingPipeline(const MSTR& sFromSpace, const MSTR& sDisplay,
			const MSTR& sView, bool inverse = false, const double* chanTM = nullptr) = 0;
	///@} ----------------------------------------------------------------------

	/** \name Functions related to Saving\Loading Settings
	*/
	///@{ ----------------------------------------------------------------------

	/** Saves the current color pipeline settings as the marketing defaults.
	 * System defaults will be loaded when 3ds %Max starts, resets or on new file
	 * operation.  
	 *
	 * \return  True if the operation was successful. 
	 */
	virtual bool	SaveSystemDefaults() = 0;

	/** Loads the settings from the marketing defaults.  Returns false if the
	 * operation fails, for example due to the missing OCIO config file.
	 *
	 * \return  True if successful. False if the operation fails - for example
	 *          due to the missing OCIO config file.
	 */
	virtual bool	LoadSystemDefaults() = 0;
	
	/** Loads color management settings from a scene file, used internally. */
	virtual IOResult Load(ILoad* iload, int loadVersion) = 0;

	/** Saves color management settings to a scene file, used internally.	*/
	virtual IOResult Save(ISave* isave) = 0;
	///@} ----------------------------------------------------------------------
};


} // namespace MaxSDK::ColorManagement
