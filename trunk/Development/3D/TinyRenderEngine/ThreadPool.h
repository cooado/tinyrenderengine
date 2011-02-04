// ThreadPool.h: interface for the CThreadPool class.
// Rajeev Sadasivan, Trivandrum, INDIA
// 12-Dec-2002
// mailto: rajeevcs_in@yahoo.com
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREADPOOL_H__653A92C6_53BE_42B9_90BD_26F5DC231A56__INCLUDED_)
#define AFX_THREADPOOL_H__653A92C6_53BE_42B9_90BD_26F5DC231A56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning( disable: 4786 )
#include <queue>
using namespace std ;
#pragma warning( default: 4786 )

/* ------------------------------ USAGE ------------------------------ >> 
1. Insert ThreadPool.cpp & .h in project.
2. Project Settings 
	- On the Project menu, Click Settings. 
	- In the Project Settings dialog box, click the C/C++ tab. 
	- Select Code Generation from the Category drop-down list box. 
	- From the Use Run-Time Library drop-down box, select MULTITHREADED. 
	- Click OK.
3. Constuct - Max threads, tasks can be recommended. Tasks list size 
	will grow if required.
4. Call Initialize() with required thread count.
5. Assign Task with a _cdecl function pointer of the client.
6. Function will be executed from any of the free thread in thread pool.
7. If DATA members need to be sync, in the submitted function in client,
	that has to be done by the client.
8. When thread count need to be increased/decreased, just call Initialize again. 
	NO Resource/Memory LEAK. The current threads finishes executuion.
	Then it will be aborted.
9. To destroy the thread pool, without deleting threadpool object,
	call UnInitialize(). This will delete all resources. If threads are still
	working, after WM_QUIT posting & a small waiting, it terminates all threads.
10. UnInitialize() is called from destructor() also.
   << ------------------------------ USAGE ---------------------------
*/

// -------Common External Data structures---------------

// e.g. bool MyThreadPoolCallback(UINT nFunctionIdentifier, LPVOID pMyData)
typedef bool (*THREAD_POOL_EXECUTE_FN)(UINT, LPVOID);

typedef struct _tagTHREAD_POOL_TASK
{
	UINT  unMessage;					// Task Id; Can be 0.
	THREAD_POOL_EXECUTE_FN	pFunction;	// Execution Procedure provided by client.
	LPVOID pExtraInfo;					// Extra information.			
}
THREAD_POOL_TASK, *PTHREAD_POOL_TASK;

// --------------------------------------------

// --- Private Data structures --- >>

#define THREADPOOL_MAX_THREAD_COUNT	24
#define THREADPOOL_MAX_TASK_COUNT	1024

typedef struct _tagTHREAD_POOL_QUEUE_INFO
{
	THREAD_POOL_TASK FuncInfo;
	UINT ixTask;	
} THREAD_POOL_QUEUE_INFO, * PTHREAD_POOL_QUEUE_INFO;

typedef unsigned int (__stdcall* THREAD_PROC)(void *);

typedef struct _tagTHREAD_DATA
{
	unsigned long hThread;		// Thread handle.
	unsigned int  nThreadId;	// Thread Id.
	unsigned int  nFlags;		// Thread status such as marked for exit.	
}
THREAD_DATA, *PTHREAD_DATA;

class CThreadPool  
{
	queue<PTHREAD_POOL_QUEUE_INFO> m_TaskQueue;	// Task queue.
	int *m_aFreeIndex;
	THREAD_DATA *  m_aThreadData;
	UINT m_cntThreads;
	PTHREAD_POOL_QUEUE_INFO m_aTasks;	// Task Data block.
	UINT m_ixNextTask;
	HANDLE m_hTaskQueueSync;			
	HANDLE m_hClientSync;

public:	// Static methods.
	static unsigned int __stdcall WorkerThreadProc( void * pInfo);

public:	// Public Methods.
	CThreadPool();
	virtual ~CThreadPool();
	bool Initialize( UINT cntThreads=1/*Default value*/, 
				UINT nMaxThreadCount= THREADPOOL_MAX_THREAD_COUNT, 
				UINT nMaxTaskCount= THREADPOOL_MAX_TASK_COUNT );
	void Uninitialize();
	bool AssignTask(PTHREAD_POOL_TASK pTask);
	void WorkerThreadProc();

private: // Data
	UINT m_cntRemove;
	UINT m_cntMaxThread;
	UINT m_cntMaxTasks;

private: // Methods
	void NotifyAll(UINT nMsg, WPARAM wParam, LPARAM lParam);
	void FreeThreadDataBlock(UINT ixThread);
	UINT GetMyThreadDataIndex();
	void RemoveIfMarked();
	void MarkThreadsForDelete(UINT cntThrds);
	void AllocNewThreads(UINT cntThrds);
};

#endif // !defined(AFX_THREADPOOL_H__653A92C6_53BE_42B9_90BD_26F5DC231A56__INCLUDED_)
