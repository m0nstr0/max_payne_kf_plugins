//**************************************************************************/
// Copyright (c) 2017 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// AUTHOR: Tristan Lapalme
//***************************************************************************/

#pragma once

#ifdef NOTIFICATION_API_UTILS_MODULE
#define NotificationAPIUtilsExport __declspec(dllexport)
#else
#define NotificationAPIUtilsExport __declspec(dllimport)
#endif

#include "../strclass.h"
#include "../maxapi.h"

namespace MaxSDK
{;

/** 
* This namespace exposes 4 private internal calls to the 3dsmax application.
* Useful for Active Shade renderers to keep track of the active view.
*/	
namespace NotificationAPIUtils
{;
	/** 
	* Gets Active ViewExp considering extended and non extended views.
    * \param outIsAnExtendedView will be true if the view is an extended view.
    * \return the ViewExp* for the active viewport
	*/
	NotificationAPIUtilsExport ViewExp* GetActiveViewExpIncludingExtendedViews(bool& outIsAnExtendedView);
	
	/**
	* Gets the ViewExp from its view undoID considering extended views and all tab panels not only the active tab panel.
	* \param viewUndoID the view undo ID
	* \param outIsAnExtendedView will be true if the view is an extended view.
    * \return the ViewExp* for the viewport whose undoId is viewUndoID
	*/	
	NotificationAPIUtilsExport ViewExp* GetViewExpFromUndoIDIncludingExtendedViews(int viewUndoID, bool& outIsAnExtendedView);
	
	/**
	* Gets the viewport label from its view undoID considering extended views and all tab panels not only the active tab panel.
	* \param viewUndoID the view undo ID
	* \param outViewportLabel the viewport label.
	*/
	NotificationAPIUtilsExport void GetViewportLabelFromUndoIDIncludingExtendedViews(int viewUndoID, MSTR& outViewportLabel);
	
	/**
	* To know if the renderer is locked
	* \param renderSettingID the render setting ID
    * \return true if it is locked to a given viewport
	*/
	NotificationAPIUtilsExport bool IsUsingActiveView(RenderSettingID renderSettingID);
};

};//end of namespace MaxSDK