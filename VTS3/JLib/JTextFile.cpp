
#include "stdafx.h"

#include "JConfig.hpp"
#include "JError.hpp"
#include "JTextFile.hpp"

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JTextFile::JTextFile
//

JTextFile::JTextFile( int bufSize, JFileEOL eol )
#if Mac_Platform
	: JFile( 'TEXT' , '????' )
#elif UNIX_Platform
	: JFile()
#endif
	, textBuffSize( bufSize ), textPos(0), textEOL(eol)
{
	ThrowIfMemFail_( textBuff = new char[bufSize] );
}

//
//	JTextFile::~JTextFile
//

JTextFile::~JTextFile( void )
{
	delete[] textBuff;
}

//
//	JTextFile::Open
//

void JTextFile::Open( void )
{
	textPos = 0;
	JFile::Open();
}

//
//	JTextFile::Read
//

int JTextFile::Read( char *data )
{
	int		stat, fsize = Size()
	;
	char	*src, *dst
	;
	
	// check for end-of-file
	if (textPos >= fsize)
		return -1;
	
	// read a chunk of data
	stat = JFile::Read( textPos, textBuffSize, textBuff);
	if (stat && (stat != EOF))	// ### some system didn't like the last bit
		return stat;
	
	// null terminate the end of the file
	if ((textPos + textBuffSize) >= fsize)
		textBuff[ fsize - textPos ] = 0;
	
	// copy to user buffer or scan
	if (data) {
		src = textBuff;
		dst = data;
		while (*src && (*src != '\n') && (*src != '\r')) {
			*dst++ = *src++;
			textPos += 1;
		}
		*dst = 0;
	} else {
		src = textBuff;
		while (*src && (*src != '\n') && (*src != '\r')) {
			src += 1;
			textPos += 1;
		}
	}
	
	// skip returns (Mac or DOS)
	if (*src == '\r') {
		*src++ = 0;
		textPos += 1;
	}
	
	// skip line feeds (Unix or DOS)
	if (*src == '\n') {
		*src++ = 0;
		textPos += 1;
	}
	
	return 0;
}

//
//	JTextFile::Write
//

int JTextFile::Write( char *data )
{
	int		stat
	;
	char	*eor
	;
	
	// copy user data
	eor = textBuff;
	if (data)
		while (*data)
			*eor++ = *data++;
	else
		while (*eor)
			eor++;
	
	// put in the correct end-of-line sequence
	switch (textEOL) {
		case eNativeEOL:
#if (Mac_Platform || DOS_Platform)
			*eor++ = '\r';
#endif
#if (UNIX_Platform || DOS_Platform)
			*eor++ = '\n';
#endif
			break;
		case eMacintoshEOL:
			*eor++ = '\r';
			break;
		case ePCEOL:
			*eor++ = '\r';
			*eor++ = '\n';
			break;
		case eUnixEOL:
			*eor++ = '\n';
			break;
	}
	
	// save the data
	stat = JFile::Write( textPos, (eor - textBuff), textBuff );
	if (stat)
		return stat;
	
	// update the file position
	textPos += (eor - textBuff);
	
	// success
	return 0;
}

//
//	JTextFile::GetPos
//

int JTextFile::GetPos( long &pos )
{
	pos = textPos;
	
	return 0;
}

//
//	JTextFile::SetPos
//

int JTextFile::SetPos( long pos )
{
	textPos = pos;
	
	return 0;
}
