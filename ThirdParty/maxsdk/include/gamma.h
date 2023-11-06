/**********************************************************************
 *<
	FILE:  gamma.h

	DESCRIPTION:  Gamma utilities

	CREATED BY: Dan Silva

	HISTORY: created 26 December 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "maxtypes.h"
#include "coreexp.h"
#include "geom/color.h"

#define WRDMAX 65535
#define FWRDMAX 65535.0f

#define RCBITS 13   // number of bits used to represent colors before gamma correction.
					// this keeps the lookup table a reasonable size
#define RCOLN (1<<RCBITS)    
#define RCMAX (RCOLN-1)	
#define FRCMAX ((float)RCMAX) 
#define RCHALF (RCOLN>>1)
#define RCSH (RCBITS-8)		  /* shift amount from 8 bit to RCBITS */
#define RCSH16 (16-RCBITS)	  /* shift amount from 16 bit to RCBITS */
#define RCFRACMASK ((ulong)((1<<RCSH)-1))	  
#define RC_SCL (1<<RCSH)
#define RC_SCLHALF (1<<(RCSH-1))
#define FRC_SCL ((float)RC_SCL)
#define RCSHMASK (0xffffffffL<<RCSH)
#define RCSHMAX (0xffL<<RCSH)


#define GAMMA_NTSC		2.2f
#define	GAMMA_PAL		2.8f

/** \deprecated This class is deprecated: Please use \ref IColorPipelineMgr which
provides better color management functionality.  */
/*! \sa Class Color, <a href="https://msdn.microsoft.com/en-us/library/dd183449(v=vs.85).aspx">COLORREF</a>.\n\n
\par Description:
Deprecated gamma manager class: Please use \ref IColorPipelineMgr which
provides better color management functionality.
Methods of this class are used to gamma correct and
de-gamma colors in various formats. Various settings from the 3ds Max user
interface are also accessible via data members of this class (for example the
display, and file gamma settings). These settings may be read but should not be
set by a plug-in developer. All methods of this class are implemented by the
system.\n\n
There is a global instance of this class (defined in
<b>/MAXSDK/INCLUDE/GAMMA.H</b>):\n\n
<b>GammaMgr gammaMgr;</b>\n\n
Note the following \#defines. These are used to reduce the size of the gamma
tables for correcting 16 bit values.\n\n
\code
#define RCBITS 13
#define RCOLN (1<<RCBITS)
\endcode
This class provides a set of commonly used gamma tables. This class does not
provide tables for all types of conversion however. For example if you have a
different gamma setting that you are using, or if you are going in a different
conversion direction than the tables provided here you may use the classes
GamConvert16 and
GamConvert8 to build gamma tables.
\par Data Members:
<b>BOOL enable;</b>\n\n
Indicates if gamma correction is enabled or disabled.\n\n
<b>BOOL dithTrue;</b>\n\n
Indicates if output dithering is to be used for true color images.\n\n
<b>BOOL dithPaletted;</b>\n\n
Indicates if output dithering is to be used for paletted images.\n\n
<b>float dispGamma;</b>\n\n
The display gamma setting.\n\n
<b>float fileInGamma;</b>\n\n
The file input gamma setting.\n\n
<b>float fileOutGamma;</b>\n\n
The file output gamma setting.\n\n
<b>UBYTE disp_gamtabw[RCOLN];</b>\n\n
Display gamma table (RCBITS-\>8).\n\n
<b>UWORD file_in_degamtab[256];</b>\n\n
For de-gamifying bitmaps on input. (8-\>16)\n\n
<b>UWORD file_out_gamtab[RCOLN];</b>\n\n
Gamma correct for file output, before dither (RCBITS-\>16).  */
class GammaMgr: public MaxHeapOperators 
{
	public:
		BOOL dithTrue;
		BOOL dithPaletted;  
		
		UBYTE disp_gamtabw[RCOLN];    // (RCBITS->8) display gamma 
		UWORD file_in_degamtab[256];  // (8->16)  for de-gamifying bitmaps on input
		UWORD file_out_gamtab[RCOLN]; // (RCBITS->16) gamma correct for file output, before dither 

		/*! \remarks Gamma corrects the specified color using the display
		gamma setting.
		\par Parameters:
		<b>Color c</b>\n\n
		The color to gamma correct.
		\return  The gamma corrected color. */
		CoreExport Color DisplayGammaCorrect(Color c);
			
		/*! \remarks Sets the gamma correction enabled setting.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to enable; FALSE to disable. */
		CoreExport void Enable(BOOL onOff);
		/*! \remarks Returns the gamma correction enabled setting; TRUE if
		enabled; FALSE if disabled. */
		BOOL IsEnabled() const { return enable;}

		/*! \remarks Sets the display gamma setting.
		\par Parameters:
		<b>float gam</b>\n\n
		The value to set. */
		CoreExport void  SetDisplayGamma(float gam);
		/*! \remarks Returns the display gamma setting. */
		float GetDisplayGamma() const { return dispGamma; }

		/*! \remarks Sets the file input gamma setting.
		\par Parameters:
		<b>float gam</b>\n\n
		The value to set. */
		CoreExport void SetFileInGamma(float gam);
		/*! \remarks Returns the file input gamma setting. */
		float GetFileInGamma() const { return fileInGamma; }

		/*! \remarks Sets the file output gamma setting.
		\par Parameters:
		<b>float gam</b>\n\n
		The value to set. */
		CoreExport void SetFileOutGamma(float gam);
		/*! \remarks Returns the file output gamma setting. */
		float GetFileOutGamma() const { return fileOutGamma; }

		/*! \remarks Constructor. */
		CoreExport GammaMgr();

		/*! \remarks Destructor. */
		CoreExport ~GammaMgr();
private:
		BOOL enable			= TRUE;
		float dispGamma		= 2.2f;
		float fileInGamma	= 2.2f;
		float fileOutGamma	= 2.2f;
	};

CoreExport extern GammaMgr gammaMgr;

#define GAMMA16to8(b)  gammaMgr.disp_gamtabw[b>>RCSH16]

// Build a Gamma table that maps 8->16
/*! \remarks Builds a gamma table that maps 8-\>16.
\par Parameters:
<b>UBYTE gamtab[256]</b>\n\n
The table to build.\n\n
<b>float gamma</b>\n\n
The gamma setting.\n\n
<b>int onoff=TRUE</b>\n\n
TRUE to enable; FALSE to disable. */
CoreExport void BuildGammaTab8(UWORD gamtab[256], float gamma, int onoff=TRUE);

// Build Gamma table that maps RCBITS->8
/*! \remarks Build a gamma table that maps RCBITS-\>8.
\par Parameters:
<b>UBYTE gamtab[RCOLN]</b>\n\n
The table to build.\n\n
<b>float gamma</b>\n\n
The gamma setting.\n\n
<b>int onoff=TRUE</b>\n\n
TRUE to enable; FALSE to disable. */
CoreExport void BuildGammaTab(UBYTE gamtab[RCOLN], float gamma, int onoff=TRUE);

// Build Gamma table that  maps RCBITS->16
/*! \remarks Build a gamma table that maps RCBITS-\>16.
\par Parameters:
<b>UWORD gamtab[RCOLN]</b>\n\n
The table to build.\n\n
<b>float gamma</b>\n\n
The gamma setting.\n\n
<b>int onoff=TRUE</b>\n\n
TRUE to enable; FALSE to disable. */
CoreExport void BuildGammaTab(UWORD gamtab[RCOLN], float gamma, int onoff=TRUE);

/*! \remarks Gamma corrects the value passed using the specified gamma
setting.
\par Parameters:
<b>float v</b>\n\n
The value to gamma correct.\n\n
<b>float gamma</b>\n\n
The gamma setting.
\return  The gamma corrected value. */
CoreExport float gammaCorrect(float v, float gamma);

/*! \remarks De-gamma corrects the value passed using the specified gamma
setting.
\par Parameters:
<b>float v</b>\n\n
The value to de-gamma correct.\n\n
<b>float gamma</b>\n\n
The gamma setting. */
CoreExport float deGammaCorrect(float v, float gamma);

/*! \remarks Gamma corrects the value passed using the specified gamma
setting.
\par Parameters:
<b>UWORD c</b>\n\n
The value to gamma correct.\n\n
<b>float gamma</b>\n\n
The gamma setting.
\return  The gamma corrected value. */
CoreExport UWORD gammaCorrect(UWORD c, float gamma);

/*! \remarks De-gamma corrects the value passed using the specified gamma
setting.
\par Parameters:
<b>UWORD c</b>\n\n
The value to de-gamma correct.\n\n
<b>float gamma</b>\n\n
The gamma setting. */
CoreExport UWORD deGammaCorrect(UWORD c, float gamma);


// Temporary table for converting 16->16.
/*! \sa  Class GammaMgr, Class GamConvet8.\n\n
\par Description:
A temporary table for converting 16-\>16. A developer may define an instance of
this class and it will build a gamma correction table. The constructor will
build the table with the specified gamma setting and the destructor will free
the table. All methods of this class are implemented by the system.  */
class GamConvert16: public MaxHeapOperators {
	float gamma;
	UWORD* gtab;
	public:	
		/*! \remarks Constructor. The gamma table is built using the specified
		gamma setting. */
		GamConvert16(float gam=1.0f);  
		/*! \remarks Destructor. The gamma table is deleted. */
		~GamConvert16();  
		/*! \remarks Sets the gamma setting to the value specified and builds
		the gamma table.
		\par Parameters:
		<b>float gam</b>\n\n
		The gamma value to set. */
		void SetGamma(float gam);
		/*! \remarks Gamma corrects the specified color.
		\par Parameters:
		<b>UWORD v</b>\n\n
		The color to gamma correct.
		\return  The gamma corrected value. */
		UWORD Convert(UWORD v) { return gtab[v>>RCSH16]; }

	};

// Temporary table for converting 8->16.
/*! \sa  Class GammaMgr, Class GamConvert16.\n\n
\par Description:
A temporary table for converting 8-\>16. A developer may define an instance of
this class and it will build a gamma correction table. The constructor will
build the table with the specified gamma setting and the destructor will free
the table. All methods of this class are implemented by the system.  */
class GamConvert8: public MaxHeapOperators {
	float gamma;
	UWORD gtab[256];
	public:	
		/*! \remarks Constructor. The gamma table is built using the specified
		gamma setting. */
		GamConvert8(float gam=1.0f);  
		/*! \remarks Sets the gamma setting to the value specified and builds
		the gamma table.
		\par Parameters:
		<b>float gam</b>\n\n
		The gamma value to set. */
		void SetGamma(float gam);
		/*! \remarks Gamma corrects the specified color.
		\par Parameters:
		<b>UBYTE v</b>\n\n
		The color to gamma correct.
		\return  The gamma corrected value. */
		UWORD Convert(UBYTE v) { return gtab[v]; }

	};

