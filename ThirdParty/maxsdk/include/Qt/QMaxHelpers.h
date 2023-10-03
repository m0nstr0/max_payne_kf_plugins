//-----------------------------------------------------------------------------
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//-----------------------------------------------------------------------------

#pragma once

#include "../CoreExport.h"
#include "QtMax.h"

#pragma warning(push)
#pragma warning(disable: 4127 4251 4275 4512 4800 )
#include <QtWidgets/QtWidgets>
#include <QtCore/QObject>
#pragma warning(pop)

class Bitmap;


namespace QtHelpers
{

	////////////////////////////////////////////////////////////////////////////
	// Image functions.

	//Takes the name of the icon to load directory/iconName.png. It loads all versions of the 
	//icon, example: directory/iconName_XX.png, directory/iconName_on_XX.png where XX is the size
	//of the icon.
	//Input Parameters: iconName - The name of the icon to load.
	//OutputParameters: numIcons- the number of elements in the returned array.
	//Return Value    : An array of HIcon containing the HIcons of the loaded icons. Must be destroyed 
	//                  by the caller function.
	//Note: The returned HICON array must be destroyed by the user. Also, the user has to call DestroyIcon
	//      to destroy the icon when not used anymore.
	MaxQTExport HICON* CreateHIconsFromMultiResIcon(const MCHAR* iconName, int& numIcons);

	// Convert a 3ds Max SDK bitmap to a Qt icon.
	MaxQTExport QIcon QIconFromBitmap( Bitmap* bm );

	// Convert a Qt icon to a 3ds Max SDK bitmap.
	MaxQTExport Bitmap* BitmapFromQIcon( const QIcon& icon, int width, int height, QIcon::Mode mode = QIcon::Mode::Normal, QIcon::State state = QIcon::State::Off );

	// Convert a Qt icon to a HBITMAP.
	MaxQTExport HBITMAP HBITMAPFromQIcon( const QIcon& icon, int width, int height, QIcon::Mode mode = QIcon::Mode::Normal, QIcon::State state = QIcon::State::Off );

	/* \brief Associates a help context identifier with a given widget.
	* Equivalent to the windows method SetWindowContextHelpId().
	* Use this function for registering a help context id on a qwidget without 
	* making it native win32 window.
	* \param widget The widget with which the help context id should be associated.
	* \param contextId The Help context identifier.
	*/
	MaxQTExport void SetWindowContextHelpId( QWidget* widget, int contextId );

	/* \brief Disables 3ds Max keyboard accelerators for a specified widget when it or a child widget receives
	* focus. Once the widget gets a focusOut event the 3ds Max accelerators are re-enabled.
	* If you don't need any 3ds Max accelerators at all when the focus is in your dialog, you can disable the
	* accelerators at your top level widget hierarchy, which is most likely your dialog. You can also be more
	* specific and just disable the accelerators on the specific Qt control that requires keyboard input.
	* For some Qt input controls (such as QLineEdit and QSpinBox) this mechanism is set up
	* automatically on UI creation by 3ds Max.
	* \note These Qt input controls have this mechanism set up automatically: QLineEdit, QSpinBox, QDoubleSpinBox,
	* QComboBox, QTableWidget, QListView, QTreeView, QTextEdit, QPlainTextEdit, QCheckBox, QRadioButton.
	* \note There is no need to call DisableMaxAcceleratorsOnFocus(widget, false) before the widget gets destroyed.
	* \param widget The widget on which the 3ds max keyboard accelerators should be disabled when it or child widget
	* receives the focus.
	* \param disableMaxAccelerators Pass true if 3ds Max should disable accelerators on focusIn, otherwise false.
	*/
	MaxQTExport void DisableMaxAcceleratorsOnFocus( QWidget* widget, bool disableMaxAccelerators = true );

}; // end of QtHelpers namespace -------------------------------------------------
