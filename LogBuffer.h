#ifndef LOGBUFFER_H_
#define LOGBUFFER_H_
#define BUFSIZE 4096
#include<boost/noncopyable.hpp>

class LogBuffer:boost::noncopyable
{
public:
	LogBuffer():p_used_size_(used_size_),used_size_(0),part_ptr_(buf_){};
	~LogBuffer(){};
	void Write(const char*str,int len);//写入buf不溢出由日志库来控制
	int LeftSize();
	//int WriteLine(int fd);
	//bool Buf_EOF();
	void Clear();
	int Flush(int fd,int left_file_size);//Flush函数用来把缓存的内容写到文件中，如果文件剩余空间比要写的小，则只能写完刚超过文件大小的第一个\n的日志,返回实际写入的数量
	const int &p_used_size_;
private:
	char buf_[BUFSIZE];
	int used_size_;
	char *part_ptr_;
};

#endif
