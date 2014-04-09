// File contains PrivateThreadPool implementation and its helper classes 
// Author: Hari Pulapaka
//
#pragma once

#include <iostream> 
#include <Windows.h>
#include <assert.h>
#include <functional>

using namespace std; 

namespace windowsthreadpool
{

	namespace internal
	{
		// Internal class which submits the work items 
		template <class Function>
		class WorkCallBack
		{
		private: 
			const Function m_Func;
			PVOID state;
			PTP_WORK work;

			static void CALLBACK callback (PTP_CALLBACK_INSTANCE Instance, PVOID Param, PTP_WORK work)
			{
				UNREFERENCED_PARAMETER(Instance);
				UNREFERENCED_PARAMETER(work);
			
				WorkCallBack<Function> *pc = reinterpret_cast<WorkCallBack<Function>*>(Param);
				pc->m_Func(pc->state);			
				delete pc; 

				return; 
			}

		public:
			WorkCallBack(const Function Func, PVOID st, PTP_CALLBACK_ENVIRON pEnv) : m_Func(Func), state(st)
			{			
				work = CreateThreadpoolWork(callback, this, pEnv); 
				if (!work)
				{
					throw "Error: Could not create threadpool work.";
				}
			
				SubmitThreadpoolWork(work);
			}
		};
	} 

	class PrivateThreadPool
	{
	private:
		PTP_POOL Pool; 
		TP_CALLBACK_ENVIRON CallbackEnvironment;
		PTP_CLEANUP_GROUP CleanupGroup;
		bool InfraInitialized; 
		HKEY foo; 

		void InitializeInfra()
		{
			// Function cant fail (I think) 
			// The reason to have a separate Initialize function is 
			// to avoid error if someone waits for callbacks
			// and then calls queueuserworkitem again. 

			if (!InfraInitialized)
			{
				InitializeThreadpoolEnvironment(&CallbackEnvironment);
				SetThreadpoolCallbackPool(&CallbackEnvironment, Pool);
				CleanupGroup = CreateThreadpoolCleanupGroup();
				SetThreadpoolCallbackCleanupGroup(&CallbackEnvironment, CleanupGroup, NULL);
				InfraInitialized = true; 
			}
			return; 			
		}

		void WaitForAllCallbacks(bool CancelNotStarted)
		{
			// Wait for all callbacks to complete and cancel any work items 
			// which havent started if required.

			assert(InfraInitialized == true); 
			CloseThreadpoolCleanupGroupMembers(CleanupGroup, CancelNotStarted, NULL);
			CloseThreadpoolCleanupGroup(CleanupGroup);
			InfraInitialized = false; 
		}

		template <class Function>
		bool QueueUserWorkItemInternal(Function cb, PVOID State)
		{
			InitializeInfra(); 

			// Queues the work item to the default process-wide ThreadPool 			
			windowsthreadpool::internal::WorkCallBack<Function> *wcb = new windowsthreadpool::internal::WorkCallBack<Function>(cb, State, &CallbackEnvironment);
		
			return true;
		}

		template <class Function>
		bool QueueUserWorkItemInternal(Function cb, PVOID State, PTP_CALLBACK_ENVIRON env)
		{
			InitializeInfra(); 

			// Queues the work item to the default process-wide ThreadPool 			
			windowsthreadpool::internal::WorkCallBack<Function> *wcb = new windowsthreadpool::internal::WorkCallBack<Function>(cb, State, env);
		
			return true;
		}

		template <class Function>
		PVOID RegisterEventInternal(Function cb, PVOID state, HANDLE Event, size_t timeout)
		{
			if (timeout > -1)
			{
				throw "timeout should be greater than -1."; 
			}
			InitializeInfra(); 
			windowsthreadpool::internal::WaitCallBack<Function> *wcb = new windowsthreadpool::internal::WaitCallBack<Function>(cb, state, &CallbackEnvironment, Event, timeout); 

			return reinterpret_cast<PVOID>(wcb); 
		}

	public:
		PrivateThreadPool(): InfraInitialized(false)
		{
			Pool = CreateThreadpool(NULL); 
			if (!Pool)
			{
				throw "Error: CreateThreadpool Failed.";
			}
		}

		~PrivateThreadPool()
		{
			CloseThreadpool(Pool); 
		}

		bool SetThreadpoolMax(size_t max)
		{
			SetThreadpoolThreadMaximum(Pool, max); 
			return true; 
		}

		bool SetThreadpoolMin(size_t min)
		{
			return SetThreadpoolThreadMinimum(Pool, min); 
		}

		template <class Function> 
		bool QueueUserWorkItem(Function cb)
		{
			return QueueUserWorkItemInternal(cb, NULL); 
		}

		template <class Function> 
		bool QueueUserWorkItem(Function cb, PVOID State)
		{
			return QueueUserWorkItemInternal(cb, State);
		}

		template <class Function>
		PVOID RegisterEvent(Function cb, HANDLE Event, int timeout)
		{
			return RegisterEventInternal(cb, NULL, Event, timeout);
		}

		template <class Function>
		PVOID RegisterEvent(Function cb, PVOID State, HANDLE Event, int timeout)
		{
			return RegisterEventInternal(cb, State, Event, timeout);
		}

		// Needs to be called before WaitForAll
		// 
		template <class Function>
		bool DestroyEvent(PVOID EventHandle)
		{
			windowsthreadpool::internal::WaitCallBack<Function> *wcb = reinterpret_cast<windowsthreadpool::internal::WaitCallBack<Function>*> (EventHandle); 
			if (InfraInitialized)
			{
				wcb->DestroyEvent(); 
			}
			delete wcb; 
			return true; 
		}

		// Re-Register the event 
		//
		template <class Function>
		bool ReRegisterEvent(PVOID EventHandle)
		{
			windowsthreadpool::internal::WaitCallBack<Function> *wcb = reinterpret_cast<windowsthreadpool::internal::WaitCallBack<Function>*> (EventHandle);
			return wcb->ReRegisterEvent(); 
		}

		// Re-Register the event with a different timeout 
		// 
		template <class Function>
		bool ReRegisterEvent(PVOID EventHandle, int timeout)
		{
			windowsthreadpool::internal::WaitCallBack<Function> *wcb = reinterpret_cast<windowsthreadpool::internal::WaitCallBack<Function>*> (EventHandle);
			return wcb->ReRegisterEvent(timeout); 
		}

		// Queues a work item to the end of the queue
		//
		template <class Function>
		bool QueueUserWorkItemWithLowPri(Function cb)
		{
			InitializeInfra(); 
			TP_CALLBACK_ENVIRON LowPriCallbackEnvironment;
			InitializeThreadpoolEnvironment(&LowPriCallbackEnvironment);
			SetThreadpoolCallbackPool(&LowPriCallbackEnvironment, Pool);
			SetThreadpoolCallbackPriority(&LowPriCallbackEnvironment, TP_CALLBACK_PRIORITY_LOW); 
			SetThreadpoolCallbackCleanupGroup(&LowPriCallbackEnvironment, CleanupGroup, NULL);

			return QueueUserWorkItemInternal(cb, NULL, &LowPriCallbackEnvironment); 
		}

		template <class Function>
		bool QueueUserWorkItemWithLowPri(Function cb, PVOID State)
		{
			InitializeInfra(); 
			TP_CALLBACK_ENVIRON LowPriCallbackEnvironment;
			InitializeThreadpoolEnvironment(&LowPriCallbackEnvironment);
			SetThreadpoolCallbackPool(&LowPriCallbackEnvironment, Pool);
			SetThreadpoolCallbackPriority(&LowPriCallbackEnvironment, TP_CALLBACK_PRIORITY_LOW); 
			SetThreadpoolCallbackCleanupGroup(&LowPriCallbackEnvironment, CleanupGroup, NULL);

			return QueueUserWorkItemInternal(cb, State, &LowPriCallbackEnvironment); 
		}

		// Queues a work item to the start of the queue
		//
		template <class Function>
		bool QueueUserWorkItemWithHiPri(Function cb)
		{
			InitializeInfra(); 
			TP_CALLBACK_ENVIRON HiPriCallbackEnvironment;
			InitializeThreadpoolEnvironment(&HiPriCallbackEnvironment);
			SetThreadpoolCallbackPool(&HiPriCallbackEnvironment, Pool);
			SetThreadpoolCallbackPriority(&HiPriCallbackEnvironment, TP_CALLBACK_PRIORITY_HIGH); 
			SetThreadpoolCallbackCleanupGroup(&HiPriCallbackEnvironment, CleanupGroup, NULL);

			return QueueUserWorkItemInternal(cb, NULL, &HiPriCallbackEnvironment); 
		}

		template <class Function>
		bool QueueUserWorkItemWithHiPri(Function cb, PVOID State)
		{
			InitializeInfra(); 
			TP_CALLBACK_ENVIRON HiPriCallbackEnvironment;
			InitializeThreadpoolEnvironment(&HiPriCallbackEnvironment);
			SetThreadpoolCallbackPool(&HiPriCallbackEnvironment, Pool);
			SetThreadpoolCallbackPriority(&HiPriCallbackEnvironment, TP_CALLBACK_PRIORITY_HIGH); 
			SetThreadpoolCallbackCleanupGroup(&HiPriCallbackEnvironment, CleanupGroup, NULL);

			return QueueUserWorkItemInternal(cb, State, &HiPriCallbackEnvironment); 
		}

		// Waits for all work items incl. low priority
		//
		void WaitForAll()
		{
			WaitForAllCallbacks(false); 
		}
		
		// Waits for all work items incl. low priority
		//
		void WaitForAllCurrentlyRunning()
		{
			WaitForAllCallbacks(true); 
		}

	};

}