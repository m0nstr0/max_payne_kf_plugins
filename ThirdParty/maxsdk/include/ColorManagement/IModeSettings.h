//**************************************************************************/
// Copyright (c) 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "..\strclass.h"

namespace MaxSDK::ColorManagement {

/// Enum for color management modes.
enum class ColorPipelineMode : uint8_t
{
	kUNMANAGED,    //!< %Color management turned off.
	kGAMMA,        //!< %Color management using de-gamma/re-gamma work flow.
	kOCIO_DEFAULT, //!< Uses the built-in OCIO config
	kOCIO_CUSTOM,  //!< Uses custom OCIO config.

	kNumColorPipelineModes
};

// Enum for the initialization state of the mode
enum class ModeStatus : uint8_t
{
	kNORMAL,       //!< Mode is initialized as intended and is healthy.
	kFALLBACK_RAW, //!< Mode initialization failed but initialized as "raw". If this mode is selected 3ds %Max will operate as colors unmanaged.
	kINVALID,      //!< Something terrible happened, Switching to this mode may make system unstable.
};

/// Enum for source of the config file that's used in the mode.
enum class ConfigSource : uint8_t
{
	kBUILT_IN,     //!< The OCIO Setting is based on a build-in OCIO config.
	kENV_VARIABLE, //!< The OCIO Setting is based on a OCIO config file specified by the 'OCIO' environment variable.
	kUSER_DEFINED  //!< The OCIO Setting is based on a user-provided OCIO config file.
};

/// Enum for identifying display/view pair targets.
enum class DisplayViewTarget : uint8_t
{
	kGLOBAL,       //!< The Global default - all automatic view targets derive from this.
	kVIEWPORT,     //!< The Viewport
	kFRAME_BUFFER, //!< The FrameBuffer
	kMTL_EDITOR,   //!< The %Material Editor
	kCOLOR_PICKER, //!< The %Color Picker - the Color Swatch will derive from this, if it is set to automatic.
	kCOLOR_SWATCH, //!< The %Color Swatch - when set to automatic, the value from the Color-Picker will be derived.

	kNumDisplayViewTargets
};

/// Enum for Output Color conversion options.
enum class OutputConversion : uint8_t
{
	kNO_CONVERSION,          //!< No conversion - output images will be saved in the Rendering %Color Space.
	kCOLOR_SPACE_CONVERSION, //!< %Color Space conversion: output images will be converted into the given Output %Color Space.
	kDISPLAY_VIEWTRANSFORM,  //!< Display/View Transform: output images will be converted into the given Display %Color Space and and optionally given %View Transform may be applied.
};

/** This class holds the detailed settings for each of the color management
 * modes (see ColorPipelineMode). You can get the instance for the currently
 * active mode by calling IColorPipelineMgr::Settings() function. You can also
 * get the read-only instances for the inactive modes via
 * IColorPipelineMgr::GetModeSettings() function.  
 *
 * \warning Please note that none of the functions of this class are thread safe
 *          and should ideally be called from the main thread.  
 */
class IModeSettings
{
public:
	virtual ~IModeSettings() = default;

	/** Returns the ColorPipelineMode used in this setting.  
	 * \return  The ColorPipelineMode used in this setting.   
	 * \see     ColorPipelineMode */
	virtual ColorPipelineMode Mode() const = 0;


	/** Returns if the setting is active.  
	 * The IModeSettings returned by IColorPipelineMgr::Settings() is always
	 * active, while the various IModeSettings returned by
	 * IColorPipelineMgr::GetModeSettings() may or may not be active.  
	 * \return  True if the mode is the currently active one.  
	 * \see     IColorPipelineMgr::Settings(),
	 *          IColorPipelineMgr::GetModeSettings() */
	virtual bool IsActive() const = 0;


	/** Returns true if this IModeSettings is based on a OCIO configuration.  
	 * Convenience function for checking if the ColorPipelineMode is
	 * ColorPipelineMode::kOCIO_DEFAULT or ColorPipelineMode::kOCIO_CUSTOM.  
	 * \see     ColorPipelineMode */
	virtual bool IsOCIOBased() const = 0;


	/** Returns the initialization state of the mode.  
	 * \return  ModeStatus::kNORMAL if the mode is initialized successfully as
	 *          intended. If there were problems during initialization (for
	 *          example due to invalid OCIO config file) then this will return
	 *          either ModeStatus::kFALLBACK_RAW or ModeStatus::kINVALID. If
	 *          the return value is ModeStatus::kFALLBACK_RAW it's still
	 *          possible to use this mode but it will act as "unmanaged" mode.
	 *          ModeStatus::kINVALID indicates internal error.  
	 * \see     ModeStatus */
	virtual ModeStatus GetStatus() const =0;
	

	/** Will lock/unlock the mode. If the mode is unlocked you can change the
	 * OCIO config file it uses. Note that many of the the built-in modes are
	 * internally locked and can't be unlocked.  
	 * \param   [in] lock true to lock, false to unlock.  
	 * \return  true if the operation is successful.  
	 * \see     GetLocked() */
	virtual bool SetLocked(bool lock) = 0;

	/** Returns true if the current mode is locked. Configuration file of the
	 * Locked modes cannot be changed from the UI or MaxScript.  
	 * \return  True if the current mode is locked.  
	 * \see     SetLocked() */
	virtual bool GetLocked() const = 0;


	/** \name Description Retrieval Functions  
	 * These functions can be (and internally are) used to display some
	 * additional information inside the UI. In OCIO modes, these information is
	 * extracted from the OCIO config file. */
	///@{ ----------------------------------------------------------------------

	/** Returns some descriptive information about the current Mode.  
	 * \return  A MSTR describing the current mode. Might be empty.  
	 * \see     GetColorSpaceDescription, GetDisplayViewDescription */
	virtual MSTR GetModeDescription() const = 0;
	
	/** Returns some descriptive information about the given %Color Space.  
	 * \param   [in] colorSpace The %Color Space to be queried.
	 * \return  A MSTR describing the given %Color Space. Might be empty.  
	 * \see     GetModeDescription, GetDisplayViewDescription */
	virtual MSTR GetColorSpaceDescription(const MSTR& colorSpace) const = 0;

	/** Returns some descriptive information about the given %Display and %View
	 * Transform.  
	 * \param   [in] display The %Display to be queried.
	 * \param   [in] view The %View to be queried.
	 * \return  A MSTR describing the given %Display and %View. Might be empty.  
	 * \see     GetModeDescription, GetColorSpaceDescription */
	virtual MSTR GetDisplayViewDescription(const MSTR& display, const MSTR& view) const = 0;

	// @} ----------------------------------------------------------------------


	/** This will trigger re-initialization of the mode, causing the OCIO config
	 * file (if any used in this mode) be re-loaded using the current values of
	 * the environment and context variables.  If this mode is the active mode
	 * then \ref NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 notification will be broadcast.  
	 * \return  True if the mode is re-initialized successfully as intended. If
	 *          there were problems during initialization (such as invalid OCIO
	 *          config file) then this will return false. It's still possible to
	 *          use this mode in that case but it will act as "unmanaged"
	 *          mode.*/
	virtual bool ReInitilize() = 0; //TODO: rename to ReInitialize


	/** Returns the full path of the config file used in the current mode.
	 * Returns empty string if the mode uses an in-memory raw config.  
	 * \see    SetOCIOConfigFilePath */
	virtual MSTR GetOCIOConfigFilePath() const = 0;
	
	/** If the mode is not Locked, this sets the config file to be used in this
	 * mode.  
	 * If the operation is successful, \ref NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE,
	 * \ref NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 notification messages will be sent
	 * and the function will return true.  
	 * If the operation was unsuccessful it will return false.
	 * IColorPipelineMgr::GetLastErrorText() may have information about the
	 * cause of the failure.  
	 * \param   [in] pathToConfig The path to the OCIO config file to load.  
	 * \return  True, if the mode is not locked and the config file could be
	 *          loaded successfully. If the operation was unsuccessful it
	 *          will return false.  IColorPipelineMgr::GetLastErrorText() may
	 *          have information about the cause of the failure.   
	 * \see     GetOCIOConfigFilePath, GetLocked(),
	 *          SetLocked() */
	virtual bool SetOCIOConfigFilePath(const MSTR& pathToConfig) = 0;

	/** Returns the source of the OCIO config file used in this mode.  
	 * \return  The source of the OCIO config file used in this mode.  
	 * \see     ConfigSource */
	virtual ConfigSource GetOCIOConfigFileSource() const = 0;


	/** \name Rendering Color Space Functions  
	 *  Rendering color space is the scene-linear color space where the lighting
	 *  and shading computations are performed. Following functions will let the
	 *  API user to get a list of available rendering color spaces in the active
	 *  OCIO config. */
	///@{ ----------------------------------------------------------------------

	/** Returns the number of available %Rendering %Color Spaces in this
	 * setting.  
	 * \return  The number of available %Rendering %Color Spaces. */
	virtual size_t GetNumRenderingColorSpaces() const = 0;

	/** Returns the name of the %Rendering %Color Space with the given index.  
	 * \param   [in] idx The index of the %Rendering %Color Space to be asked.  
	 * \return  The name of the %Rendering %Color Spaces. with the given index
	 *          of an empty string (if the given idx is invalid). */
	virtual MSTR GetRenderingColorSpace(size_t idx) const = 0;
	
	/** Returns the index of the %Rendering %Color Space with the given name or
	 * \c (size_t)(-1) if the color space with the given name cannot be found in
	 * the active setting.  
	 * \note    %Color Space names comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] colorSpaceName The name of the %Rendering %Color Space to
	 *          be queried.  
	 * \return  The index of the %Rendering %Color Spaces, or \c (size_t)(-1).
	 * */
	virtual size_t GetRenderingColorSpaceIndex(const MSTR& colorSpaceName) const = 0;

	/** Returns the name of the current %Rendering %Color Space used to render
	 * the scene.  
	 * \return  The name of the %Rendering %Color Spaces used. */
	virtual MSTR GetCurrentRenderingColorSpace() const = 0;

	/** Sets the current %Rendering %Color Space to be used within this setting.
	 * If the given %Color Space can be applied, \ref
	 * NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 will be sent.  
	 * \note    %Color Space name comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] colorSpaceName The name of the %Rendering %Color Space to
	 *          be used.  
	 * \return  True, if the colorspace could be changed. If the operation was
	 *          unsuccessful it will return false.
	 *          IColorPipelineMgr::GetLastErrorText() may have information about
	 *          the cause of the failure.   
	 * \see     GetCurrentRenderingColorSpace, GetLocked */
	virtual bool SetCurrentRenderingColorSpace(const MSTR& colorSpaceName) = 0;
	///@} ----------------------------------------------------------------------

	/** Returns the name of the %Color Space that is used for RGB values
	 * specified and stored in the scene (such as param blocks).  
	 * \return  The name of the %Color Space used for RGB values.  
	 * \see     SetParameterColorSpaceName */
	virtual MSTR GetParameterColorSpaceName() const = 0;
	
	/** Sets the %Color Space that is used for specifying and storing RGB values
	 * in the scene (such as param blocks). This color space needs to be a scene
	 * referred linear color space.  
	 * If the given %Color Space can be applied, \ref
	 * NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE, \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and \ref
	 * NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 will be sent.  
	 * \note    %Color Space names comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] paramColorSpaceName The name of the %Color Space to be
	 *          used.  
	 * \return  True if the given %Color Space could be set. If the operation
	 *          was unsuccessful it will return false.
	 *          IColorPipelineMgr::GetLastErrorText() may have information about
	 *          the cause of the failure.    
	 * \see     GetParameterColorSpaceName */
	virtual bool SetParameterColorSpaceName(const MSTR& paramColorSpaceName) = 0;

	/** Returns the name of the %Color Space that is used for data colors or
	 * data images. Usually the "raw" color space.  
	 * \return  Returns the name of the %Color Space that is used for data
	 *          colors or data images.  
	 * \see     SetDataColorSpaceName */
	virtual MSTR GetDataColorSpaceName() const = 0;
	
	/** Sets the %Color Space that is used for specifying data colors or data
	 * images. Usually the "raw" color space. If the given %Color Space can be
	 * applied, \ref NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE,
	 * \ref NOTIFY_COLOR_MANAGEMENT_POST_CHANGE and
	 * \ref NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 will be sent.  
	 * \note    %Color Space name comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] dataColorSpaceName The name of the %Color Space to be used.
	 * \return  True if the given %Color Space could be set. If the operation
	 *          was unsuccessful it will return false.  
	 *          IColorPipelineMgr::GetLastErrorText() may have information about
	 *          the cause of the failure.  
	 * \see     GetParameterColorSpaceName */
	virtual bool SetDataColorSpaceName(const MSTR& dataColorSpaceName) = 0;

	/** \name File I/O Color Space Functions  
	 *  These functions will let you get a list of currently available file I/O
	 *  color spaces as well the input rules which are used for assigning color
	 *  spaces to the input textures based on the file name.
	 */
	///@{ ----------------------------------------------------------------------

	/** Returns the number of %Color Spaces available for use in file IO.
	 * \return  The number of available %Color Spaces.  
	 * \see     GetFileIOColorSpace, GetFileIOColorSpaceIndex */
	virtual size_t GetNumFileIOColorSpaces() const = 0;

	/** Returns the name of the file IO %Color Space with the given index.  
	 * \param   [in] idx The index of the file IO %Color Space to be returned.  
	 * \return  The name of the file IO %Color Space with the given index or an
	 *          empty string in case the index is invalid.  
	 * \see     GetNumFileIOColorSpaces, GetFileIOColorSpaceIndex */
	virtual MSTR GetFileIOColorSpace(size_t idx) const = 0;

	/** Returns the index of the file IO %Color Space with the given name.  
	 * \note    %Color Space names comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] colorSpaceName The name of the file IO %Color Space to be
	 *          queried.  
	 * \return  The index of the file IO %Color Space with the given name or
	 *          \c size_t(-1) if the passed color space name is not found.  
	 * \see     GetNumFileIOColorSpaces, GetFileIOColorSpace */
	virtual size_t GetFileIOColorSpaceIndex(const MSTR& colorSpaceName) const = 0;

	/** Returns the number of available file IO rules in this setting.  
	 * \return  The number of available file IO rules.  
	 * \see     GetFileIORule, GetColorSpaceFromImageFilePath */
	virtual size_t GetNumFileIORules() const = 0;

	/** Gets the details of the idx'th file IO rule. Returns true if the
	 * information is successfully retrieved.  
	 * \note    Either the (pattern, ext) pair or the regex will be used in each
	 *          rule. If the (pattern, ext) pair is used then regex will return
	 *          empty (or vice verse)  
	 * \param   [in] idx The index of the file IO rule to retrieve information
	 *          on.  
	 * \param   [out] ruleName The name of the rule.  
	 * \param   [out] pattern The pattern of the rule.  
	 * \param   [out] ext The file extension this rule should apply on. This may
	 *          be an empty string.  
	 * \param   [out] regex A optional regular expression that is used to check
	 *          if this rule is to be applied - or an empty string.  
	 * \param   [out] colorSpace The name of the file IO %Color Space that this
	 *          rule will resolve to.  
	 * \return  True if the information could be successfully retrieved.  
	 * \see     GetNumFileIORules, GetColorSpaceFromImageFilePath */
	virtual bool GetFileIORule(size_t idx, MSTR& ruleName, MSTR& pattern, MSTR& ext, MSTR& regex, MSTR& colorSpace) const = 0;

	/** Returns if the file IO rules are applied using case-sensitive file
	 * names. By default the file rules in 3ds %Max are case-insensitive.  
	 * \return  True, if the file io rules resolution is done using
	 *          case-sensitive informations.  
	 * \see     SetFileIORulesCaseSensitive, GetColorSpaceFromImageFilePath */
	virtual bool GetFileIORulesCaseSensitive() const = 0;

	/** Sets if the file IO rules are applied using case-sensitive operations.
	 * \see     GetFileIORulesCaseSensitive, GetColorSpaceFromImageFilePath */
	virtual void SetFileIORulesCaseSensitive(bool bCaseSensitive) = 0;

	/** Runs the input rules on the passed image file path and returns the
	 * resulting color space.  
	 * \param   [in] filePath The file path to be checked.  
	 * \param   [out] ruleIndex Receives the index of the first rule that
	 *          matched the file path.  
	 * \return  The color space name to be used. */ 
	virtual MSTR GetColorSpaceFromImageFilePath(const MSTR& filePath, size_t& ruleIndex) const = 0;
	///@} ----------------------------------------------------------------------

	/** \name Rendering Output Functions */
	///@{ ----------------------------------------------------------------------
	/** Returns the default output conversion. The default output conversion is
	 * getting applied when saving a bitmap and the output conversion type is
	 * set to "Automatic".  
	 * The contents of colorSpaceOrDisplay and viewTransform will have different
	 * meanings - based on the return value of this function.  
	 * \param   [out] colorSpaceOrDisplay The MSTR passed in here as an optional
	 *          pointer may receive the name of the color space or the name of
	 *          the display. Please refer to the return value of this function
	 *          on how to interpret this value.   
	 * \param   [out] viewTransform The MSTR passed in here as an optional
	 *          pointer may receive the name of the view transform. Please refer
	 *          to the return value of this function on how to interpret this
	 *          value.  
	 * \return  Returns the default OutputConversion. Based on this value, the
	 *          contents of the passed in MSTR may have different meaning:  
	 *          If this function returns OutputConversion::kNO_CONVERSION, both
	 *          parameters will be ignored.  
	 *          If the return value is
	 *          OutputConversion::kCOLOR_SPACE_CONVERSION, colorSpaceOrDisplay
	 *          will receive the name of the color space and viewTransform will
	 *          be ignored.  
	 *          In case it returns OutputConversion::kDISPLAY_VIEWTRANSFORM,
	 *          colorSpaceOrDisplay will receive the name of the display and
	 *          viewTransform will receive the name of the view transform.  
	 * \see     OutputConversion, SetDefaultOutputConversion */
	virtual OutputConversion GetDefaultOutputConversion(MSTR* colorSpaceOrDisplay = nullptr, MSTR* viewTransform = nullptr) const = 0;
	
	/** Sets the default output conversion. The default output conversion is
	 * getting applied when saving a bitmap and the output conversion type is
	 * set to "Automatic".  
	 * \param   [in] conversion The OutputConversion to be used.  
	 * \param   [in] colorSpaceOrDisplay If the param conversion is set to
	 *          OutputConversion::kCOLOR_SPACE_CONVERSION, this is read as the
	 *          name of the color space to convert to. If the param conversion
	 *          is set to OutputConversion::kDISPLAY_VIEWTRANSFORM, this is read
	 *          as the name of the display, and the viewTransform param is also
	 *          required. If the param conversion is set to
	 *          OutputConversion::kNO_CONVERSION, this parameter is ignored.  
	 * \param   [in] viewTransform If the param conversion is set to
	 *          OutputConversion::kDISPLAY_VIEWTRANSFORM, this is read as the
	 *          name of the view transform. In other conversion modes, this
	 *          parameter is ignored.  
	 * \return  Returns true, if the passed parameter combination is available
	 *          and could be set, false otherwise.  
	 * \see     OutputConversion, GetDefaultOutputConversion */
	virtual bool SetDefaultOutputConversion(OutputConversion conversion, const MSTR& colorSpaceOrDisplay = {}, const MSTR& viewTransform = {}) = 0;
	///@} ----------------------------------------------------------------------

	/** \name Default Display/View Transforms */
	///@{ ----------------------------------------------------------------------
	/** Returns the default Display/View Transform for the given target.  
	 * \param   [in] target The DisplayViewTarget to be queried.  
	 * \param   [out] automatic Will be set to true, if the given target is set
	 *          to automatic. Regardless the automatic mode, display and view
	 *          will be filled with the effective values.  
	 * \param   [out] display The Display for the given target.  
	 * \param   [out] view The View Transform for the given target.  
	 * \return  True, if successful.  
	 * \see     SetDefaultDisplayView, DisplayViewTarget */
	virtual bool GetDefaultDisplayView(DisplayViewTarget target, bool& automatic, MSTR& display, MSTR& view) const = 0;
	
	/** Sets the default Display/View Transform for the given DisplayViewTarget.
	 * \param   [in] target The DisplayViewTarget to be altered.  
	 * \param   [in] automatic If be set to true, if the given target is set to
	 *          automatic. In this case the display and view parameters will be
	 *          ignored.  
	 * \param   [in] display If automatic is set to false, this will be the
	 *          display for the given target.  
	 * \param   [in] view If automatic is set to false, this will be the view
	 *          transform for the given target.  
	 * \return  True, if successful. False, if the given combination of
	 *          display/view-transform isn't available in the current setting.  
	 * \see     SetDefaultDisplayView, DisplayViewTarget */
	virtual bool SetDefaultDisplayView(DisplayViewTarget target, bool automatic, const MSTR& display = {}, const MSTR& view = {}) = 0;
	///@} ----------------------------------------------------------------------

	/** \name Display Functions */
	///@{ ----------------------------------------------------------------------
	/** Returns the number of displays.  
	 * \return  The number of available displays.  
	 * \see     GetDisplay, GetDisplayIndex, GetDefaultDisplay */
	virtual size_t GetNumDisplays() const = 0;

	/** Returns the name of the display with the given index.  
	 * \param   [in] idx The index of the display to be queried.  
	 * \return  The name of the display - or an empty string.  
	 * \see     GetNumDisplays, GetDefaultDisplay */
	virtual MSTR GetDisplay(size_t idx) const = 0;

	/** Returns the index of the given display name.  
	 * \note    Display name comparisons in OCIO are always case-insensitive.  
	 * \param   [in] display the name of the display to be queried.  
	 * \return  The index of the given display, or the \c (size_t)(-1) if the
	 *          given display could not be found.  
	 * \see     GetNumDisplays, GetDisplayIndex, GetDefaultDisplay */
	virtual size_t GetDisplayIndex(const MSTR& display) const = 0;
	
	/** Returns the name of the default display.  
	 * \return  The name of the default display.  
	 * \see     GetNumDisplays, GetDisplay, GetDefaultDisplayView*/
	virtual MSTR GetDefaultDisplay() const = 0;
	///@} ----------------------------------------------------------------------

	/** \name View Functions */
	///@{ ----------------------------------------------------------------------
	// Views applicable to the given display
	/** Returns the number of view transforms available for the given display.  
	 * \note    Display name comparisons in OCIO are always case-insensitive.  
	 * \param   [in] display The display to be queried.  
	 * \return  The number of view transforms available for the given display.  
	 * \see     GetView, GetViewIndex, GetDefaultView */
	virtual size_t GetNumViews(const MSTR& display) const = 0;

	/** Returns the name of the view transform with the given index for the
	 * given display.  
	 * \note    Display name comparisons in OCIO are always case-insensitive.   
	 * \param   [in] display The display to be queried.  
	 * \param   [in] idx The index of the view transform to be retrieved.  
	 * \return  The name of the view transform with the given index for the
	 *          given display.  
	 * \see     GetView, GetViewIndex, GetDefaultView */
	virtual MSTR	GetView(const MSTR& display, size_t idx) const = 0;

	/** Returns the index of the view transform with the given name for the
	 * given display.  
	 * \note    Display name comparisons in OCIO are always case-insensitive.   
	 * \param   [in] display The display to be queried.  
	 * \param   [in] view The name of the view transform to be searched.  
	 * \return  The index of the view transform with the given index for the
	 *          given display, or \c (size_t)(-1) if the given display could not
	 *          be found in the given context.  
	 * \see     GetView, GetViewIndex, GetDefaultView */
	virtual size_t	GetViewIndex(const MSTR& display, const MSTR& view) const = 0; // returns (size_t)(-1) if not found.

	/** Returns the name of the default view transform for the given display.  
	 * \note    Display name comparisons in OCIO are always case-insensitive.   
	 * \param   [in] display The display to be queried.  
	 * \return  The name of the default view transforms for the given display.  
	 * \see     GetView, GetViewIndex, GetDefaultView */
	virtual MSTR	GetDefaultView(const MSTR& display) const = 0;

	/** Returns the number of view transforms available for the given color
	 * space and display.  
	 * \note    Display and %Color Space name comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] sourceColorSpace The %Color Space to be queried.  
	 * \param   [in] display The display to be queried.  
	 * \return  The number of view transforms available for the given display.  
	 * \see     GetView, GetViewIndex, GetDefaultView */
	virtual size_t GetNumViews(const MSTR& sourceColorSpace, const MSTR& display) const = 0;

	/** Returns the name of the idx'th view transform with the given index for
	 * the given display and color space.  
	 * \note    Display and %Color Space name comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] sourceColorSpace The %Color Space to be queried.  
	 * \param   [in] display The display to be queried.  
	 * \param   [in] idx The index of the view transform to be retrieved.  
	 * \return  The name of the view transform with the given index for the
	 *          given display and color space.  
	 * \see     GetNumViews, GetViewIndex, GetDefaultView */
	virtual MSTR GetView(const MSTR& sourceColorSpace, const MSTR& display, size_t idx) const = 0;

	/** Returns the index of the view transform with the given name for the
	 * given display and color space.  
	 * \note    Display and %Color Space name comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] sourceColorSpace The %Color Space to be queried.  
	 * \param   [in] display The display to be queried.  
	 * \param   [in] view The name of the view transform to be searched.  
	 * \return  The index of the view transform with the given index for the
	 *          given display, or \c (size_t)(-1) if the given display could not
	 *          be found in the given context.  
	 * \see		GetNumViews, GetView, GetDefaultView */
	virtual size_t GetViewIndex(const MSTR& sourceColorSpace, const MSTR& display, const MSTR& view) const = 0;

	/** Returns the name of the default view transform for the given display and
	 * color space.  
	 * \note    Display and %Color Space name comparisons in OCIO are always
	 *          case-insensitive.  
	 * \param   [in] sourceColorSpace The %Color Space to be queried.  
	 * \param   [in] display The display to be queried.  
	 * \return  The name of the default view transforms for the given display
	 *          and color space.  
	 * \see     GetNumViews, GetView, GetViewIndex */
	virtual MSTR GetDefaultView(const MSTR& sourceColorSpace, const MSTR& display) const = 0;
	///@} ----------------------------------------------------------------------

};


} // namespace MaxSDK::ColorManagement
