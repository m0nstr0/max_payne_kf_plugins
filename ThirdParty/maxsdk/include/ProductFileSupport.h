/*****************************************************************************
* FILE:          ProductFileSupport.h
* DESCRIPTION:  Defines symbols used to support files saved by different 
*                vertical apps. 
* CREATED BY:    Michael Russo
* HISTORY:  
* - 2003.oct.14 - AS - Pulled them out in their own file
******************************************************************************
* Copyright (c) 2003 Autodesk, All Rights Reserved.
******************************************************************************/

#pragma once

#include "buildver.h"

// File format support tokens
#define FILE_SUPPORT_NONE       0x000
#define FILE_SUPPORT_OPEN       0x001
#define FILE_SUPPORT_SAVE       0x002
#define FILE_SUPPORT_DEFAULT    0x004
//! \deprecated Deprecated as of 3ds Max 2024. Unsupported for many versions.
#pragma deprecated("FILE_SUPPORT_EXPORT")
//! \deprecated Deprecated as of 3ds Max 2024. Unsupported for many versions.
#pragma deprecated("FILE_SUPPORT_IMPORT")
#define FILE_SUPPORT_MERGE      0x020
#define FILE_SUPPORT_REPLACE    0x040
#define FILE_SUPPORT_XREF       0x080
#define FILE_SUPPORT_RPS_OPEN   0x100
#define FILE_SUPPORT_RPS_SAVE   0x200
// support for material libraries
#define FILE_SUPPORT_MAT_OPEN   0x400
#define FILE_SUPPORT_MAT_SAVE   0x800

#define FILE_SUPPORT_NATIVE  (FILE_SUPPORT_SAVE | FILE_SUPPORT_OPEN | \
                              FILE_SUPPORT_MERGE | FILE_SUPPORT_REPLACE | \
                              FILE_SUPPORT_XREF | FILE_SUPPORT_DEFAULT)

// File formats

#define FILE_FORMAT_MAX       FILE_SUPPORT_NATIVE
#define FILE_FORMAT_RPS		  (FILE_SUPPORT_RPS_OPEN|FILE_SUPPORT_RPS_SAVE)
#define FILE_FORMAT_CHR       (FILE_SUPPORT_SAVE|FILE_SUPPORT_OPEN|FILE_SUPPORT_MERGE|FILE_SUPPORT_REPLACE|FILE_SUPPORT_XREF)
#define FILE_FORMAT_MAT		  (FILE_SUPPORT_MAT_OPEN|FILE_SUPPORT_MAT_SAVE)
