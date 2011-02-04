// ThreadPool.cpp: implementation of the CThreadPool class.
// Rajeev Sadasivan, Trivandrum, INDIA
// 12-Dec-2002
// mailto: rajeevcs_in@yahoo.com
//////////////////////////////////////////////////////////////////////

#ifndef _AFXDLL 
	#include <Windows.h>
	#pragma message("ThreadPool: <Windows.h> included")
#endif

//#include "stdafx.h" // TODO: Remove comments if MFC application.
#pragma message("ThreadPool: stdafx.h included")

#include <process.h>
#include <tchar.h>
#include "ThreadPool.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define THREAD_POOL_BEGIN_BLOCK		do
#define THREAD_POOL_END_BLOCK		while(0);

#define THREAD_POOL_TASKQUEUE_SYNC  _T("TaskQueue_ThreadPool")
#define THREAD_POOL_CLIENT_SYNC		_T("ClientSync_ThreadPool")
#define THREAD_POOL_WAIT_TIMEOUT	120000 // 2 minutes.

#define INVALID_THREAD_COUNT		0xFFFFFFFF
#define INCREMENT_RANGE				64

#define THREAD_MARKED_FOR_REMOVAL	0x0001

#define WM_THREADPOOL_RESET			(WM_USER + 200)
#define WM_THREADPOOL_DO_TASK		(WM_THREADPOOL_RESET + 1)
 

/* C'tor. Application can set the maximum number of threads
	and maximum number of tasks, in the Initialize() method;
	The default value for thread count is currently 24
	The default value for task count is currently 1024.
		However the task info will grow, if necessary.
*/
CThreadPool::CThreadPool()
{
	m_cntMaxThread	= THREADPOOL_MAX_THREAD_COUNT;
	m_cntMaxTasks	= THREADPOOL_MAX_TASK_COUNT;
	m_cntThreads	= INVALID_THREAD_COUNT;
	m_cntRemove		= 0;
	m_ixNextTask	= 0;
	m_aFreeIndex	= NULL;
	m_aTasks		= NULL;
	m_aThreadData	= NULL;
	m_hTaskQueueSync= NULL;
	m_hClientSync	= NULL;

	// This Sync, applicable when multiple threads using the ThreadPool object.
	m_hClientSync = CreateMutex(NULL, FALSE, THREAD_POOL_CLIENT_SYNC);

	// Clear the Task Queue.
	while( false == m_TaskQueue.empty() )
		m_TaskQueue.pop();
}

// Calls UnInitialize(); Also destroys the sync mutexes.
CThreadPool::~CThreadPool()
{
	Uninitialize();
	if (NULL != m_hTaskQueueSync)	// Close worker thread's sync mutex.
		CloseHandle(m_hTaskQueueSync);
	if (NULL != m_hClientSync)		// Closes client sync mutex.
		CloseHandle(m_hClientSync);
}

/*
Start ThreadPool using with this call. It allocates all the resources it needs
	including the threads, memory. Frequent memory allocation is avoided to reduce
	memory fragmentation.
Params:
	cntThreads - Maximum number of threads the pool should have.
		If already initialized, then the existing number of threads 
		will be altered to the NEW requested count. If extra threads
		are there, they will be Freed; Otherwise new ones are allocated.
	nMaxThreadCount - Maximum thread - default is 24. Threads won't be allocated
		even if requsted thru cntThreads.
	nMaxTaskCount - Maximum task count - default is 1024. Grow if necessary.
Returns: 
	bool - true on success.
*/
bool CThreadPool::Initialize(UINT cntThreads, UINT nMaxThreadCount, UINT nMaxTaskCount)
{
	bool bRet = false;
	THREAD_POOL_BEGIN_BLOCK
	{
		if ( (0 >= cntThreads) || (cntThreads > nMaxThreadCount) ) break;
		
		m_cntMaxTasks = nMaxTaskCount;

		// Alloc thread data store.
		WaitForSingleObject(m_hClientSync, THREAD_POOL_WAIT_TIMEOUT);
		{
			if (NULL == m_aThreadData)
			{
				// First Time: Allocate the maximum number of blocks.
				m_aThreadData = (THREAD_DATA *)malloc(
									sizeof(THREAD_DATA) * nMaxThreadCount);
				memset(m_aThreadData , 0, (sizeof(THREAD_DATA) * nMaxThreadCount));
			}
			else
			{
				if ( nMaxThreadCount != m_cntMaxThread )
				{
					// Re-allocate it.
					m_aThreadData = (THREAD_DATA *)realloc( m_aThreadData, 
										sizeof(THREAD_DATA) * nMaxThreadCount );
					if ( nMaxThreadCount > m_cntMaxThread)
					{
						int nDiff = nMaxThreadCount - m_cntMaxThread;
						// Initialize the last portion.
						memset( LPBYTE(m_aThreadData)+(m_cntMaxThread * sizeof(THREAD_DATA)),
								0, (sizeof(THREAD_DATA) * nDiff));
					}
				}
			}
			m_cntMaxThread = nMaxThreadCount;

			// Initialize the count in class. First time.
			if (INVALID_THREAD_COUNT == m_cntThreads) m_cntThreads = 0;

			UINT cntRequested  = cntThreads;
			int  nThreadDiffer = m_cntThreads - cntThreads; 
			if ( nThreadDiffer < 0 )
			{
				nThreadDiffer = (nThreadDiffer * (-1));
				// New threads have to be allocated.
				AllocNewThreads((unsigned int)nThreadDiffer);
			}
			else if ( nThreadDiffer > 0)
			{
				// The remaining threads have to be destroyed. Mark those threads as 
				//	"marked for exit". The last threads in pool will be deleted.
				MarkThreadsForDelete((unsigned int)nThreadDiffer);
			}
			else 
			{
				// Do nothing. The requested count is already in the POOL.
				bRet = true;
				ReleaseMutex(m_hClientSync);
				break;
			}

			// Allocate the remaining resources.	
			if ( NULL == m_aFreeIndex )
			{
				m_aFreeIndex = (int*)malloc( m_cntMaxTasks * sizeof(int));
				for (UINT ixFree = 0; ixFree < m_cntMaxTasks; ixFree++)
					m_aFreeIndex[ixFree] = ixFree;
			}
			if ( NULL == m_aTasks)
			{
				m_aTasks = (PTHREAD_POOL_QUEUE_INFO)malloc ( 
									m_cntMaxTasks * sizeof(THREAD_POOL_QUEUE_INFO)); 
				memset(m_aTasks, 0, m_cntMaxTasks * sizeof(THREAD_POOL_QUEUE_INFO));
			}
		}
		ReleaseMutex(m_hClientSync);

		m_hTaskQueueSync = CreateMutex(NULL, FALSE, THREAD_POOL_TASKQUEUE_SYNC);
		// Broadcast abt thread pool reset.
		NotifyAll( WM_THREADPOOL_RESET, 0, 0 );
		bRet = true;
	}
	THREAD_POOL_END_BLOCK
	
	return bRet;
}

/*
Finally call this to free all resources including the threads.
	Upon object destruction, this will be called by the destructor.
	It terminates all threads, free allocated memory and clear the Task Queue.
Params:
	NIL
Returns:
	NIL
Special:
	TerminateThread is used to cause a thread to exit. When this occurs, 
	the target thread has no chance to execute any user-mode code and its 
	initial stack is not deallocated. DLLs attached to the thread are not 
	notified that the thread is terminating. However this is done as 
	a second attempt.
*/
void CThreadPool::Uninitialize()
{
	WaitForSingleObject(m_hTaskQueueSync, THREAD_POOL_WAIT_TIMEOUT);
	{
		// Clear the Task Queue.
		while( false == m_TaskQueue.empty() )
			m_TaskQueue.pop();
	}
	ReleaseMutex(m_hTaskQueueSync);

	UINT ixThread = 0;
	while(ixThread < m_cntMaxThread)
	{
		if ( 0 != m_aThreadData[ixThread].nThreadId )
		{
			// Allow thread to terminate itself gracefully.
			WaitForSingleObject(m_hClientSync, THREAD_POOL_WAIT_TIMEOUT);
			PostThreadMessage(m_aThreadData[ixThread].nThreadId, WM_QUIT, 1, 0);
			ReleaseMutex(m_hClientSync);

			Sleep(100); 

			// If not exited, then try to Terminate.
			if ( 0 != m_aThreadData[ixThread].nThreadId )
			{
				TerminateThread( (HANDLE)m_aThreadData[ixThread].hThread, 
								 1/*ExitCode*/ );
				FreeThreadDataBlock(ixThread);
			}
		}
		ixThread++;
	}
	m_cntThreads = INVALID_THREAD_COUNT;

	WaitForSingleObject(m_hClientSync, THREAD_POOL_WAIT_TIMEOUT);
	{
		free(m_aFreeIndex);
		m_aFreeIndex = NULL;
		free(m_aTasks);
		m_aTasks = NULL;
		free(m_aThreadData);
		m_aThreadData = NULL;
	}
	ReleaseMutex(m_hClientSync);
}

/*
Assin task to thread pool. If threads are free, tasks are assigned immidiately.
Otherwise tasks are queued for execution.
Params:
	PTHREAD_POOL_TASK pTask - Task request for execution.
	struct THREAD_POOL_TASK
	{
		UINT  unTaskId;-: Task Id; Can be 0; Identify the functionality.
		THREAD_POOL_EXECUTE_FN	pFunction;-: Must NOT be NULL.
										      Execution Procedure provided by client.
	};
Returns:
	bool; true on success.
Special:
	NO thread synchronization in the procedure by ThreadPool. If some shared data 
	is there, application should protect it using its own sync objects.
*/
bool CThreadPool::AssignTask(PTHREAD_POOL_TASK pTask)
{
	bool bRet = false;
	THREAD_POOL_BEGIN_BLOCK
	{
		int nFreeIndex = 0;
		WaitForSingleObject(m_hClientSync, THREAD_POOL_WAIT_TIMEOUT);
		{
			if ( (NULL == pTask) || 
				 (NULL == m_aFreeIndex) || 
				 (NULL == m_aTasks) )	
			{
				ReleaseMutex(m_hClientSync);
				break;	// Internal data block NOT allocated.
			}

			// Find first free index.
			UINT ixFree = 0;
			for (; ixFree < m_cntMaxTasks; ixFree++)
				if ( -1 != m_aFreeIndex[ixFree] ) break; // From For loop;

			// If free is not there, allocate new block;
			//	Check extra allocation is needed or not. Re-allocate it.
			if ( ixFree == m_cntMaxTasks)
			{
				// No free index. Increase Task Info size. Grow the array.
				m_cntMaxTasks += INCREMENT_RANGE;
				m_aTasks = (PTHREAD_POOL_QUEUE_INFO)realloc( m_aTasks, 
								(m_cntMaxTasks * sizeof(THREAD_POOL_QUEUE_INFO)) ); 
				memset( (LPBYTE(m_aTasks) + (ixFree * sizeof(THREAD_POOL_QUEUE_INFO))), 
						0, INCREMENT_RANGE * sizeof(THREAD_POOL_QUEUE_INFO) );
				m_aFreeIndex = (int *)realloc( m_aFreeIndex, 
								(m_cntMaxTasks * sizeof(int)) ); 
				// Initialize newly allocated free indexes.
				for (UINT ixNewFree = ixFree; ixNewFree < m_cntMaxTasks; ixNewFree++)
					m_aFreeIndex[ixNewFree] = ixNewFree;
			}
			nFreeIndex = m_aFreeIndex[ixFree];
			memcpy(&(m_aTasks[nFreeIndex].FuncInfo), pTask, sizeof(THREAD_POOL_TASK));
			m_aTasks[nFreeIndex].ixTask = ixFree;
			m_aFreeIndex[ixFree] = -1;	// (-1) means, seat is occupied.
		}
		ReleaseMutex(m_hClientSync);
		
		// Task queue is only need to be sync. All other data blocks cause no damage.
		WaitForSingleObject(m_hTaskQueueSync, THREAD_POOL_WAIT_TIMEOUT);
		m_TaskQueue.push(&(m_aTasks[nFreeIndex]));
		ReleaseMutex(m_hTaskQueueSync);
		
		// Broadcast message to all threads that one task has arrived.
		NotifyAll( WM_THREADPOOL_DO_TASK, 0, 0 );

		bRet = true;
	}
	THREAD_POOL_END_BLOCK
	
	return bRet;
}

// Static private function for the thread callback.
/*static*/ UINT CThreadPool::WorkerThreadProc( void * pData )
{	
	if ( NULL == pData ) return 1;

	CThreadPool *pThisThreadPool = (CThreadPool*) pData;
	pThisThreadPool->WorkerThreadProc();

	return 0;
}

// Allocate the new threads. All threads r generated thru this function.
void CThreadPool::AllocNewThreads(UINT cntThrds)
{
	int ixThreadData = m_cntThreads;
	while ( ixThreadData <  (m_cntThreads + cntThrds) )
	{
		unsigned unThrdId = 0;
		unsigned long hThread = _beginthreadex( 0, 0, 
									(THREAD_PROC)CThreadPool::WorkerThreadProc, 
									(void *)this, 0, &unThrdId );
		if ( 0 == hThread) break;
		// Add thread handle to handle queue.
		m_aThreadData[ixThreadData].hThread	  = hThread;
		m_aThreadData[ixThreadData].nThreadId = unThrdId;
		m_aThreadData[ixThreadData].nFlags	  = 0;

		// Change the index.
		ixThreadData++; 
		// If maximum number of thread reached, exit.
		if ( m_cntMaxThread == ixThreadData ) break;
	}
	// Increment total valid thread counter.
	m_cntThreads = ixThreadData;
}

// Mark the given count of threads for Removal. Starting from the 
//	Higher Index to Lower Index.
void CThreadPool::MarkThreadsForDelete(UINT cntThrds)
{
	int ixThreadData = m_cntThreads - 1;
	while ( ixThreadData >= (m_cntThreads - cntThrds) )
	{
		// Add thread handle to handle queue.
		m_aThreadData[ixThreadData].nFlags = THREAD_MARKED_FOR_REMOVAL;

		// Change the index.
		ixThreadData--;
	}
	// Marking for removal enabled.
	m_cntRemove = cntThrds;
}


// Final procedure for thread callback.
void CThreadPool::WorkerThreadProc()
{
	THREAD_POOL_BEGIN_BLOCK
	{
		MSG msg = {0};
		// Get the Task OR wait for the task.
		while (WM_QUIT != GetMessage(&msg, NULL, 0, 0) )
		{
			// Kill the thread by self, if its marked for exit.
			RemoveIfMarked();
			
			if ( WM_THREADPOOL_RESET == msg.message )
			{
				continue;
			}
			else if ( WM_THREADPOOL_DO_TASK == msg.message )
			{
				PTHREAD_POOL_QUEUE_INFO pQueueInfo = NULL;
				
				WaitForSingleObject(m_hTaskQueueSync, THREAD_POOL_WAIT_TIMEOUT);
				{
					if (false == m_TaskQueue.empty())
					{
						pQueueInfo = m_TaskQueue.front();
						m_TaskQueue.pop();
					}
				}
				ReleaseMutex(m_hTaskQueueSync);
				if (NULL == pQueueInfo) 
				{
					continue;
				}
				
				PTHREAD_POOL_TASK pTask = (PTHREAD_POOL_TASK)pQueueInfo;

				// Execute the function, provided by the client.
				if (NULL != pTask->pFunction)
					pTask->pFunction(pTask->unMessage, pTask->pExtraInfo);

				// Update free-table & re-initialize the task block;
				WaitForSingleObject(m_hClientSync, THREAD_POOL_WAIT_TIMEOUT);
				m_aFreeIndex[pQueueInfo->ixTask] = pQueueInfo->ixTask;
				ReleaseMutex(m_hClientSync);

				pTask->unMessage  = 0;
				pTask->pFunction  = 0;
				pTask->pExtraInfo = 0;
				pQueueInfo->ixTask= 0;
			}
		}

		// Reset thread-data structure block belongs to my thread.
		DWORD ixMyThread = GetMyThreadDataIndex();
		WaitForSingleObject( m_hClientSync, THREAD_POOL_WAIT_TIMEOUT );
		{
			// Free the thread data block, belongs to this thread.
			FreeThreadDataBlock(ixMyThread);
			m_cntThreads--;	// Alter total thread count.
		}
		ReleaseMutex(m_hClientSync);
	}
	THREAD_POOL_END_BLOCK

	return;
}

/* Check whether this thread is marked for removal.
	If it is, end the thread, close the thread handle, release the 
	thread data block entry.
*/
void CThreadPool::RemoveIfMarked()
{
	THREAD_POOL_BEGIN_BLOCK
	{
		// If threads are marked for removal, remove it if this is the same thread.
		if (m_cntRemove > 0 ) // Thread pool marked some threads for removal
		{
			DWORD ixMyThread = GetMyThreadDataIndex();
			if ( NULL == m_aThreadData ) break;
			if ( 0 != (m_aThreadData[ixMyThread].nFlags & THREAD_MARKED_FOR_REMOVAL) ) 
			{
				WaitForSingleObject( m_hClientSync, THREAD_POOL_WAIT_TIMEOUT );
				{
					// Free the thread data block, belongs to this thread.
					FreeThreadDataBlock(ixMyThread);
					m_cntRemove--;	// Decrement removal counter.
					m_cntThreads--;	// Alter total thread count.
				}
				ReleaseMutex(m_hClientSync);
				_endthreadex(0);
			}
		}
	}
	THREAD_POOL_END_BLOCK

	return;
}

// Returns the current thread index. The caller must be from thread pool.
UINT CThreadPool::GetMyThreadDataIndex()
{
	UINT ixReturn = 0;
	THREAD_POOL_BEGIN_BLOCK
	{
		if ( NULL == m_aThreadData ) break;

		// The caller must be in one from this thread pool.
		DWORD dwId = GetCurrentThreadId();
		WaitForSingleObject( m_hClientSync, THREAD_POOL_WAIT_TIMEOUT );
		{
			while( ixReturn < m_cntMaxThread )
			{
				if ( m_aThreadData[ixReturn].nThreadId == dwId ) break;//From while()
				ixReturn++;
			}
		}
		ReleaseMutex(m_hClientSync);
	}
	THREAD_POOL_END_BLOCK
	return ixReturn;
}


// Free the thread data block, belongs to the given thread index.
void CThreadPool::FreeThreadDataBlock(UINT ixThread)
{
	THREAD_POOL_BEGIN_BLOCK
	{
		if ( NULL == m_aThreadData ) break;
		WaitForSingleObject( m_hClientSync, THREAD_POOL_WAIT_TIMEOUT );
		{
			CloseHandle( (HANDLE)(m_aThreadData[ixThread].hThread) );
			m_aThreadData[ixThread].hThread	  = 0;
			m_aThreadData[ixThread].nThreadId = 0;
			m_aThreadData[ixThread].nFlags    = 0;
		}
		ReleaseMutex(m_hClientSync);
	}
	THREAD_POOL_END_BLOCK
}

// Broadcast message to all threads that one task has arrived.
void CThreadPool::NotifyAll(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	for (int ixThread=0; ixThread < m_cntMaxThread; ixThread++ )
	{
		if ( 0 != m_aThreadData[ixThread].nThreadId )
		{
			PostThreadMessage( m_aThreadData[ixThread].nThreadId, 
				nMsg, wParam, lParam );
		}
	}
}
