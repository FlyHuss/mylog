#include "LogFront.h"
#include <stdio.h>

LogFront& LogFront::operator<<(const char *str){
	MyLog::my_log_obj_->front(str,file_,line_);
	return *this;
}

LogFront& LogFront::operator<<(const std::string &str){
	MyLog::my_log_obj_->front(str.c_str(),file_,line_);
	return *this;
}

LogFront& LogFront::operator<<(int num){
	char buf[60]={0};
	sprintf(buf,"%d",num);
	MyLog::my_log_obj_->front(buf,file_,line_);
	return *this;
}