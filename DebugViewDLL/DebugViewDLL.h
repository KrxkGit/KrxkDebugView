// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� DEBUGVIEWDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// DEBUGVIEWDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#include"APIHook.h"

VOID CALLBACK MyOutputDebugStringW(LPCWSTR);//�滻OutputDebugStringW
VOID CALLBACK MyOutputDebugStringA(LPCSTR);
VOID CALLBACK MyExitThread(DWORD dwExitCode);
VOID CALLBACK MyExitProcess(UINT uExitCode);
BOOL CALLBACK MyTerminateProcess(HANDLE,DWORD);
BOOL CALLBACK MyTerminateThread(HANDLE hThread,DWORD dwExitcode);
BOOL CALLBACK MyEndDialog(HWND,INT_PTR);
INT CALLBACK MyMessageBoxW(HWND,LPCWSTR,LPCWSTR,UINT);
INT CALLBACK MyMessageBoxA(HWND,LPCSTR,LPCSTR,UINT);
VOID DllInit();
