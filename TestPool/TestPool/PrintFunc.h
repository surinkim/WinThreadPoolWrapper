#pragma once

#include <iostream>
using namespace std;

#define PRINT_ERROR()	{												\
							cout << "ERROR!! - " << endl;				\
							TraceFunc(__FILE__, __FUNCTION__, __LINE__); \
						}

static void TraceFunc(const char* file_name, const char* function, int line) 
{
	std::cout << " Source : " << file_name << endl << " Function Name : " << function << endl << " Line : " << line << endl << endl;
}
