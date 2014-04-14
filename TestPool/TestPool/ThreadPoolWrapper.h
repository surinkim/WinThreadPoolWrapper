#pragma once

#include <assert.h>
#include <windows.h>

#include "PrintFunc.h"
#include "WorkItem.h"
#include "WorkItem.cpp"

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
	bool SetCallback(T func, PVOID param);															//set callback

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
bool ThreadPoolWrapper::SetCallback(T func, PVOID param)
{
	WorkItem<T>* work_item = new WorkItem<T>(func, param, &callback_env_);
	if(work_item == nullptr)
	{
		LOG_FATAL();
		return false;
	}

	if(!work_item->StartWork())
	{
		LOG_FATAL();
		return false;
	}

	return true;
}