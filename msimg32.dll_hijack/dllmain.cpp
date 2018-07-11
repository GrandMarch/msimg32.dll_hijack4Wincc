// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
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

// ����ԭʼģ��
__inline BOOL WINAPI Load()
{
	TCHAR tzPath[MAX_PATH];
	//TCHAR tzTemp[MAX_PATH * 2];
	GetSystemDirectory(tzPath, MAX_PATH); // �����Ƿ��ϵͳĿ¼���ػ��ߵ�ǰĿ¼�������޸�

	lstrcat(tzPath, TEXT("\\msimg32.dll"));
	g_OldModule = LoadLibrary(tzPath);
	if (g_OldModule == NULL)
	{
		//wsprintf(tzTemp, TEXT(), tzPath);
		ZL_LOG1("�޷��ҵ�ģ�� %s,�����޷���������", tzPath);
		//MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
	}

	return (g_OldModule != NULL);
}

// �ͷ�ԭʼģ��
__inline VOID WINAPI Free()
{
	if (g_OldModule)
	{
		FreeLibrary(g_OldModule);
	}
}

// ��ȡԭʼ������ַ
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
		ZL_LOG1("�޷��ҵ����� %S,�����޷���������", pszProcName);
		//MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		ExitProcess(-2);
	}
	return fpAddress;
}

// ��ʼ����ȡԭ������ַ
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

//��������
DWORD WINAPI ThreadProc(LPVOID lpThreadParameter)
{
	using namespace std;
	ZL_LOG("Begin to hack wincc....");
	//x0�򿪽���,�õ����̵ľ��
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());//Ҫ��ȡ���е�Ȩ��
	if (hProcess == 0)
	{
		ZL_LOG("Open CCLicenseService.exe's process FAILED!!!");
		//cout << "Open CCLicenseService.exe's process FAILED!!!" << endl;
		return FALSE;
	}
	//0x1��ѯ�ڴ�����
	byte const arr[7] = { 0x8D,0xD3,0xFD,0xFF,0xFF,0x85,0xC9 };//������(74֮ǰ��7���ֽ���Ϊ������)
	MEMORY_BASIC_INFORMATION mbi;
	DWORD dSize = 0x0;
	DWORD fromAddress = 0x40000;//��ʼ�����ĵ�ַ
	DWORD FileSize = 0;
	DWORD VRA = 0x0;
	BOOL bFoundTheVRA = 0x0;
	DWORD VA = 0x0;
	do
	{
		dSize = VirtualQueryEx(hProcess, reinterpret_cast<PVOID>(fromAddress), &mbi, sizeof(mbi));
		fromAddress += mbi.RegionSize;
		if ((mbi.Type == MEM_IMAGE))//����imageҳ��
		{
			SIZE_T size = 0;
			byte* buffer = new byte[mbi.RegionSize];
			byte* bufferFirst = buffer;
			BOOL bRet = ReadProcessMemory(hProcess, mbi.BaseAddress, buffer, mbi.RegionSize, &size);//��ȡ���ڵĶ��ҳ
			if (!bRet)//��ȡ���ڵĶ��ҳ
			{
				delete[] bufferFirst;
				continue;//�����ȡʧ��,��ô������һ��ѭ��
			}
			for (VRA = 0; VRA < size - 7; VRA++)
			{
				int _pos = 0;
				for (_pos = 0; _pos < 7; _pos++)//�Ա�������
				{
					if ((*(buffer + _pos)) != arr[_pos])
					{
						break;
					}
				}
				if (_pos == 7)//�������7,��ô��ʾ�Ѿ��ҵ���������
				{
					bFoundTheVRA = TRUE;//��־λ��λ
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
	//0x2��ȡ���޸��ڴ�����
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

//��ڵ�
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		if (Load() && Init())
		{
			TCHAR szAppName[MAX_PATH] = TEXT("CCLicenseService.exe");	//Dll�����ļ���
			TCHAR szFullPath[MAX_PATH] = { 0 };
			int nLength = 0;
			nLength = GetModuleFileName(NULL, szFullPath, MAX_PATH);
			PathStripPath(szFullPath);
			if (StrCmpI(szAppName, szFullPath) == 0 ) //�����Ƿ��ж�����������
			{
				CreateThread(NULL, NULL, ThreadProc, NULL, NULL, NULL); //�򲹶��߳�
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

// ��������
EXTERN_C __declspec(naked) void __cdecl AheadLib_vSetDdrawflag(void)
{
	__asm jmp pfnAheadLib_vSetDdrawflag;//��һ��ı�����̫��ĵײ��֪ʶ
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