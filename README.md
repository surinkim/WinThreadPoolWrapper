Introduction
====================

WinThreadPoolWrapper is a simple wrapper for windows thread pool apis, such as `CreateThreadpool()`.  
Using by this, we don't need to know detaily those windows thread pool apis.


How to use
====================

This project is written in VS2010 and google log.

First, you should write your own work thread. 
For exsample, below code is work thread which prints odd numbers between min and max number.
Let's say this function's name is `MyWorkCallback()`.

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


And then, we simply use above function using by `ThreadPoolWrapper`.
Like this, in your main,

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



References
====================

 - http://msdn.microsoft.com/en-us/library/windows/desktop/ms686980(v=vs.85).aspx  
 - http://blogs.msdn.com/b/harip/archive/2010/10/11/introduction-to-the-windows-threadpool-part-1.aspx  
 - Windows via C/C++(5th edition) by Jeffrey Richter


Author
====================

 - web : http://attractiveapproach.blogspot.com  
 - mail : nnhope@hotmail.com  
 - stackoverflow : http://stackoverflow.com/users/2231098/codedreamer


