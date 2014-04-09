// TestPool.cpp : Defines the entry point for the console application.
//

#include <iostream>
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
	
	//Set Max/Min Thread Count
	wrapper.SetThreadCount(1, 3);

	//Put first Callback into the Threadpool with param
	ParamInfo info(1, 50);
	wrapper.SetCallback(MyWorkCallback, static_cast<PVOID>(&info));

	//Put second Callback into the Threadpool with param
	ParamInfo info2(51, 100);
	wrapper.SetCallback(MyWorkCallback, static_cast<PVOID>(&info2));

	//Wait for Callback finish
	wrapper.WaitCallbackFinish();

	getchar();

	return 0;
}

