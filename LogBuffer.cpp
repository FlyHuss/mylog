#include "LogBuffer.h"
#include <string.h>//memcpy
#include <unistd.h>
#include <assert.h>

void LogBuffer::Write(const char*str,int len){
	memcpy(buf_+used_size_,str,len*sizeof(char));
	used_size_+=len;
}

int LogBuffer::LeftSize(){
	return BUFSIZE-used_size_;
}

/*int LogBuffer::WriteLine(int fd){
	if(Buf_EOF())
		return 0;
	int len=0;
	while(*line_ptr_!='\n'){
		line_ptr_++;
		len++;
	}
	line_ptr_++;
	len++;
	write(fd,line_ptr_-len,len);
	return len;
}*/

/*bool LogBuffer::Buf_EOF(){
	return line_ptr_-buf_>=used_size_;
}*/

void LogBuffer::Clear(){
	used_size_=0;
	part_ptr_=buf_;
}

int LogBuffer::Flush(int fd,int left_file_size){
	if(part_ptr_-buf_>=used_size_)
		return 0;
	int left_used_size_=used_size_-(part_ptr_-buf_);
	if(left_file_size>left_used_size_){
		int r=write(fd,part_ptr_,left_used_size_);
		assert(r!=-1);
		part_ptr_+=left_used_size_;
		return r;
	}
	else{
		char *p=part_ptr_;
		p+=left_file_size;
		while(*p!='\n'){
			p++;
		}
		p++;
		int len=p-part_ptr_;
		int r=write(fd,part_ptr_,len);
		assert(r!=-1);
		part_ptr_=p;
		return r;
	}
}