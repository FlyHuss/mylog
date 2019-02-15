maketest :./syn/Condition.o ./syn/MyMutex.o ./syn/MyMutexPre.o \
LogBuffer.o MyLog.o LogFront.o test2.o
	g++ ./syn/Condition.o ./syn/MyMutex.o ./syn/MyMutexPre.o \
LogBuffer.o MyLog.o LogFront.o test2.o -o maketest -lpthread -std=c++11

./syn/Condition.o:./syn/Condition.h

./syn/MyMutex.o:./syn/MyMutex.h

./syn/MyMutexPre.o:./syn/MyMutexPre.h

LogBuffer.o:LogBuffer.h

MyLog.o:MyLog.h

LogFront.o:LogFront.h

test2.o:

.PHONY:clean
clean:
	-rm *.o