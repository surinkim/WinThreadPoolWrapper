// WinThreadPool.cpp : Test application that demonstrates all classes in namespace windowsthreadpool
// Author: Hari Pulapaka
//
#include <tchar.h>
#include <iostream> 
#include <Windows.h>
#include <assert.h>
#include <functional>

#include "WindowsThreadPool.h"

using namespace std; 

// I want to avoid users knowing anything about TP_* 
//
void CALLBACK PrintI(PVOID state)
{
	int *i = reinterpret_cast<int *> (state); 
	cout << *i << " ," << endl;	
}

void CALLBACK PrintRand(PVOID state)
{
	UNREFERENCED_PARAMETER(state); 
	
	cout << rand() << " ," << endl;	
}

void CALLBACK HelloRepeat(PVOID state)
{
	UNREFERENCED_PARAMETER(state); 

	cout << "Hello World - this should occur roughly every second." << endl; 
}

void CALLBACK HelloWorld2(PVOID state)
{
	UNREFERENCED_PARAMETER(state); 

	cout << "Hello World2." << endl; 
}

void CALLBACK HelloWorld(PVOID state)
{
	UNREFERENCED_PARAMETER(state); 

	cout << "Hello World." << endl; 
}

int _cdecl _tmain()
{
	using namespace windowsthreadpool; 

	SimpleThreadPool stp; 
	PrivateThreadPool ptp; 

	int arr[10]; 

	for(int i=0; i<10; ++i)
		arr[i] = i; 

	for(int i=0; i<10; ++i)
	{
		stp.QueueUserWorkItem(PrintI, &arr[i]);
		ptp.QueueUserWorkItem(PrintRand);
	}

	stp.WaitForAll(); 
	ptp.WaitForAll(); 

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == hEvent) 
	{
		throw "Could not create event.";	
    }

	PVOID handle = stp.RegisterEvent(HelloWorld2, hEvent, 100000); 

	SetEvent(hEvent);

	stp.DestroyEvent<THREADPOOLCALLBACK>(handle); 

	stp.WaitForAll(); 

	PVOID phandle = ptp.RegisterEvent(HelloWorld, hEvent, 100000); 

	SetEvent(hEvent);
	Sleep(100);

	ptp.ReRegisterEvent<THREADPOOLCALLBACK>(phandle); 

	SetEvent(hEvent);

	Sleep(100);

	ptp.DestroyEvent<THREADPOOLCALLBACK>(phandle); 

	ptp.WaitForAll(); 

	// Background threadpool - only one thread running
	// work items are executed in FIFO order 
	//
	PrivateThreadPool backgroundworker;
	backgroundworker.SetThreadpoolMax(1); 
	
	for(int i=0; i<10; ++i)
	{
		backgroundworker.QueueUserWorkItemWithLowPri(PrintI, &arr[i]);		
	}

	// NOTE: when the timer object goes out of scope, it destroys the timer. 
	// 
	Timer<THREADPOOLCALLBACK> *t = new Timer<THREADPOOLCALLBACK>(1000, 1000, HelloRepeat);  

	Sleep(10000);

	delete t;

	backgroundworker.WaitForAll(); 

	return 0;
}
