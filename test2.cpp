#include <iostream>
#include "MyLog.h"

using namespace std;

/*#include <stdio.h>
#define DEBUG__
#ifdef DEBUG__
#define DEBUG(format,...) printf("FILE:"__FILE__"LINE:%d"__LINE__"format",##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif
int main(){
	char str[]="hello world!";
	DEBUG("%s",str);
	return 0;
}*/


int main(int arcg,char *argv[]){
	//MyLog* mlog=MyLog::Get_instance();
	LOGIN<<"1.hello world!"<<"2.hello world!";
	LOGIN<<"hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!hello world!";
	LOGIN<<"3.hello world!";
	//sleep(3);
	cout<<"main end!"<<endl;
	return 0;
}
