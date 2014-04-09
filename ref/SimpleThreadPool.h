// File contains the SimpleThreadPool class and its helper classes 
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
		// Internal class which waits on events, registers events and re-registers events 
		// 
		template <class Function>
		class WaitCallBack
		{
		private: 
			const Function m_Func;
			PVOID state;
			PTP_WAIT wait;
			HANDLE m_Event; 
			int TimeOut; 

			// Callback which really calls the user-provided callback function
			// 
			static void CALLBACK callback (PTP_CALLBACK_INSTANCE Instance, PVOID Param, PTP_WAIT wait, TP_WAIT_RESULT WaitResult)
			{
				WaitCallBack<Function> *pc = reinterpret_cast<WaitCallBack<Function>*>(Param);
				pc->m_Func(pc->state);						
				return; 
			}

		public:
			// WaitCallBack Constructor accepts callback function, context parameter for the callback function, 
			// threadpool environment, event and timeout in milliseconds (-1 indicates infinite wait). 
			//
			WaitCallBack(const Function Func, PVOID st, PTP_CALLBACK_ENVIRON pEnv, HANDLE Event, int timeout) : m_Func(Func), state(st), m_Event(Event), TimeOut(timeout)  
			{			
				wait = CreateThreadpoolWait(callback, this, pEnv); 
				if (!wait)
				{
					throw "Error: Could not create threadpool wait object.";
				}
			
				if (timeout > -1)
				{
					FILETIME dueTime;
					*reinterpret_cast<PLONGLONG>(&dueTime) = -static_cast<LONGLONG>(MILLI_SECOND_TO_NANO100(timeout));
					SetThreadpoolWait(wait, Event, &dueTime);
				}
				else 
				{
					SetThreadpoolWait(wait, Event, NULL);	// timeout is -1 which indicates infinite timeout 
				}
			}

			// Reregister interest in event with a new timeout. 
			// NOTE: This does update the TimeOut value stored in the class when it was first created. 
			// 
			bool ReRegisterEvent(int timeout)
			{
				if (timeout > -1)
				{
					FILETIME dueTime;
					*reinterpret_cast<PLONGLONG>(&dueTime) = -static_cast<LONGLONG>(MILLI_SECOND_TO_NANO100(timeout));
					SetThreadpoolWait(wait, m_Event, &dueTime);
				}
				else 
				{
					SetThreadpoolWait(wait, m_Event, NULL); 
				}
				return true; 
			}
		
			// Reregister interest in event with the same timeout it was created with. 
			//
			bool ReRegisterEvent()
			{
				if (TimeOut > -1)
				{
					FILETIME dueTime;
					*reinterpret_cast<PLONGLONG>(&dueTime) = -static_cast<LONGLONG>(MILLI_SECOND_TO_NANO100(TimeOut));
					SetThreadpoolWait(wait, m_Event, &dueTime);
				}
				else 
				{
					SetThreadpoolWait(wait, m_Event, NULL); 
				}
				return true; 
			}

			// Destroy event
			//
			void DestroyEvent()
			{
				SetThreadpoolWait(wait, NULL, NULL); 
				return ; 
			}
		};

		// Internal class which submits the work items 
		template <class Function>
		class SimpleCallBack
		{
		private: 
			const Function m_Func;
			PVOID state;

			// Threadpool callback function which calls the user provided callback 
			// function
			//
			static void CALLBACK callback (PTP_CALLBACK_INSTANCE Instance, PVOID Param)
			{
				UNREFERENCED_PARAMETER(Instance);

				SimpleCallBack<Function> *pc = reinterpret_cast<SimpleCallBack<Function>*>(Param);
				pc->m_Func(pc->state);						
				delete pc; 
				return; 
			}

		public:
			// Constructor which accepts the callback function, context parameter to the callback and 
			// the threadpool environment 
			//
			SimpleCallBack(const Function Func, PVOID st, PTP_CALLBACK_ENVIRON pEnv) : m_Func(Func), state(st)
			{			
				if (!TrySubmitThreadpoolCallback(callback, this, pEnv))
				{
					throw "Error: Could not submit work item."; 
				}
			}

		};
	} 

	// SimpleThreadPool class uses the default process-wide threadpool 
	//
	class SimpleThreadPool 
	{
		private:
			TP_CALLBACK_ENVIRON CallbackEnvironment;
			PTP_CLEANUP_GROUP CleanupGroup;
			bool InfraInitialized; 
			
			void InitializeInfra()
			{
				// Function cant fail (I think) 
				// The reason to have a separate Initialize function is 
				// to avoid error if someone waits for callbacks
				// and then calls queueuserworkitem again. 
				//
				if (!InfraInitialized)
				{
					InitializeThreadpoolEnvironment(&CallbackEnvironment);
					CleanupGroup = CreateThreadpoolCleanupGroup();
					SetThreadpoolCallbackCleanupGroup(&CallbackEnvironment, CleanupGroup, NULL);
					InfraInitialized = true; 
				}
				return; 			
			}

			// Wait for all callbacks to complete and cancel any work items 
			// which havent started if required.
			//
			void WaitForAllCallbacks(bool CancelNotStarted)
			{
				assert(InfraInitialized == true); 
				CloseThreadpoolCleanupGroupMembers(CleanupGroup, CancelNotStarted, NULL);
				CloseThreadpoolCleanupGroup(CleanupGroup);
				InfraInitialized = false; 
			}

			// Queues the work item to the default process-wide ThreadPool 	
			//
			template <class Function>
			bool QueueUserWorkItemInternal(Function cb, PVOID State)
			{
				InitializeInfra(); 						
				windowsthreadpool::internal::SimpleCallBack<Function> *scb = new windowsthreadpool::internal::SimpleCallBack<Function>(cb, State, &CallbackEnvironment);		
				return true;
			}

			// Queues the work item to the default process-wide ThreadPool 	
			//
			template <class Function>
			bool QueueUserWorkItemInternal(Function cb, PVOID State, PTP_CALLBACK_ENVIRON env)
			{
				InitializeInfra(false); 						
				windowsthreadpool::internal::SimpleCallBack<Function> *scb = new windowsthreadpool::internal::SimpleCallBack<Function>(cb, State, env);		
				return true;
			}

			// Register the event and wait in the threadpool 
			//
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
			SimpleThreadPool (): InfraInitialized(false)
			{
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

			// Destroys the event 
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

			// Queues a work item to the end of the queue
			//
			template <class Function>
			bool QueueUserWorkItemWithLowPri(Function cb)
			{
				InitializeInfra(); 	
				TP_CALLBACK_ENVIRON LowPriCallbackEnvironment;
				InitializeThreadpoolEnvironment(&LowPriCallbackEnvironment);
				SetThreadpoolCallbackPriority(&LowPriCallbackEnvironment, TP_CALLBACK_PRIORITY_LOW); 
				SetThreadpoolCallbackCleanupGroup(&LowPriCallbackEnvironment, CleanupGroup, NULL);

				return QueueUserWorkItemInternal(cb, NULL, &LowPriCallbackEnvironment); 
			}

			// Queues a work item to the start of the queue
			//
			template <class Function>
			bool QueueUserWorkItemWithHiPri(Function cb)
			{
				InitializeInfra(); 
				TP_CALLBACK_ENVIRON HiPriCallbackEnvironment;
				InitializeThreadpoolEnvironment(&HiPriCallbackEnvironment);
				SetThreadpoolCallbackPriority(&HiPriCallbackEnvironment, TP_CALLBACK_PRIORITY_HIGH); 
				SetThreadpoolCallbackCleanupGroup(&HiPriCallbackEnvironment, CleanupGroup, NULL);

				return QueueUserWorkItemInternal(cb, NULL, &HiPriCallbackEnvironment); 
			}

			// Queues a work item to the end of the queue
			//
			template <class Function>
			bool QueueUserWorkItemWithLowPri(Function cb, PVOID State)
			{
				InitializeInfra(); 
				TP_CALLBACK_ENVIRON LowPriCallbackEnvironment;
				InitializeThreadpoolEnvironment(&LowPriCallbackEnvironment);
				SetThreadpoolCallbackPriority(&LowPriCallbackEnvironment, TP_CALLBACK_PRIORITY_LOW); 
				SetThreadpoolCallbackCleanupGroup(&LowPriCallbackEnvironment, CleanupGroup, NULL);

				return QueueUserWorkItemInternal(cb, State, &LowPriCallbackEnvironment); 
			}

			// Queues a work item to the start of the queue
			//
			template <class Function>
			bool QueueUserWorkItemWithHiPri(Function cb, PVOID State)
			{
				InitializeInfra(); 
				TP_CALLBACK_ENVIRON HiPriCallbackEnvironment;
				InitializeThreadpoolEnvironment(&HiPriCallbackEnvironment);
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