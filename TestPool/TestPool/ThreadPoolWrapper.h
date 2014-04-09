#pragma once

#include <glog/logging.h>
#include <windows.h>

#include "WorkItem.h"

using namespace std;

class ThreadPoolWrapper
{
public:
	ThreadPoolWrapper(void);
	~ThreadPoolWrapper(void);

	bool Init();																		//init resources related in thread pool apis
	bool SetThreadCount(const DWORD thread_min_count, const DWORD thread_max_count);	//set min/max threadpool count
	void WaitCallbackFinish();															//wait for callback function to complete

	template <typename T>
	void SetCallback(T func, PVOID param);															//set callback

private:

	enum CountType
	{
		MAX_COUNT = 0,
		MIN_COUNT = 1,
	};

	bool _SetCount(const CountType count_type, const DWORD count);

	TP_CALLBACK_ENVIRON		callback_env_;
	PTP_POOL				pool_;
	PTP_CLEANUP_GROUP		cleanup_group_;
	PTP_WORK_CALLBACK		callback_func_;
	PTP_WORK				work_;
};


template <typename T>
void ThreadPoolWrapper::SetCallback(T func, PVOID param)
{
	WorkItem<T>* work_item = new WorkItem<T>(func, param, &callback_env_);
	if(work_item == nullptr)
	{
		LOG(FATAL) << "Function = " << __FUNCTION__;
		return;
	}

	if(!work_item->StartWork())
	{
		LOG(FATAL) << "Function = " << __FUNCTION__;
		return;
	}
}