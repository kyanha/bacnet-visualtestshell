
#ifndef _JError
#define _JError

#include "JConfig.hpp"

//
//	JError
//
//	These routines are lifted from PowerPlant, but simplified to exclude
//	all the redirection stuff and other PP includes.  I'll use these until 
//	there is a better way to mix these in with error classes.
//

#ifndef Throw_

#if Mac_Platform

#ifndef __ERRORS__
#include <Errors.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif
#endif

	// Exception codes

typedef long	ExceptionCode;

enum {
	err_NilPointer		= 'nilP',
	err_AssertFailed	= 'asrt'
};

	// Useful macros for signaling common failures

#define Throw_(err)			throw (ExceptionCode)(err)

	// This macro avoids evaluating "err" twice by assigning
	// its value to a local variable.

#if Mac_Platform
#define ThrowIfOSErr_(err)											\
	do {															\
		OSErr	__theErr = err;										\
		if (__theErr != noErr) {									\
			Throw_(__theErr);										\
		}															\
	} while (false)
#endif

#define ThrowIfError_(err)											\
	do {															\
		ExceptionCode	__theErr = err;								\
		if (__theErr != 0) {										\
			Throw_(__theErr);										\
		}															\
	} while (false)

#define ReturnIfError_(err)											\
	do {															\
		int	__theErr = err;											\
		if (__theErr != 0) {										\
			return(__theErr);										\
		}															\
	} while (false)

#if Mac_Platform
#define ThrowOSErr_(err)	Throw_(err)
#endif

#define	ThrowIfNil_(ptr)											\
	do {															\
		if ((ptr) == nil) Throw_(err_NilPointer);					\
	} while (false)

#define	ThrowIfNULL_(ptr)											\
	do {															\
		if ((ptr) == nil) Throw_(err_NilPointer);					\
	} while (false)

#if Mac_Platform
#define	ThrowIfResError_()	ThrowIfOSErr_(ResError())
#define	ThrowIfMemError_()	ThrowIfOSErr_(MemError())

#define	ThrowIfResFail_(h)											\
	do {															\
		if ((h) == nil) {											\
			OSErr	__theErr = ResError();							\
			if (__theErr == noErr) {								\
				__theErr = resNotFound;								\
			}														\
			Throw_(__theErr);										\
		}															\
	} while (false)
	
#define	ThrowIfMemFail_(p)											\
	do {															\
		if ((p) == nil) {											\
			OSErr	__theErr = MemError();							\
			if (__theErr == noErr) __theErr = memFullErr;			\
			Throw_(__theErr);										\
		}															\
	} while (false)
#else
#define	ThrowIfMemFail_(p)											\
	do {															\
		if ((p) == nil) {											\
			Throw_(-1);												\
		}															\
	} while (false)
#endif

#define	ThrowIf_(test)												\
	do {															\
		if (test) Throw_(err_AssertFailed);							\
	} while (false)

#define	ThrowIfNot_(test)											\
	do {															\
		if (!(test)) Throw_(err_AssertFailed);						\
	} while (false)

#if Mac_Platform
#define	FailOSErr_(err)		ThrowIfOSErr_(err)
#define FailNIL_(ptr)		ThrowIfNil_(ptr)
#endif

#endif
#endif
