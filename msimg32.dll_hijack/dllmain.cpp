// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#pragma comment(linker, "/EXPORT:vSetDdrawflag=_AheadLib_vSetDdrawflag,@1")
#pragma comment(linker, "/EXPORT:AlphaBlend=_AheadLib_AlphaBlend,@2")
#pragma comment(linker, "/EXPORT:DllInitialize=_AheadLib_DllInitialize,@3")
#pragma comment(linker, "/EXPORT:GradientFill=_AheadLib_GradientFill,@4")
#pragma comment(linker, "/EXPORT:TransparentBlt=_AheadLib_TransparentBlt,@5")

PVOID pfnAheadLib_vSetDdrawflag;
PVOID pfnAheadLib_AlphaBlend;
PVOID pfnAheadLib_DllInitialize;
PVOID pfnAheadLib_GradientFill;
PVOID pfnAheadLib_TransparentBlt;

static HMODULE	g_OldModule = NULL;

// 加载原始模块
__inline BOOL WINAPI Load()
{
	TCHAR tzPath[MAX_PATH];
	//TCHAR tzTemp[MAX_PATH * 2];
	GetSystemDirectory(tzPath, MAX_PATH); // 这里是否从系统目录加载或者当前目录，自行修改

	lstrcat(tzPath, TEXT("\\msimg32.dll"));
	g_OldModule = LoadLibrary(tzPath);
	if (g_OldModule == NULL)
	{
		//wsprintf(tzTemp, TEXT(), tzPath);
		ZL_LOG1("无法找到模块 %s,程序无法正常运行", tzPath);
		//MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
	}

	return (g_OldModule != NULL);
}

// 释放原始模块
__inline VOID WINAPI Free()
{
	if (g_OldModule)
	{
		FreeLibrary(g_OldModule);
	}
}

// 获取原始函数地址
FARPROC WINAPI GetAddress(PCSTR pszProcName)
{
	FARPROC fpAddress;
	CHAR szProcName[128];
	//TCHAR tzTemp[MAX_PATH];

	fpAddress = GetProcAddress(g_OldModule, pszProcName);
	if (fpAddress == NULL)
	{
		if (HIWORD(pszProcName) == 0)
		{
			wsprintfA(szProcName, "%d", pszProcName);
			pszProcName = szProcName;
		}

		//wsprintf(tzTemp, TEXT(), pszProcName);
		ZL_LOG1("无法找到函数 %S,程序无法正常运行", pszProcName);
		//MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		ExitProcess(-2);
	}
	return fpAddress;
}

// 初始化获取原函数地址
BOOL WINAPI Init()
{
	if (NULL == (pfnAheadLib_vSetDdrawflag = GetAddress("vSetDdrawflag")))
		return FALSE;
	if (NULL == (pfnAheadLib_AlphaBlend = GetAddress("AlphaBlend")))
		return FALSE;
	if (NULL == (pfnAheadLib_DllInitialize = GetAddress("DllInitialize")))
		return FALSE;
	if (NULL == (pfnAheadLib_GradientFill = GetAddress("GradientFill")))
		return FALSE;
	if (NULL == (pfnAheadLib_TransparentBlt = GetAddress("TransparentBlt")))
		return FALSE;
	return TRUE;
}

//补丁函数
DWORD WINAPI ThreadProc(LPVOID lpThreadParameter)
{
	using namespace std;
	ZL_LOG("Begin to hack wincc....");
	//x0打开进程,得到进程的句柄
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());//要获取所有的权限
	if (hProcess == 0)
	{
		ZL_LOG("Open CCLicenseService.exe's process FAILED!!!");
		//cout << "Open CCLicenseService.exe's process FAILED!!!" << endl;
		return FALSE;
	}
	//0x1查询内存数据
	byte const arr[7] = { 0x8D,0xD3,0xFD,0xFF,0xFF,0x85,0xC9 };//特征码(74之前的7个字节做为特征码)
	MEMORY_BASIC_INFORMATION mbi;
	DWORD dSize = 0x0;
	DWORD fromAddress = 0x40000;//开始搜索的地址
	DWORD FileSize = 0;
	DWORD VRA = 0x0;
	BOOL bFoundTheVRA = 0x0;
	DWORD VA = 0x0;
	do
	{
		dSize = VirtualQueryEx(hProcess, reinterpret_cast<PVOID>(fromAddress), &mbi, sizeof(mbi));
		fromAddress += mbi.RegionSize;
		if ((mbi.Type == MEM_IMAGE))//查找image页面
		{
			SIZE_T size = 0;
			byte* buffer = new byte[mbi.RegionSize];
			byte* bufferFirst = buffer;
			BOOL bRet = ReadProcessMemory(hProcess, mbi.BaseAddress, buffer, mbi.RegionSize, &size);//读取相邻的多个页
			if (!bRet)//读取相邻的多个页
			{
				delete[] bufferFirst;
				continue;//如果读取失败,那么继续下一次循环
			}
			for (VRA = 0; VRA < size - 7; VRA++)
			{
				int _pos = 0;
				for (_pos = 0; _pos < 7; _pos++)//对比特征码
				{
					if ((*(buffer + _pos)) != arr[_pos])
					{
						break;
					}
				}
				if (_pos == 7)//如果等于7,那么表示已经找到了特征码
				{
					bFoundTheVRA = TRUE;//标志位置位
					VA = (DWORD)mbi.BaseAddress + VRA + 7;
					break;
				}
				buffer++;
			}
			delete[] bufferFirst;
		}

	} while ((dSize == 28) && (!bFoundTheVRA));
	if (bFoundTheVRA)
	{
		ZL_LOG1("find the characteristic code at 0x%p", VA);
	}
	else
	{
		ZL_LOG("cant find the characteristic code");
		return FALSE;
	}
	//0x2读取和修改内存数据
	byte v = 0xEB;
	SIZE_T size = 0;
	//bool bRet = WriteProcessMemory(hProcess, (LPVOID)0x00412967, &w, 1, &size);
	BOOL bRet = WriteProcessMemory(hProcess, (LPVOID)VA, &v, 1, &size);
	if (!bRet)
	{
		ZL_LOG1("WriteProcessMemory Error:%d", GetLastError());
		//cout << "WriteProcessMemory Error:" << GetLastError() << endl;
		return FALSE;
	}
	ZL_LOG("Sucessfully Hacked!");
	//cout << "Sucessfully Completed!" << endl << "Press Enter to Exit!";
	CloseHandle(hProcess);
	return TRUE;
}

//入口点
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		if (Load() && Init())
		{
			TCHAR szAppName[MAX_PATH] = TEXT("CCLicenseService.exe");	//Dll宿主文件名
			TCHAR szFullPath[MAX_PATH] = { 0 };
			int nLength = 0;
			nLength = GetModuleFileName(NULL, szFullPath, MAX_PATH);
			PathStripPath(szFullPath);
			if (StrCmpI(szAppName, szFullPath) == 0 ) //这里是否判断宿主进程名
			{
				CreateThread(NULL, NULL, ThreadProc, NULL, NULL, NULL); //打补丁线程
			}
		}
		else
		{
			return FALSE;
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Free();
	}

	return TRUE;
}

// 导出函数
EXTERN_C __declspec(naked) void __cdecl AheadLib_vSetDdrawflag(void)
{
	__asm jmp pfnAheadLib_vSetDdrawflag;//这一句的背后有太多的底层的知识
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_AlphaBlend(void)
{
	__asm jmp pfnAheadLib_AlphaBlend;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_DllInitialize(void)
{
	__asm jmp pfnAheadLib_DllInitialize;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_GradientFill(void)
{
	__asm jmp pfnAheadLib_GradientFill;
}

EXTERN_C __declspec(naked) void __cdecl AheadLib_TransparentBlt(void)
{
	__asm jmp pfnAheadLib_TransparentBlt;
}