
#ifndef _JConfig
#define _JConfig

//
//	Configuration
//
//	This file contains all of the #define's necessary for the different
//	platforms the code library runs on.
//

#define Mac_Platform		0
#define Mac_OS_6_Supported	0
#define UNIX_Platform		0
#define DOS_Platform		0
#define NT_Platform			1

#if NT_Platform
#define nil		((void *)0)
#define Boolean	BOOL
#endif

#endif
