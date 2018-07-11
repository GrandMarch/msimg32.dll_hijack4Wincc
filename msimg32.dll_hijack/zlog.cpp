#include "StdAfx.h"  
#include "zlog.h"  
#include <Windows.h>  

//全局日志文件
//该日志文件会在超过1024*1024后自动清空
const TCHAR* g_pLogFile =L"C:\\Windows\\Logs\\msimg32.dll.log";

zLog  g_log;
zLog::zLog(void)
{

	GetModuleFileName(NULL, LogPath, sizeof(LogPath)); ;//获取路径
}

zLog::~zLog(void)
{
}

void zLog::Write(const char* pSourcePath, const char* pFunName, const long lLine, const char* pLogText)
{
	if (pLogText == NULL)return;
	int nLogLen = strlen(pLogText);
	if (nLogLen == 0)return;
	int nSourceLen = strlen(pSourcePath);
	int nFunLen = strlen(pFunName);
	char szLine[10] = { 0 };
	sprintf_s(szLine, "%ld", lLine);
	int nLineLen = strlen(szLine);
	//int nSpaceLen = 80 - nSourceLen - nFunLen - nLineLen;//nSourceLen + nFunLen + nLineLen>80?
	string strTime = GetTime();
	FILE* fp = NULL;

	//DWORD nBufferLength = 260;
	//WCHAR buffer[260] = { 0 };


	//UpdateLogFileName(LogPath);
	//_wfopen_s(&fp, _LogFile.c_str(), L"a+");
	_wfopen_s(&fp, g_pLogFile, L"a+");
	fseek(fp, 0, SEEK_END);
	long  size = ftell(fp);
	if (size >1* 1024 * 1024)
	{
		fclose(fp);
		_wfopen_s(&fp, g_pLogFile, L"w");
		if(fp)fclose(fp);
		_wfopen_s(&fp, g_pLogFile, L"a+");
	}
	//errno_t err=	fopen_s(&fp, _LogFile.c_str(), "a+");
	fwrite(strTime.c_str(), strTime.size(), 1, fp);
	fwrite(" ", 1, 1, fp);
	fwrite(pSourcePath, nSourceLen, 1, fp);
	//for (int i = 0; i<nSpaceLen; ++i)
	fwrite(" ", 1, 1, fp);
	fwrite(pFunName, nFunLen, 1, fp);
	fwrite("() ", 1, 3, fp);
	fwrite(szLine, nLineLen, 1, fp);
	fwrite(":", 1, 1, fp);
	fwrite(pLogText, nLogLen, 1, fp);
	fwrite("\n", 1, 1, fp);
	fclose(fp);
}

void zLog::Write(const char* pSourcePath, const char* pFunName, const long lLine, const wchar_t* pLogText)
{
	string strLogText = U2A(pLogText);
	Write(pSourcePath, pFunName, lLine, strLogText.c_str());
}

void zLog::ScanfWrite(const char* pSourcePath, const char* pFunName, const long lLine, const char* pLogText, ...)
{
	va_list pArgs;
	va_start(pArgs, pLogText);
	char szBuffer[1024] = { 0 };
	_vsnprintf_s(szBuffer, 1024, pLogText, pArgs);
	va_end(pArgs);
	Write(pSourcePath, pFunName, lLine, szBuffer);
}

void zLog::ScanfWrite(const char* pSourcePath, const char* pFunName, const long lLine, const wchar_t* pLogText, ...)
{
	va_list pArgs;
	va_start(pArgs, pLogText);
	wchar_t szBuffer[1024] = { 0 };
	_vsnwprintf_s(szBuffer, 1024, pLogText, pArgs);
	va_end(pArgs);
	Write(pSourcePath, pFunName, lLine, szBuffer);
}

string zLog::GetTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	char szTime[26] = { 0 };
	sprintf_s(szTime, "%04d-%02d-%02d %02d:%02d:%02d.%d  ", st.wYear, st.wMonth, st.wDay, st.wHour,
		st.wMinute, st.wSecond, st.wMilliseconds);
	return szTime;
}

string zLog::U2A(const wstring& str)
{
	string strDes;
	if (!str.empty())
	{
		int nLen = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
		if (0 != nLen)
		{
			char* pBuffer = new char[nLen + 1];
			memset(pBuffer, 0, nLen + 1);
			WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), pBuffer, nLen, NULL, NULL);
			pBuffer[nLen] = '\0';
			strDes.append(pBuffer);
			delete[] pBuffer;
		}
	}
	return strDes;
}

void zLog::UpdateLogFileName(wchar_t* logPath)
{
	//WCHAR szPath[MAX_PATH];
	wstring _log_file;
	wchar_t szFile[MAX_PATH] = { 0 };
	int index = 0;
	while (*logPath != '\0')
	{
		szFile[index] = *logPath;
		logPath++;
		index++;
	}
	_log_file.append(szFile);
	SYSTEMTIME st;
	GetLocalTime(&st);
	wchar_t szTime[15];
	swprintf_s(szTime, L"%04d-%02d-%02d.log", st.wYear, st.wMonth, st.wDay);
	//sprintf_s(szTime, "%04d-%02d-%02d.log", st.wYear, st.wMonth, st.wDay);
	_log_file.append(szTime);
	_LogFile = _log_file;
}