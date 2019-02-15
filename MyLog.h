#ifndef MYLOG_H
#define MYLOG_H
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include "syn/MyMutex.h"
#include "syn/MyEvent.h"
#include <string>
#include "LogBuffer.h"
#include "LogFront.h"
#define LOGBUFNUM 4//初始化有4个空buf，后面不够用会再自行分配，空的>3个了会释放
#define FILESIZE 1024*1024*1024//测试滚动功能，定义文件大小64字节，实际可以定义1G
#define FLUSHTIME 3

class MyLog :boost::noncopyable
{
public:
	static MyLog* Get_instance();//只创建一个对象的办法，构造函数放在private，主动调用静态函数Get_instance,并且只在第一次创建，
	void front(const char *str,std::string file_,int line_);//前端用于组装日志格式，写入buf，通知后台
	static MyLog *my_log_obj_;
	static MyMutexPre *mutex_;//并且这个互斥量需要比Get_instance提前使用，用于第一次创建日志的互斥
private:
	//ptr_container不能有复制构造函数和赋值表达式，因为容器是独占的，不这样做会编译出错，
	//尼玛，这么重要的东西不早说，浪费我好几天时间。我就说怎么一直编译出错，而且错误还写的那么恶心，看不懂
	MyLog(MyLog&);
	void operator=(MyLog&);

	//static void Get_Mutex();//因为这个互斥量是用来Get_instance，日志对象还没创建之前用的，所以创建mutex也要是一个static函数
	static void ProcessExit();
	std::string Getfilename();//获取日志文件名。调用open函数打开文件需要的第一个参数
	MyLog();//构造函数和析构函数变成private，防止用户随意创建对象，因为构造函数私有，只能在类的内部调用
	~MyLog(){};
	void Append(const char *str,int len);//由前端调用，写入buf,并通知后台
	void Flush();
	static void* back(void* obj);//后台用于写文件，可能会阻塞，所以单独开一个线程来运行，并且每3秒会刷一次缓存，在getinstance里面开启
private:
	int m_fd_;
	//static变量必须要在.cpp文件里面初始化，不然会被链接器认为重复定义（.h文件是用来声明用的）
	//static bool process_exit_flag_;//如果为true,则说明ProcessExit()已经调用，缓存已经没了，应该直接写到文件而不是缓存,由写mutex保护
	boost::ptr_vector<LogBuffer> empty_bufs_;//这里用栈可以利用局部性原理
	boost::ptr_deque<LogBuffer> full_bufs_;//队列，先来的消息当然先写
	boost::ptr_vector<LogBuffer>::auto_type current_buf_;
	Condition *cond_;//创建一个不带计数的event，用于前台通知后台
	int write_size_;//用来控制写入文件的大小，文件滚动
	bool runing_;//可以用来控制while死循环线程的开始与停止，很好用
	pthread_t back_thread_;
	std::string logfile_name;
};

#define LOGIN__
#ifdef LOGIN__
#define LOGIN (LogFront(__FILE__,__LINE__))
#else
#define LOGIN
#endif

#endif

/*
没有考虑多进程的情况
没有考虑日志输出的级别(用宏+if的方法实现，输出日志的级别比当前日志对象级别高就不管，<=才写在if里)
*/
