//**************************************************************************/
// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Max Script File streams
// AUTHOR:      Richard Houle
// DATE:        April 29 2011
//***************************************************************************/

#ifndef __MAXSCRIPT_STREAMFILE_H__
#define __MAXSCRIPT_STREAMFILE_H__

#include "../../maxbinarystream.h"
#include "../../maxtextfile.h"
#include "CharStream.h"

class CharBinaryStream : public CharStream
{
protected:
	MaxSDK::Util::BinaryStream*   stream;
	MaxSDK::Util::TextFile::BaseTextReader* textReader;
	MaxSDK::Util::TextFile::BaseTextWriter* textWriter;

public:
	CharBinaryStream();
	virtual ~CharBinaryStream();

public:
	/**
	* Open a text file
	* \param[in]  pStream The binary stream that holds the file.
	* \param[in]  mode The file open mode. See the MAXScript Help file, topic FileStream Values, for a description of the mode string values.
	* \param[in]  encoding The encoding to use if if can not be determined from the file contents, If -1, a default
	*			encoding based on the file language settings in Preference /Files (exposed through Interface14) will
	*			be used.
	* \return false if pStream is null or this instance already has an open stream, true otherwise.
	*/
	bool Open(MaxSDK::Util::BinaryStream *pStream, const MSTR& mode, unsigned int encoding = -1);

public:
 	MaxSDK::Util::Char get_char  () override;
 	void               unget_char(const MaxSDK::Util::Char& c) override;
 	MaxSDK::Util::Char peek_char () override;

public:
 	int		at_eos() override;
 	void	rewind() override;
	void	seek(int pos) override;

	unsigned int pos() override;
	unsigned int line() override;

public:
 	void flush() override;
	void flush_to_eol() override;

public:
 	MaxSDK::Util::Char putch (const MaxSDK::Util::Char& c) override;
	const MCHAR*       puts  (const MCHAR* str) override;
 	int                printf(const MCHAR* format, ...) override;

public:
 	void close() override;

public:
	bool IsEncrypted() const;

public:
	void check_file_is_open() const;
	void check_readable() const;
	void check_writeable() const;
	void check_is_not_eof() const;

protected:
	virtual void ThrowNotOpenedError() const;
	virtual void ThrowNotReadableError() const;
	virtual void ThrowNotWritableError() const;
	virtual void ThrowReachEOFError() const;
};

#endif
