CC = gcc
XX = g++
CFLAGS = -Wall -O 

TARGET = ./sysmonitor
INC_SYSMONITOR = ../Sysmonitor/
INC_LIBEVENT = ../libevent/include/ 
INC_MYSQL = ../database/mysql/include/
#INC_ORACLE = ../database/oracle/include/

%.o: %.c

	$(CC) $(CFLAGS) -c -I$(INC_LIBEVENT)  $< -o $@

%.o:%.cpp

	$(XX) $(CFLAGS) -c -I$(INC_SYSMONITOR) -I$(INC_LIBEVENT)  -I$(INC_MYSQL)$< -o $@

 

SOURCES = $(wildcard *.c *.cpp )
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

LIBPATH_LIBEVENT = -L../libevent/lib
LIBPATH_MYSQL = -L../database/mysql/lib
LIB_LIBEVENT = -levent -levent_core -levent_extra 
LIB_MYSQL = -lmysqlclient
$(TARGET) : $(OBJS)
	$(XX) $(OBJS) $(LIBPATH_LIBEVENT) $(LIB_LIBEVENT) $(LIBPATH_MYSQL) $(LIB_MYSQL) -lm -lrt -o $(TARGET)  

	chmod a+x $(TARGET)

clean:

	rm -rf *.o sysmonitor

