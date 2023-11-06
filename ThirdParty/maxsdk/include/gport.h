/**********************************************************************
 *<
	FILE: gport.h

	DESCRIPTION: Palette management.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include <WTypes.h>
#include "maxtypes.h"
#include "geom/color.h"
#include "geom/box2.h"


/** Deprecated utility class for managing user interface colors
\deprecated This class is deprecated and will be removed in the future. 
\sa  Advanced Topics on
~{ Color Palettes }~,
Class Color, Class GammaMgr, <a href="https://msdn.microsoft.com/en-us/library/dd183449(v=vs.85).aspx">COLORREF</a>.\n\n
\par Description:
A useful utility class for managing user interface colors. This class has
several purposes:\n\n
* Maintain the default 3ds Max palette for doing 256 color graphics.\n\n
* Provides a mechanism for allocating "animated color slots" in the default
palette for use in the user interface.\n\n
* Provide various functions for doing dithered graphics using the default 3ds
Max palette.\n\n
All methods of this class are implemented by the system.\n\n
The following global function is used to get a pointer that may be used to call
the methods of this class: */
class GPort: public MaxHeapOperators {
	public:
		/*! \remarks Destructor */
		virtual ~GPort() {;}

		// Display an array of 24bit colors in the HDC: if the current display is 8 bit
		//  it will display it (with dither) using in the GPort palette, otherwise it 
		//  will just blit to the screen. Does NOT do gamma correction.
		//   "drect" is the destination rectangle in the hdc.
		//   "map" points to an array of RGB triples, with bytesPerRow bytes on each scanline.
		//   "xsrc" and "ysrc" are the position within this source raster of the upper left
		//    corner of the rectangle to be copied..
		/*! \remarks Display an array of 24 bit colors in the HDC. If the
		current display is 8 bit it will display it (with dither) using in the
		GPort palette, otherwise it will just blit to the screen. This method
		does NOT do gamma correction.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.\n\n
		<b>Rect\& drect</b>\n\n
		The destination rectangle in the hdc.\n\n
		<b>int xsrc</b>\n\n
		The X position within this source raster of the upper left corner of
		the rectangle to be copied.\n\n
		<b>int ysrc</b>\n\n
		The Y position within this source raster of the upper left corner of
		the rectangle to be copied.\n\n
		<b>UBYTE *map</b>\n\n
		Points to an array of BGR triples.\n\n
		<b>int bytesPerRow</b>\n\n
		The number of bytes per row on each scanline of <b>map</b>. */
		//[[deprecated("Please use StretchDIBits() directly")]]
		virtual void DisplayMap(HDC hdc, Rect& drect,int xsrc, int ysrc, UBYTE *map, int bytesPerRow)=0;
		
		// This version stretches the image (if src!=dest).
		//  "dest" is the destination rectangle in the hdc;
		//  "src" is the source rectangle in map.
		/*! \remarks This version of <b>DisplayMap()</b> stretches the image
		if the source rectangle is not the same size as the destination
		rectangle. <b>src</b> should be the size of the image.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.\n\n
		<b>Rect\& dest</b>\n\n
		The destination rectangle in the <b>hdc</b>.\n\n
		<b>Rect\& src</b>\n\n
		The source rectangle in <b>map</b>.\n\n
		<b>UBYTE *map</b>\n\n
		Points to an array of RGB triples.\n\n
		<b>int bytesPerRow</b>\n\n
		The number of bytes per row on each scanline of <b>map</b>. */
		//[[deprecated("Please use StretchDIBits() directly")]]
		virtual void DisplayMap(HDC hdc, Rect& dest, Rect& src, UBYTE* map, int bytesPerRow) = 0; 

		// FillColorSwatch
		// fills the given rectangle with the passed color. if transformColors is true, display gamma correction will be applied,
		// otherwise the color is assumed to be in display color space already.
		//TODO ColorManagement: add full color space support (or move elsewhere). /coz
		virtual void FillColorSwatch(HDC hdc, const Rect& rect, const Color color, bool transformColors=true) = 0;
	};

// Normally this is the only one of these, and this gets you a pointer to it.
/*! \remarks There is only a single global instance of this class, and this
method returns a pointer to it. A developer may use this pointer to call the
methods of <b>GPort</b>.*/
extern CoreExport GPort* GetGPort();



