
#ifndef _JTextFile
#define _JTextFile

#ifndef _JFile
#include "JFile.hpp"
#endif

#define	_JTextFileDebug		0

//
//	JTextFile
//
//	A text file is a simple subclass of JFile that reads and writes
//	text records.  It needs the maximum record length as a ctor parameter.
//
//	All '\n' and '\r' characters are stripped away during a read.  There 
//	has to be room to append a '\n' for the Write().
//

enum JFileEOL { eNativeEOL, eMacintoshEOL, ePCEOL, eUnixEOL };

class JTextFile : public JFile {
	protected:
		char		*textBuff;			// temporary buffer
		int			textBuffSize;		// buffer size
		long		textPos;			// fake file pointer position
		JFileEOL	textEOL;			// what kind of line termination
		
	public:
		JTextFile( int bufSize, JFileEOL eol = eNativeEOL );
		~JTextFile( void );
		
		virtual void Open(void);		// existing file
		
		int Read( char *data = 0 );		// read data
		int Write( char *data = 0 );	// write data
		
		int GetPos( long &pos );		// get current file position
		int SetPos( long pos );			// change file position
	};

typedef JTextFile *JTextFilePtr;
const int kJTextFileSize = sizeof( JTextFile );

#endif
