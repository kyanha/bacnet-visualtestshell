// VTSQueue.cpp: implementation of the VTSQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTS.h"
#include "VTSQueue.h"

#ifndef AFX_SCRIPTEXECUTOR_H__AC59C5B2_BAFF_11D4_BEF7_00A0C95A9812__INCLUDED_
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

//
//	VTSQueue<T>::VTSQueue
//

template<class T>
	VTSQueue<T>::VTSQueue()
		: qFirst(0), qLast(0)
	{
	}

//
//	VTSQueue<T>::~VTSQueue
//

template<class T>
	VTSQueue<T>::~VTSQueue()
	{
		if (qFirst)
			TRACE0( "Warning: queue not empty\n" );
	}

//
//	VTSQueue<T>::Read
//
//	This function returns a pointer to the first (oldest) element that was
//	added to the queue.  If the queue is empty it returns nil.
//

template<class T>
	T* VTSQueue<T>::Read( void )
	{
		// lock the queue
		qCS.Lock();

		VTSQueueElem	*cur = qFirst
		;
		T				*rslt = 0
		;

		// if the queue not is empty, extract the first element
		if (cur) {
			// set result
			rslt = cur->qElem;

			// remove from list
			qFirst = cur->qNext;
			if (!qFirst)
				qLast = 0;

			// delete wrapper element
			delete cur;
		}

		// unlock the queue
		qCS.Unlock();

		// fini
		return rslt;
	}

//
//	VTSQueue<T>::Write
//
//	This function adds a pointer to an element to the end of the queue.
//

template<class T>
	void VTSQueue<T>::Write( T* tp )
	{
		// lock the queue
		qCS.Lock();

		VTSQueueElem	*cur = new VTSQueueElem()
		;

		cur->qElem = tp;
		cur->qNext = 0;

		if (qFirst)
			qLast->qNext = cur;
		else
			qFirst = cur;

		qLast = cur;

		// unlock the queue
		qCS.Unlock();
	}

