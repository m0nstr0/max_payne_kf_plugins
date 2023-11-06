//
// Copyright [2010] Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "streams.h"

invisible_class (ReadonlyTextFileStream)

/**
 * ReadonlyTextFileStream is only used for opening text file in read-only 
 * mode. It's equivalent to use FileStream when opening files in "rt" mode. 
 * But FileStream is very slow when reading files. So we optimize the original
 * codes in FileStream and create this new class which is thousand times faster
 * for reading text files than FileStream.
 * \note Invoke any non-read-only methods in ReadonlyTextFileStream will result
 *		 in a runtime exception. Such methods include putch(),puts(),printf().
 */
class ReadonlyTextFileStream : public CharStream
{
public:
	/**
	* The default constructor.
	*/
	ScripterExport ReadonlyTextFileStream();
	/**
	* The default destructor.
	*/
	~ReadonlyTextFileStream();

	/**
	* Open a new text file.
	* \param[in]  ifile_name The file name.
	* \param[in]  encoding The encoding to use if if can not be determined from the file contents, If -1, a default
	*			encoding based on the file language setting in Preference /Files (exposed through Interface14) will
	*			be used.
	* \return The ReadonlyTextFileStream instance itself if max is able to open the file or undefined if it fails.
	*/
	ScripterExport ReadonlyTextFileStream* open(const MCHAR* ifile_name, unsigned int encoding = -1);

	/* internal char stream protocol */
	MaxSDK::Util::Char get_char  () override;
	void               unget_char(const MaxSDK::Util::Char& c) override;
	MaxSDK::Util::Char peek_char () override;

	int		at_eos() override;
	unsigned int	pos() override;
	void	seek(int pos) override;
	void	rewind() override;
	void	flush_to_eol() override;
	MaxSDK::Util::Char	putch(const MaxSDK::Util::Char& c) override;
	const MCHAR*	puts(const MCHAR* str) override;
	int		printf(const MCHAR *format, ...) override;

	void	collect() override;
	void	close() override;

	void	gc_trace() override;
	void	log_to(CharStream* log) override;
	void	close_log() override;
	CharStream*	get_log() override;

private:
	void VerifyFileOpen() const;
	MaxSDK::Util::Char& CharAtPos(size_t pos);

protected:
	int                 mUngetcharCount;
	MaxSDK::Util::Char  mUngetcharBuf[8];
	MaxSDK::Util::Char* mFileBuf;
	size_t              mCharCount;
	size_t              mCharPos;
	CharStream*         mLog;
};
