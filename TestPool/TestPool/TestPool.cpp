// TestPool.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <algorithm>
#include "ThreadPoolWrapper.h"

using namespace std;

int g_found_count = 0;
SRWLOCK g_lock;

struct ParamInfo
{
	ParamInfo(const int min, const int max) : min_number_(min), max_number_(max) {};
	const int min_number_;
	const int max_number_;
};


VOID CALLBACK MyWorkCallback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work)
{
	if(parameter == nullptr) { return; }

	ParamInfo* param_info = static_cast<ParamInfo*>(parameter);
	for(int i = param_info->min_number_; i < param_info->max_number_; i++)
	{
		if((i % 2) == 1)
		{
			AcquireSRWLockExclusive(&g_lock);
			cout << "[thread id = " << GetCurrentThreadId() << "] find number = " << i << endl;
			g_found_count++;
			cout << "[found count = " << g_found_count << "]" << endl;
			ReleaseSRWLockExclusive(&g_lock);
		}
		Sleep(100);
	}

	return;
}


int main(int argc, char* argv[])
{
	//Init glog
	google::InitGoogleLogging("pool_test");

	FLAGS_log_dir = ".\\";

	//Init SRWLOCK object
	InitializeSRWLock(&g_lock);

	//Init ThreadPoolWrapper
	ThreadPoolWrapper wrapper;
	wrapper.Init();
	
	//Set Min/Max Thread Count
	wrapper.SetThreadCount(1, 3);

	//put callback with pram into the Threadpool
	ParamInfo infos[] = {ParamInfo(1, 50), ParamInfo(51, 100)};
	for_each(std::begin(infos), std::end(infos), [&](ParamInfo& info)
	{
		wrapper.SetCallback(MyWorkCallback, static_cast<PVOID>(&info));
	});

	//Wait for Callback finish
	wrapper.WaitCallbackFinish();

	getchar();

	return 0;
}

