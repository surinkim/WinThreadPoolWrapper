#include "ThreadPoolWrapper.h"

#include <limits>
#include <iostream>

#pragma comment(lib, "libglog.lib")

using namespace std;

ThreadPoolWrapper::ThreadPoolWrapper(void)
	: pool_(nullptr)
	, cleanup_group_(nullptr)
	, work_(nullptr)
{
	memset(&callback_env_, 0, sizeof(callback_env_));
}


ThreadPoolWrapper::~ThreadPoolWrapper(void)
{
	DestroyThreadpoolEnvironment(&callback_env_);

	if(cleanup_group_)
	{
		CloseThreadpoolCleanupGroup(cleanup_group_);
	}
	
	if(pool_)
	{
		CloseThreadpool(pool_);
	}
	
}


bool ThreadPoolWrapper::Init()
{
	//init 'callback_env_' object
	InitializeThreadpoolEnvironment(&callback_env_);

	//create 'pool_' object
	pool_ = CreateThreadpool(nullptr);
	if(pool_ == nullptr)
	{
		LOG(FATAL) << "Function = " << __FUNCTION__ <<", GetLastError = " << GetLastError();
		return false;
	}
	
	//put 'pool_' into the 'callback_env_.Pool'
	SetThreadpoolCallbackPool(&callback_env_, pool_);

	//create 'cleanup_group_' object for graceful threadpool destruction
	cleanup_group_ = CreateThreadpoolCleanupGroup();
	if(cleanup_group_ == nullptr)
	{
		LOG(FATAL) << "Function = " << __FUNCTION__ <<", GetLastError = " << GetLastError();
		return false;
	}

	//put 'cleanup_group_' into 'callback_env_.CleanupGroup'
	SetThreadpoolCallbackCleanupGroup(&callback_env_, cleanup_group_, nullptr);

	return true;
}


bool ThreadPoolWrapper::SetThreadCount(const DWORD thread_min_count, const DWORD thread_max_count)
{
	if(thread_max_count < thread_min_count)
	{
		LOG(FATAL) << "Invalid Threadpool Count : " << "max count = " << thread_max_count
			<< ", min count = " << thread_min_count;
		return false;
	}

	if(!_SetCount(MIN_COUNT, thread_min_count)
		|| !_SetCount(MAX_COUNT, thread_max_count))
	{
		return false;
	}

	return true;
}


void ThreadPoolWrapper::WaitCallbackFinish()
{
	//Since the second parameter is FALSE, when all work item finish, this will return.
	CloseThreadpoolCleanupGroupMembers(cleanup_group_, FALSE, nullptr);
}


bool ThreadPoolWrapper::_SetCount(const CountType count_type, const DWORD count)
{
	if(count_type == MAX_COUNT)
	{
		SetThreadpoolThreadMaximum(pool_, count);
	}
	else if(count_type == MIN_COUNT)
	{
		if(!SetThreadpoolThreadMinimum(pool_, count))
		{
			LOG(FATAL) << "Function = " << __FUNCTION__ <<", GetLastError = " << GetLastError();
			return false;
		}
	}
	else
	{
		LOG(ERROR) << "Function = " << __FUNCTION__;
		return false;
	}
	return true;

}

