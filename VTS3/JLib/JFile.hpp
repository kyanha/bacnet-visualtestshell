
#ifndef _JFile
#define _JFile

#define _JFileDebug		0

// force UNIX platform hack
#if NT_Platform
#undef	UNIX_Platform
#define UNIX_Platform	1
#endif

#if ((!Mac_Platform) && (!UNIX_Platform))
#error	===== Hey!  You must include JConfig.hpp before JFile.hpp =====
#endif

#if Mac_Platform
#include <Files.h>
#include <string.h>
#endif
#if UNIX_Platform
#include <stdio.h>
#endif

//
//	JFile
//
//	This class implements the basic file management routines.  An instance is 
//	not bound to a file until the NewFile() or OpenFile() methods are called.
//
//	A class that implements a specific type of file overrides the Init() 
//	method to initialize a file and Open() to read basic structures.  Both functions 
//	may assume that the file is ready for Read() and Write() calls.  The Close() 
//	function will be called just before the file is really closed to give derived 
//	classes a chance to clean up, and Flush() is a way of saving any cached 
//	information to disk.
//

enum {
	err_UserCancel		= 'user'
};

class JFile {
	protected:
#if Mac_Platform
		short			refNum;							// file reference number
		short			vrefNum;						// volume reference number
		FInfo			fndrInfo;						// Finder information
#endif
#if UNIX_Platform
		FILE			*filePtr;						// UNIX file pointer
#endif
		
	public:
#if Mac_Platform
		long			fileType;						// passed to file system
		long			fileCreator;					//
		char			fileName[32];					// file name
		int				fileShared;						// shared access
#endif
		
#if Mac_Platform
		JFile( long fType = 'DATA', long fCreator = 'JDB ' );
#endif
#if UNIX_Platform
		JFile( void );
#endif
		virtual ~JFile(void);
		
		void NewFile( char *name = 0, char* prompt = 0 );		// SFPutFile
#if Mac_Platform
		void NewFile( FSSpec *myFSS );							//
#endif
		void OpenFile( char *name = 0 );						// SFGetFile
#if Mac_Platform
#if	Mac_OS_6_Supported
		void OpenFile( AppFile *fileStuff );					// System 6 Finder open
#endif
		void OpenFile( FSSpec *myFSS, bool isNew = false );		// System 7 AppleEvent
#endif
		void CloseFile(void);									// close it
		
		virtual void Init(void);								// newly created file
		virtual void Open(void);								// existing file
		virtual void Close(void);								// clean up operations
		virtual void Flush(void);								// no buffers unwritten
		
		long Size(void);										// size
		void SetSize(long size);								// new size
		
		int Read(long pos, const long count, void *data);		// read
		int Write(long pos, const long count, void *data);		// write
};

typedef JFile *JFilePtr, **JFileHandle;
const int kJFileSize = sizeof( JFile );

#endif
