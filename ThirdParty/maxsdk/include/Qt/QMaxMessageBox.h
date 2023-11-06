//-----------------------------------------------------------------------------
// Copyright 2019 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//-----------------------------------------------------------------------------

#pragma once

#include "..\utilexp.h"
#include <wtypes.h>

// forward declarations
class QWidget;
class QString;

namespace MaxSDK
{
	//! \brief Provides an extended message box functionality. This is used to support message dialogs with a 'Hold', a 'Help' button and/or a 'Do not show this message again' check box.
	/*! The first four parameters correspond to the Win32 MessageBox method parameters, but not all Win32 MessageBox functionality is supported.
		The last two optional arguments add the extended functionality - exType is used for adding the additional buttons, and exRet is used for getting the extra return info.
		MaxSDK::MaxMessageBox and MaxSDK::QmaxMessageBox should be used in preference to Win32 MessageBox due to string clipping that can occur with Win32 MessageBox in some locales with 
		some font types and sizes on Windows 10 when dpi scaling is greater than 100%.
		\param parent - The parent QWidget
		\param text - The main message
		\param caption - The caption or title for the message window
		\param type - The type of message box. The supported types are:
		- MB_YESNO
		- MB_OK
		- MB_YESNOCANCEL
		- MB_OKCANCEL
		- MB_ABORTRETRYIGNORE
		- MB_RETRYCANCEL
		- MB_CANCELTRYCONTINUE
		The following Win32 MessageBox flags are supported:
		- MB_DEFBUTTON*
		- MB_SYSTEMMODAL
		- MB_TOPMOST
		The following Win32 MessageBox flags are not supported and will be ignored:
		- MB_TASKMODAL
		- MB_NOFOCUS
		- MB_SETFOREGROUND
		- MB_RIGHT
		- MB_RTLREADING
		The MB_HELP flag is supported, albeit in a manner different than Win32 MessageBox. If specified and 'exRet' is not null, a Help button is displayed and clicking it
		causes 3ds Max Help to be displayed using the help topic id that is passed through 'exRet'.
		The MB_DEFBUTTON* flag applies to the standard message box buttons, and does not apply to the optional Help and Hold buttons
		\param exType - Used for adding additional UI elements. The supported bit field values are:
		- MAX_MB_HOLD - display a 'Hold' button.
		- MAX_MB_DONTSHOWAGAIN - display a 'Do not show this message again' check box
		\param exRet - Used for specifying the help file topic id and for returning extra info:
		- if MAX_MB_DONTSHOWAGAIN is specified as part of 'exType', on return the MAX_MB_DONTSHOWAGAIN bit will or will not be set in 'exRet' based on check box setting.
		- if MAX_MB_HOLD is specified as part of 'exType', if the button is pressed the MAX_MB_HOLD bit will set in 'exRet' and a value of IDOK is returned.
		\note See "https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-messagebox" for a description of the Win32 MessageBox method parameters.
		*/
	UtilExport int QmaxMessageBox(QWidget* parent, const QString& text, const QString& caption, unsigned int type, unsigned int exType = 0, DWORD *exRet = nullptr);
}