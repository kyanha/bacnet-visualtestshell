
#ifndef _BACnetTask
#define _BACnetTask

//-------------------------------------------------- BACnetTask
//
//	A task is something that needs to be done.  There is a global list of all
//	of the tasks that need to be processed, the task manager takes care of making
//	sure all of the tasks get processed.
//
//	The taskTime is the time in clock ticks that the task should be processed.  If 
//	it's less than the current time (including zero) the task will fire.  An active
//	task is one that has been installed and will fire, NOT that a task is currently
//	running.
//
//	The taskType describes one three types of tasks: one-shot, one-shot and deleted, 
//	and recurring.  The taskInterval specifies how often a recurring task should 
//	execute.  If the taskInterval is zero for these recurring tasks, the task will 
//	be processed once each time the TaskManager function is called.
//
//	The suspend and resume methods are virtual so other functions can be tied to them
//	(like activating/deactivating a text field).  The ProcessTask function is pure 
//	virtual so useful objects must supply a method.
//
//	The TaskManager function is called by the main event loop.  It could be called 
//	by anybody that might tie up the machine for a while, like inside a TrackControl
//	loop.  The TaskControlProc is a glue routine that should be assigned to controls
//	where no other function would normally be provided.
//

class BACnetTask {
	public:
		enum BACnetTaskType
				{ oneShotTask		= 0
				, oneShotDeleteTask	= 1
				, recurringTask		= 2
				};

		BACnetTaskType	taskType;				// how to process
		long			taskInterval;			// how often to reschedule (ms)
		int				isActive;				// task is in queue to fire
		
		BACnetTask( BACnetTaskType typ = oneShotTask, long delay = 0 );
		virtual ~BACnetTask(void);
		
		void InstallTask(void);			 		// install into queue
		void SuspendTask(void);					// remove from execution queue
		void ResumeTask(void);					// put back in
		
		virtual void ProcessTask(void) = 0;		// do something
	};

typedef BACnetTask *BACnetTaskPtr;

//
//	BACnetTaskManager
//

class BACnetTaskManager {
	public:
		BACnetTaskManager( void ) {};
		virtual ~BACnetTaskManager( void ) {};

		virtual void InstallTask( BACnetTaskPtr tp ) = 0;
		virtual void SuspendTask( BACnetTaskPtr tp ) = 0;
		virtual void ResumeTask( BACnetTaskPtr tp ) = 0;
		
		virtual void ProcessTasks( void ) = 0;
	};

typedef BACnetTaskManager *BACnetTaskManagerPtr;

extern BACnetTaskManagerPtr gTaskManager;

#endif