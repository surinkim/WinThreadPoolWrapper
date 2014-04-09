#include "WorkItem.h"

//template <typename T>
//WorkItem<T>::WorkItem(const T func, const PTP_CALLBACK_ENVIRON callback_env) 
//	: func_(func)
//	, callback_env_(callback_env)
//	, work_(nullptr) 
//{
//}


//template <typename T>
//bool WorkItem<T>::StartWork()
//{
//	work_ = CreateThreadpoolWork(callback, this, callback_env_); 
//	if(work_ == nullptr)
//	{
//		LOG(FATAL) << "Function = " << __FUNCTION__ <<", GetLastError = " << GetLastError();
//		return false;
//	}
//
//	SubmitThreadpoolWork(work_);
//	return true;
//}


//template <typename T>
//void CALLBACK WorkItem<T>::callback (PTP_CALLBACK_INSTANCE Instance, PVOID Param, PTP_WORK work)
//{
//	UNREFERENCED_PARAMETER(Instance);
//	UNREFERENCED_PARAMETER(work);
//
//	WorkItem<T>* work_item = reinterpret_cast<WorkItem<T>*>(Param);
//	if(work_item == nullptr)
//	{
//		LOG(FATAL) << "Function = " << __FUNCTION__;
//		return;
//	}
//
//	work_item->func_();
//	delete work_item;
//
//	return; 
//}
