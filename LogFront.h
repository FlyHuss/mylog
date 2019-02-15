#ifndef LOGFRONT_H_
#define LOGFRONT_H_
#include <string>
#include "MyLog.h"

class LogFront:boost::noncopyable
{
public:
	LogFront(const std::string& str,int num):file_(str),line_(num){};
	~LogFront(){};
	LogFront& operator<<(const char *str);
	LogFront& operator<<(const std::string &str);
	LogFront& operator<<(int num);
private:
	LogFront(LogFront&);
	void operator=(LogFront&);
private:
	const std::string file_;
	const int line_;
};

#endif