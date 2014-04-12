#include "WorkItem.h"

template <typename T>
WorkItem<T>::WorkItem(const T func, PVOID param, const PTP_CALLBACK_ENVIRON callback_env) 
	: func_(func)
	, param_(param)
	, callback_env_(callback_env)
	, work_(nullptr) 
{
}


template <typename T>
bool WorkItem<T>::StartWork()
{
	work_ = CreateThreadpoolWork(callback, this, callback_env_); 
	if(work_ == nullptr)
	{
		PRINT_ERROR();
		return false;
	}

	SubmitThreadpoolWork(work_);
	return true;
}


template <typename T>
void CALLBACK WorkItem<T>::callback (PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work)
{
	UNREFERENCED_PARAMETER(instance);
	UNREFERENCED_PARAMETER(work);

	WorkItem<T>* work_item = reinterpret_cast<WorkItem<T>*>(param);
	if(work_item == nullptr)
	{
		PRINT_ERROR();
		return;
	}

	work_item->func_(instance, work_item->param_, work);
	delete work_item;

	return; 
}
