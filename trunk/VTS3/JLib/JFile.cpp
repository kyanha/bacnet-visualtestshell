
#include "stdafx.h"

#include "JConfig.hpp"
#include "JError.hpp"
#include "JFile.hpp"

#if _JFileDebug
#include <iostream>
#endif

#if Mac_Platform
static Point	SFGwhere = { 90, 82 };
static Point	SFPwhere = { 106, 104 };
static SFReply	reply;
void JFilePStrCpy(unsigned char *dst, unsigned char *src);

#include "MoreFiles.h"
#include "MoreFilesExtras.h"
#endif

#if UNIX_Platform
#include <stdio.h>

#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif
#endif

#if NT_Platform
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//
//	JFile::JFile
//

#if Mac_Platform
JFile::JFile( long fType, long fCreator )
	: refNum(0), vrefNum(0)
	, fileType( fType ), fileCreator( fCreator )
	, fileShared( false )
{
}
#endif

#if UNIX_Platform
JFile::JFile(void)
	: filePtr(0)
{
}
#endif

//
//	JFile::~JFile
//

JFile::~JFile(void)
{
	Close();
}

//
//	JFile::NewFile
//

void JFile::NewFile( char *name, char *prompt )
{
#if Mac_Platform
	int				stat
	;
	unsigned char	myPrompt[32]
	;
	FSSpec			tempSpec
	;
	
	if (name) {
		strcpy( (char *)fileName, name );
		CtoPstr( (char *)fileName );
	} else
		JFilePStrCpy( (unsigned char *)fileName, "\pUntitled" );
	
	if (prompt) {
		strcpy( (char *)myPrompt, prompt );
		CtoPstr( (char *)myPrompt );
	} else
		JFilePStrCpy( (unsigned char *)myPrompt, "\pCreate a new file" );
	
	SFPutFile(	SFPwhere
		,		(unsigned char *)myPrompt
		,		(unsigned char *)fileName
		,		0L
		,		&reply
		);
	if (!reply.good)
		Throw_( err_UserCancel );		// user cancel
	
	stat = FSMakeFSSpec( reply.vRefNum, 0L, reply.fName, &tempSpec );
	if ((stat != noErr) && (stat != fnfErr))
		Throw_( stat );
	
	stat = FSpCreate( &tempSpec, fileCreator, fileType, smSystemScript );
	if ((stat != noErr) && (stat != dupFNErr))
		Throw_( stat );
	
	//	tell the open function to call Init() rather than Open()
	OpenFile( &tempSpec, true );
#endif
#if UNIX_Platform
	ThrowIfNil_( filePtr = fopen( name, "w+b" ) );
	
	Init();
#endif
}

//
//	JFile::NewFile( FSSpec * )
//

#if Mac_Platform
void JFile::NewFile( FSSpec *myFSS )
{
	int				stat
	;
	
	stat = FSpCreate( myFSS, fileCreator, fileType, smSystemScript );
	if ((stat != noErr) && (stat != dupFNErr))
		Throw_( stat );
	
	// tell the open function to call Init() rather than Open()
	OpenFile( myFSS, true );
}
#endif

//
//	JFile::OpenFile
//

void JFilePStrCpy(unsigned char *dst, unsigned char *src);

void JFile::OpenFile( char *name )
{
#if Mac_Platform
	unsigned char	theName[256]
	;
	SFTypeList		myTypes
	;
	FSSpec			tempSpec
	;
	
	if (name) {
		strcpy( (char *)theName, name );
		CtoPstr( (char *)theName );
		reply.vRefNum = 0;
	} else {
		myTypes[0] = fileType;
		SFGetFile(	SFGwhere
			,		nil
			,		0L
			,		1
			,		myTypes
			,		0L
			,		&reply
			);
		if (!reply.good)
			Throw_( err_UserCancel );
		
		JFilePStrCpy( theName, reply.fName );
	}
	
	ThrowIfOSErr_( FSMakeFSSpec( reply.vRefNum, 0L, theName, &tempSpec ) );
	
	OpenFile( &tempSpec, false );
#endif
#if UNIX_Platform
	// madanner 2/7/03
	// Only caller is OnDocumentOpen and it's expecting a char* exception, not the error code given
	// by the Throw macro.  UNIX_Platform is defined here as a hack even though NT_Platform may be 
	// defined...  Sorry for the lame error message... more could be done, but, why?

//	ThrowIfNil_( filePtr = fopen( name, "r+b" ) );

	filePtr = fopen( name, "r+b" );
	if ( !filePtr )
		throw ( "Database file cannot be opened.  The filename must be valid and the file must not be read only.");
	
	Open();
#endif
}

//
//	JFile::OpenFile
//

#if Mac_Platform
#if	Mac_OS_6_Supported
void JFile::OpenFile( AppFile *fileStuff )
{
	FSSpec	tempSpec
	;
	
	ThrowIfOSErr_( FSMakeFSSpec( fileStuff->vRefNum, 0L, fileStuff->fName, &tempSpec ) );
	OpenFile( &tempSpec, false );
}
#endif
#endif

//
//	JFile::OpenFile( FSSpec * )
//

#if Mac_Platform
void JFile::OpenFile( FSSpec *myFSS, bool isNew )
{
	JFilePStrCpy( (unsigned char *)fileName, myFSS->name );
	PtoCstr( (unsigned char *)fileName );
	vrefNum = myFSS->vRefNum;
	
	// open using deny mode permissions
	ThrowIfOSErr_( FSpOpenAware( myFSS, (fileShared ? dmRdWr : dmRdWrDenyRdWr), &refNum ) );
	
	ThrowIfOSErr_( FSpGetFInfo( myFSS, &fndrInfo ) );
	
	if (isNew) {
		JFile::SetSize( 0 );
		Init();
	} else
		Open();
}
#endif

//
//	JFile::CloseFile
//

void JFile::CloseFile(void)
{
#if Mac_Platform
	if (!refNum)
		return;
	
	Close();
	ThrowIfOSErr_( FSClose( refNum ) );
	
	refNum = 0;
#endif
#if UNIX_Platform
	if (!filePtr)
		return;
	
	Close();

	fclose( filePtr );
	filePtr = 0;
#endif
}

//
//	JFile::Init
//
//	The base class does nothing special to initialize a file, override this method to
//	perform some action when a new file is being created.
//

void JFile::Init(void)
{
}

//
//	JFile::Open
//
//	The base class does nothing special when an existing file is being opened, 
//	override this method.
//

void JFile::Open(void)
{
}

//
//	JFile::Close
//
//	The base class does nothing special when a file is being closed, override
//	this method.
//

void JFile::Close(void)
{
}

//
//	JFile::Flush
//
//	All flush operations should write out their changes to disk and then call 
//	this method which will flush the file cache the system maintains.  
//	Override this method and the last step call inherited::Flush().
//

void JFile::Flush(void)
{
#if Mac_Platform
	ThrowIfOSErr_( FlushVol( "\p", vrefNum ) );
#endif
#if UNIX_Platform
	fflush( filePtr );
#endif
}

//
//	JFile::Size
//

long JFile::Size(void)
{
	long	count
	;
	
#if Mac_Platform
	ThrowIfOSErr_( ::GetEOF( refNum, &count ) );
#endif
#if UNIX_Platform
	ThrowIfError_( fseek( filePtr, 0, SEEK_END ) );
	count = ftell( filePtr );
#endif
	
	return count;
}

//
//	JFile::SetSize
//

void JFile::SetSize(long size)
{
#if Mac_Platform
	ThrowIfOSErr_( ::SetEOF( refNum, size ) );
#endif
}

//
//	JFile::Read
//

int JFile::Read( long pos, const long count, void *data )
{
	int		stat
	;
	long	bytes = count
	;
	
#if _JFileDebug
	std::cout << "JFile::Read(" << pos << ',' << count << ",...)" << std::endl;
#endif
	
#if Mac_Platform
	stat = SetFPos( refNum, fsFromStart, pos );
	if (stat)
		return stat;
	
	stat = FSRead( refNum, &bytes, data );
	if (stat)
		return stat;
#endif
#if UNIX_Platform
	stat = fseek( filePtr, pos, SEEK_SET );
	if (stat)
		return stat;
	
	bytes = fread( data, 1, count, filePtr );
	if (bytes != count)
		return -1;
#endif
	
	return 0;
}

//
//	JFile::Write
//

int JFile::Write(long pos, const long count, void *data)
{
	int		stat
	;
	long	bytes = count
	;
	
#if _JFileDebug
	std::cout << "JFile::Write(" << pos << ',' << count << ",...)" << std::endl;
#endif
	
#if Mac_Platform
	stat = SetFPos( refNum, fsFromStart, pos );
	if (stat)
		return stat;
	
	stat = FSWrite( refNum, &bytes, data );
	if (stat)
		return stat;
#endif
#if UNIX_Platform
	stat = fseek( filePtr, pos, SEEK_SET );
	if (stat)
		return stat;
	
	bytes = fwrite( data, 1, count, filePtr );
	if (bytes != count)
		return -1;
#endif
	
	return 0;
}

//
//	JFilePStrCpy
//

void JFilePStrCpy(unsigned char *dst, unsigned char *src)
{
	int	len = *src + 1
	;
	
	while (len--)
		*dst++ = *src++;
}
