#pragma once
#include"stdafx.h"

#ifdef RUNNING_DLL
#define RUNNING_DLL __declspec(dllexport)
#else 
#define RUNNING_DLL __declspec(dllimport)
#endif


class RUNNING_DLL CRunning
{
public:
	static VOID CloseProcess();
	static VOID KeepRunning();
private:
	static UINT CALLBACK MyThread(LPVOID p);
public:
	struct MyProcess
	{
		HANDLE hProcess;
		TCHAR szPath[MAX_PATH];
	}u;
};