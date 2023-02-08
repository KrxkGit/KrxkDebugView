// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DEBUGVIEWDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// DEBUGVIEWDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#include"APIHook.h"

VOID CALLBACK MyOutputDebugStringW(LPCWSTR);//替换OutputDebugStringW
VOID CALLBACK MyOutputDebugStringA(LPCSTR);
VOID CALLBACK MyExitThread(DWORD dwExitCode);
VOID CALLBACK MyExitProcess(UINT uExitCode);
BOOL CALLBACK MyTerminateProcess(HANDLE,DWORD);
BOOL CALLBACK MyTerminateThread(HANDLE hThread,DWORD dwExitcode);
BOOL CALLBACK MyEndDialog(HWND,INT_PTR);
INT CALLBACK MyMessageBoxW(HWND,LPCWSTR,LPCWSTR,UINT);
INT CALLBACK MyMessageBoxA(HWND,LPCSTR,LPCSTR,UINT);
VOID DllInit();
