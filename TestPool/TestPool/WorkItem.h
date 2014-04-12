#pragma once

#include <windows.h>

template <typename T>
class WorkItem
{
public:
	WorkItem(const T func, PVOID param, const PTP_CALLBACK_ENVIRON callback_env);
	bool StartWork();

	static void CALLBACK callback (PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work);

private:
	const T						func_;
	PVOID						param_;
	const PTP_CALLBACK_ENVIRON	callback_env_;
	PTP_WORK					work_;
};
