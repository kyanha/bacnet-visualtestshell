#ifndef _JMemory
#define _JMemory

#include "JConfig.hpp"

#if Mac_Platform

//
//	Macintosh Memory Management Routines
//
//	The MacOS has these memory management routines that are designed to allow an application 
//	to work in low memory conditions where other applications would fail.  This database 
//	code is written to take advantage of those routines whenever they are available.  If this 
//	is not a MacOS version of the application, there are fill-in routines.
//

#define JPtr						Ptr
#define JHandle						Handle
#define JNewHandle(x)				::NewHandle(x)
#define JDisposeHandle(x)			::DisposeHandle(x)
#define JHLock(x)					::HLock(x)
#define JHUnlock(x)					::HUnlock(x)
#define JSetHandleSize(x,y)			::SetHandleSize(x,y)
#define JGetHandleSize(x)			::GetHandleSize(x)
#define JMunger(p1,p2,p3,p4,p5,p6)	::Munger(p1,p2,p3,p4,p5,p6)
#define JBlockMove(p1,p2,p3)		::BlockMove(p1,p2,p3)

#else

//
//	Non-Macintosh Memory Management Routines
//
//	This glue routines provide the same function that the Macintosh routines do, except they 
//	are designed to be portable, so they may not be as fast and could lead to fragmented 
//	memory conditions.
//

typedef char	*JPtr;
typedef JPtr	*JHandle;

struct JHandleRec {
	JPtr	handlePtr;
	long	handleSize;
	};

typedef JHandleRec *JHandleRecPtr;

JHandle JNewHandle(int);
void JDisposeHandle(JHandle);

void JHLock(JHandle);
void JHUnlock(JHandle);

void JSetHandleSize(JHandle,int);
int JGetHandleSize(JHandle);
void JMunger(JHandle,int,JPtr,int,JPtr,int);
void JBlockMove(JPtr,JPtr,int);

#endif

#endif
