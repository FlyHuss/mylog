#include <iostream>
#include <time.h>//日期
#include <unistd.h>//部分系统调用
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include "MyLog.h"
#include <stdio.h>
#include <cstring>
#include <assert.h>

MyMutexPre* MyLog::mutex_=new MyMutexPre();//mutex的初始化,要放在MyLog::Get_instance();的前面
MyLog* MyLog::my_log_obj_=MyLog::Get_instance();

MyLog::MyLog():write_size_(0),runing_(true){
	char buf[256]={'\0'};
	readlink("/proc/self/exe",buf,sizeof(buf)-1);
	logfile_name=std::string(strrchr(buf, '/') + 1);

	std::string filename=Getfilename();

	m_fd_=open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);//读写，创建，接在后面写等权限
	assert(m_fd_!=-1);
	cond_=new Condition(*MyLog::mutex_);
}

/*MyLog::~MyLog(){
	std::cout<<"~MyLog"<<std::endl;
}*/

std::string MyLog::Getfilename(){
	std::string filename="./LogFile/";//每个日志文件的名字为  进程名字argv[0]+日期时间+机器名称+进程id+.log 
	filename+=logfile_name;//文件名中的时间需要精确到微秒，不然短时间内文件写满打开新文件可能会打开原来的文件
	filename+='.';
	//get time
	timespec time;
    clock_gettime(CLOCK_REALTIME, &time);  //获取相对于1970到现在的秒数
    struct tm nowTime;
    char buf[64]={'\0'};
    localtime_r(&time.tv_sec, &nowTime);
    {
    	int year=nowTime.tm_year + 1900;
    	sprintf(buf,"%d",year);
    	filename+=buf;
    }
    nowTime.tm_mon+=1;
    if(nowTime.tm_mon>9){
		buf[2]={'\0'};
  	    sprintf(buf,"%d",nowTime.tm_mon);
   		filename+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_mon);
   		filename+=buf;
	}
	if(nowTime.tm_mday>9){
  	    sprintf(buf,"%d",nowTime.tm_mday);
   		filename+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_mday);
   		filename+=buf;
	}
	filename+='-';
	if(nowTime.tm_hour>9){
  	    sprintf(buf,"%d",nowTime.tm_hour);
   		filename+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_hour);
   		filename+=buf;
	}
	if(nowTime.tm_min>9){
  	    sprintf(buf,"%d",nowTime.tm_min);
   		filename+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_min);
   		filename+=buf;
	}
	if(nowTime.tm_sec>9){
  	    sprintf(buf,"%d",nowTime.tm_sec);
   		filename+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_sec);
   		filename+=buf;
	}
	filename+='.';
	{
		const int64_t kNanoSecondsPerSecond = 1e9;
		long usecond=static_cast<long>(((time.tv_nsec ) % kNanoSecondsPerSecond)/1e3);
		sprintf(buf,"%ld",usecond);
		filename+=buf;
	}
	filename+='.';
	{
		memset(buf,0,sizeof(buf));
		gethostname(buf,sizeof(buf));
		filename+=buf;
		filename+='.';
		long mpid=getpid();
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%ld",mpid);
		filename+=buf;
	}
	filename+=".log";
	return filename;
}

MyLog* MyLog::Get_instance(){
	if(MyLog::my_log_obj_!=NULL)//这样改了以后，除了第一次创建日志对象需要锁，其他时候都不需要了。这样write_mutex也可以省了
		return MyLog::my_log_obj_;
	else{
		MyMutex mutex_lock(*(MyLog::mutex_));
		if(MyLog::my_log_obj_==NULL){//这里还是要判断NULL，因为可能会有多个线程同时创建。"双检测机制"
			MyLog::my_log_obj_=new MyLog();

			for(int i=0;i<LOGBUFNUM;i++){
				MyLog::my_log_obj_->empty_bufs_.push_back(new LogBuffer());
			}
			if(MyLog::my_log_obj_->empty_bufs_.empty())
				MyLog::my_log_obj_->empty_bufs_.push_back(new LogBuffer());
			MyLog::my_log_obj_->current_buf_=MyLog::my_log_obj_->empty_bufs_.pop_back();

			int r=atexit(MyLog::ProcessExit);
			assert(r==0);
			r=pthread_create(&(MyLog::my_log_obj_->back_thread_),NULL,MyLog::my_log_obj_->back,MyLog::my_log_obj_);
			assert(r==0);
		}
	}
	return MyLog::my_log_obj_;
}

void MyLog::front(const char *str,std::string file_,int line_){
	char line_buf[10]={'\0'};
	memset(line_buf,0,10);
	sprintf(line_buf,"%d",line_);

	std::string log_message;
	timespec time;
    clock_gettime(CLOCK_REALTIME, &time);  //获取相对于1970到现在的秒数
    struct tm nowTime;
    char buf[60]={'\0'};
    localtime_r(&time.tv_sec, &nowTime);
    {
    	int year=nowTime.tm_year + 1900;
    	sprintf(buf,"%d",year);
    	log_message+=buf;
    }
    nowTime.tm_mon+=1;
    if(nowTime.tm_mon>9){
		buf[2]={'\0'};
  	    sprintf(buf,"%d",nowTime.tm_mon);
   		log_message+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_mon);
   		log_message+=buf;
	}
	if(nowTime.tm_mday>9){
  	    sprintf(buf,"%d",nowTime.tm_mday);
   		log_message+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_mday);
   		log_message+=buf;
	}
	log_message+="  ";
	if(nowTime.tm_hour>9){
  	    sprintf(buf,"%d",nowTime.tm_hour);
   		log_message+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_hour);
   		log_message+=buf;
	}
	log_message+=":";
	if(nowTime.tm_min>9){
  	    sprintf(buf,"%d",nowTime.tm_min);
   		log_message+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_min);
   		log_message+=buf;
	}
	log_message+=":";
	if(nowTime.tm_sec>9){
  	    sprintf(buf,"%d",nowTime.tm_sec);
   		log_message+=buf;
    }
    else{
  	    sprintf(buf,"%d%d",0,nowTime.tm_sec);
   		log_message+=buf;
	}
	log_message+="  ";
	log_message+=str;
	log_message+=" - ";
	log_message+=file_;
	log_message+=":";
   	log_message+=line_buf;
	log_message+="\n";
	Append(log_message.c_str(),log_message.length());
}

void MyLog::Append(const char *str,int len){
	MyMutex mutex_lock(*(MyLog::mutex_));
	if(current_buf_->LeftSize()>len){
		current_buf_->Write(str,len);
	}
	else{
		full_bufs_.push_back(current_buf_.release());
		if(!empty_bufs_.empty()){
			current_buf_=empty_bufs_.pop_back();
		}
		else{
			empty_bufs_.push_back(new LogBuffer());
			current_buf_=empty_bufs_.pop_back();
		}
		current_buf_->Write(str,len);
		cond_->notify();
	}
}

void* MyLog::back(void* obj){
	MyLog *mylog=static_cast<MyLog*>(obj);
	boost::ptr_deque<LogBuffer> write_bufs;
	while(mylog->runing_){
		{
			MyMutex mutex_lock(*(MyLog::mutex_));
			if((mylog->full_bufs_).empty()){
				(mylog->cond_)->WaitForTime(FLUSHTIME);//wait 3 sec
			}
			if((mylog->current_buf_)->LeftSize()==BUFSIZE&&(mylog->full_bufs_).empty())//此时没有要写入磁盘的东西，继续等待
				continue;
			else if(mylog->current_buf_->LeftSize()!=BUFSIZE){
				(mylog->full_bufs_).push_back((mylog->current_buf_).release());
				if(!(mylog->empty_bufs_).empty()){
					(mylog->current_buf_)=(mylog->empty_bufs_).pop_back();
				}
				else{
					(mylog->empty_bufs_).push_back(new LogBuffer());
					(mylog->current_buf_)=(mylog->empty_bufs_).pop_back();
				}
			}
			write_bufs.transfer(write_bufs.end(),(mylog->full_bufs_).begin(),(mylog->full_bufs_).end(),(mylog->full_bufs_));
		}
		//写文件放在临界区的外面
		if(!write_bufs.empty()){
			boost::ptr_deque<LogBuffer>::auto_type write_ptr_=write_bufs.pop_front();
			while(write_ptr_){
				if(write_ptr_->p_used_size_+mylog->write_size_<FILESIZE){
					int n=(*write_ptr_).Flush(mylog->m_fd_,FILESIZE-mylog->write_size_);
					(mylog->write_size_)+=n;
				}
				else{//有可能新打开的文件，因为设置的文件滚动大小太小，还是写不完buf的内容，所以这里还需要有个判断和循环
					int n=(*write_ptr_).Flush(mylog->m_fd_,FILESIZE-mylog->write_size_);
					close((mylog->m_fd_));
					std::string filename=(*mylog).Getfilename();
					(mylog->m_fd_)=open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
					assert(mylog->m_fd_!=-1);
					while(n!=0){
						n=(*write_ptr_).Flush(mylog->m_fd_,FILESIZE);
						if(n>=FILESIZE){
							close((mylog->m_fd_));
							std::string filename=(*mylog).Getfilename();
							(mylog->m_fd_)=open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
							assert(mylog->m_fd_!=-1);
						}
						else{
							mylog->write_size_=n;
							break;
						}
					}
				}
				(*write_ptr_).Clear();
				if((mylog->empty_bufs_).size()<LOGBUFNUM)
					(mylog->empty_bufs_).push_back(write_ptr_.release());
				else
					write_ptr_.release();
				if(!write_bufs.empty())
					write_ptr_=write_bufs.pop_front();
			}
		}
	}
}

void MyLog::ProcessExit(){
	MyLog *mylog=MyLog::Get_instance();
	mylog->runing_=false;
	mylog->cond_->notify();
	pthread_join(mylog->back_thread_,0);
	mylog->Flush();
	close(mylog->m_fd_);
	delete mylog->cond_;
	delete MyLog::mutex_;
	delete MyLog::my_log_obj_;
}

void MyLog::Flush(){
	boost::ptr_deque<LogBuffer> write_bufs;
	{
		MyMutex mutex_lock(*(MyLog::mutex_));
		if(current_buf_->LeftSize()==BUFSIZE&&full_bufs_.empty())
			return;
		else if(current_buf_->LeftSize()!=BUFSIZE){
			full_bufs_.push_back(current_buf_.release());
		}
		write_bufs.transfer(write_bufs.end(),(full_bufs_).begin(),(full_bufs_).end(),(full_bufs_));
	}
	if(!write_bufs.empty()){
		boost::ptr_deque<LogBuffer>::auto_type write_ptr_=write_bufs.pop_front();
		while(write_ptr_){
			if(write_ptr_->p_used_size_+write_size_<FILESIZE){
				int n=(*write_ptr_).Flush(m_fd_,FILESIZE-write_size_);
				(write_size_)+=n;
			}
			else{
				int n=(*write_ptr_).Flush(m_fd_,FILESIZE-write_size_);
				close(m_fd_);
				std::string filename=Getfilename();
				m_fd_=open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
				assert(m_fd_!=-1);
				while(n!=0){
					n=(*write_ptr_).Flush(m_fd_,FILESIZE);
					if(n>=FILESIZE){
						close(m_fd_);
						std::string filename=Getfilename();
						m_fd_=open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
						assert(m_fd_!=-1);
					}
					else{
						write_size_=n;
						break;
					}
				}
			}
			(*write_ptr_).Clear();
			if((empty_bufs_).size()<LOGBUFNUM)
				(empty_bufs_).push_back(write_ptr_.release());
			else
				write_ptr_.release();
			if(!write_bufs.empty())
				write_ptr_=write_bufs.pop_front();
		}
	}
}
