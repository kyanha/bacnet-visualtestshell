// VTSQueue.h: interface for the VTSQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VTSQUEUE_H__AC6F4361_C02A_11D4_BEFB_00A0C95A9812__INCLUDED_)
#define AFX_VTSQUEUE_H__AC6F4361_C02A_11D4_BEFB_00A0C95A9812__INCLUDED_

#if _MSC_VER > 1000
  #pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include "stdafx.h"
#include "VTS.h"

//
//	VTSQueue
//
//	A VTSQueue is a FIFO queue that is thread safe.  It is used to transfer 
//	messages from the script executor to the main application thread, as well 
//	as transfer messages from the IO thread to the BACnetTaskManager thread.
//
//	It is a simple linked list.  Pointers to elements (rather than elements 
//	themselves) are added and removed from the list.  If dynamic elements are 
//	created and added to the list it is up to the reader to dispose of them.
//

template<class T>
	class VTSQueue {
		private:
			struct VTSQueueElem {
				T				*qElem;
				VTSQueueElem	*qNext;
				};

			CCriticalSection	qCS;			// read/write lock

			VTSQueueElem		*qFirst;		// first element
			VTSQueueElem		*qLast;			// last element

		public:
			VTSQueue();
			virtual ~VTSQueue();

			T* Read( void );					// nil if empty
			void Write( T* t );					// add a pointer to an element
		};



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



#endif // !defined(AFX_VTSQUEUE_H__AC6F4361_C02A_11D4_BEFB_00A0C95A9812__INCLUDED_)
