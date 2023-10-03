//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2022 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../maxstring.h"
#include "../utilexp.h"

namespace MaxSDK { namespace Util { 

/**
 * Opens a file and sets its buffer size.
 *
 * This class is designed as a helper when dealing with reading/writing potentially large files. When opening 
 * a file, a default buffer size of 4KB is used. When reading or writing the file, the operations occur in 4KB
 * chunks. For large files, such as bitmap files, this will result in a very large number of file operations.
 * If the file operations have a high latency, such as dealing with a slow network or operating in a cloud
 * environment, this can result in very poor performance.
 * 
 * The file is closed in the destructor.
 * 
 * Files opened using this class will have a buffer size of, depending on options used, an environment variable value,
 * an ini file file value, or a specified value that defaults to 256KB.
 */
class UtilExport BufferedFile : public MaxHeapOperators
{
public:
	// Constants related to the file buffer size used when reading/writing buffered files.
	// If environment variable is specified, its value is used.
	// If environment variable is not specified but ini file setting is specified,
	// its value is used.
	// Otherwise, DEFAULT_BUFFEREDFILE_BUFFER_SIZE is used.
	//
	// Value is buffer size in bytes.
	//
	// If specified value is negative, a buffer size value of 4GB is used.
	// In not opening the file for write, the buffer size value will be clamped to the file size.

	// The environment variable name for the file buffer size.
	static constexpr MCHAR ENV_BUFFEREDFILE_BUFFER_SIZE[] = _T("ADSK_3DSMAX_BUFFEREDFILE_BUFFERSIZE");
	// The 3dsmax.ini key name for the file buffer size, section 'Performance"
	static constexpr MCHAR INI_KEY_BUFFEREDFILE_BUFFER_SIZE[] = _T("BufferedFileBufferSize");
	// The default file buffer size.
	static constexpr DWORD DEFAULT_BUFFEREDFILE_BUFFER_SIZE = (1 << 18);

	/** Constructor */
	BufferedFile() = default;

	/**
	* Constructor
	 *
	 * \param fileName The name of the file to open
	 * \param mode The mode to open the file in. This is the same mode specifier as win32 api fopen.
	 * \param setBufferSize Whether to set buffer size. If false, the default win32 4KB buffer is used
	 * \param useEnvAndIniSettings - whether to potentially override defaultBufferSize value with environment variable /
	 * ini file setting
	 * \param defaultBufferSize The default buffer size in bytes
	 */
	BufferedFile(const MCHAR* fileName, const MCHAR* mode, bool setBufferSize, bool useEnvAndIniSettings = true,
			DWORD defaultBufferSize = DEFAULT_BUFFEREDFILE_BUFFER_SIZE);

	/** Destructor */
	virtual ~BufferedFile();

	/**
	 * Open a file
	 *
	 * \param fileName The name of the file to open
	 * \param mode The mode to open the file in. This is the same mode specifier as win32 api fopen.
	 * \param setBufferSize Whether to set buffer size. If false, the default win32 4KB buffer is used
	 * \param useEnvAndIniSettings - whether to potentially override defaultBufferSize value with environment variable /
	 * ini file setting \param defaultBufferSize The default buffer size in bytes
	 * \return Returns true if file was opened
	 */
	bool Open(const MCHAR* fileName, const MCHAR* mode, bool setBufferSize, bool useEnvAndIniSettings = true,
			DWORD defaultBufferSize = DEFAULT_BUFFEREDFILE_BUFFER_SIZE);

	/** Close the file */
	void Close();

	/** Get the FILE* */
	FILE* getFile() const; 

	/**
	 * Calculate the buffer size to use when opening the file
	 *
	 * \param fileName The name of the file to open
	 * \param openingForWrite True if opening file for write. If opening for read, the buffer size will be clamped to the file size
	 * \param useEnvAndIniSettings - whether to potentially override defaultBufferSize value with environment variable /
	 * ini file setting \param defaultBufferSize The default buffer size in bytes
	 * \return Returns the buffer size to use.
	 * \note The returned buffer size value should be passed to win32 api function setvbuf, testing return value. If fails, fall back 
	 * and use DEFAULT_BUFFEREDFILE_BUFFER_SIZE as buffer size.
	 */
	static DWORD CalculateBufferSize(const MCHAR* fileName, bool openingForWrite, bool useEnvAndIniSettings = true,
			DWORD defaultBufferSize = DEFAULT_BUFFEREDFILE_BUFFER_SIZE);

	/**
	 * Open a file
	 *
	 * \param fileName The name of the file to open
	 * \param mode The mode to open the file in. This is the same mode specifier as win32 api fopen.
	 * \param setBufferSize Whether to set buffer size. If false, the default win32 4KB buffer is used
	 * \param useEnvAndIniSettings - whether to potentially override defaultBufferSize value with environment variable /
	 * ini file setting \param defaultBufferSize The default buffer size in bytes
	 * \return Returns the FILE* if opened, nullptr if not.
	 */
	static FILE* OpenFile(const MCHAR* fileName, const MCHAR* mode, bool setBufferSize,
			bool useEnvAndIniSettings = true, DWORD defaultBufferSize = DEFAULT_BUFFEREDFILE_BUFFER_SIZE);

protected:
	FILE* stream = nullptr;
};

}}
