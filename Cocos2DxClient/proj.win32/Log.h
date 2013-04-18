#pragma once

// uncomment this when thread-safty is unnecessary
//#define LOG_THREAD_SAFE

namespace Log
{
	void Init();
	void Shutdown();

	void Log(const char * msg, ...);

	void Error(const char* fileName, const char* funcName, int line, const char* msg, ...);
	void Error(const char* fileName, const char* funcName, int line, int code, const char* msg, ...);

	void EnableTrace(bool enable);
}

#define LOG(msg, ...) Log::Log(msg, __VA_ARGS__);
#define ERROR_MSG(msg, ...) Log::Error(__FILE__, __FUNCTION__, __LINE__, msg, __VA_ARGS__);
#define ERROR_CODE(code, msg, ...) Log::Error(__FILE__, __FUNCTION__, __LINE__, code, msg, __VA_ARGS__);
