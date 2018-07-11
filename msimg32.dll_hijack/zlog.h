#pragma once  
#include <string>  
using std::string;
using std::wstring;

class zLog
{
public:
	wchar_t  LogPath[260];
	zLog();
	~zLog(void);
	void    Write(const char* pSourcePath, const char* pFunName, const long lLine, const char* pLogText);
	void    Write(const char* pSourcePath, const char* pFunName, const long lLine, const wchar_t* pLogText);
	void    ScanfWrite(const char* pSourcePath, const char* pFunName, const long lLine, const char* pLogText, ...);
	void    ScanfWrite(const char* pSourcePath, const char* pFunName, const long lLine, const wchar_t* pLogText, ...);
protected:
	wstring  _LogFile;
	string  GetTime();
	string  U2A(const wstring& str);
	void	UpdateLogFileName(wchar_t* logPath);
	//void	UpdateLogFileName();
};

extern zLog   g_log;

#define ZL_LOG(x)							g_log.Write(__FILE__, __FUNCTION__, __LINE__, x)
#define ZL_LOG1(x, p1)					g_log.ScanfWrite(__FILE__, __FUNCTION__, __LINE__, x, p1)
#define ZL_LOG2(x, p1, p2)			g_log.ScanfWrite(__FILE__, __FUNCTION__, __LINE__, x, p1, p2)
#define ZL_LOG3(x, p1, p2, p3)		g_log.ScanfWrite(__FILE__, __FUNCTION__, __LINE__, x, p1, p2, p3)
