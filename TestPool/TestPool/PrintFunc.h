#pragma once

#include <iostream>
#include <sstream>

using namespace std;

#define LOG_FATAL() TraceFunc(__FILE__, __FUNCTION__, __LINE__);


#ifdef _DEBUG
#define ASSERT_CUSTOM(file, function, line) assert(false);
#else
#define ASSERT_CUSTOM(file, function, line) AssertExit(file, function, line);
#endif


static void AssertExit(const char* file_name, const char* function, int line)
{
	if(file_name == nullptr || function == nullptr) { return;}

	std::ostringstream str_stream;
	str_stream << "Assert Failed in " << endl << endl
		<<"File Name : " << file_name << endl 
		<<"Function : " << function << endl 
		<<"Line : " << line;

	std::string msg = str_stream.str();

	MessageBoxA(NULL, msg.c_str(), NULL, MB_OK);
	exit(-1);

	return;
}


static void TraceFunc(const char* file_name, const char* function, int line) 
{
	if(file_name == nullptr || function == nullptr) { return; }

	std::cout << " Source : " << file_name << endl << " Function Name : " << function << endl << " Line : " << line << endl << endl;
	ASSERT_CUSTOM(file_name, function, line);
}