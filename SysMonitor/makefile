CC = gcc
XX = g++
CFLAGS = -Wall -O 

TARGET = ./sysmonitor
INC = ../libevent/include/ 
%.o: %.c

	$(CC) $(CFLAGS) -c -I$(INC) $< -o $@

%.o:%.cpp

	$(XX) $(CFLAGS) -c -I$(INC) $< -o $@

 

SOURCES = $(wildcard *.c *.cpp)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

LIBPATH = -L../libevent/lib
LIB = -levent -levent_core -levent_extra 

$(TARGET) : $(OBJS)
	$(XX) $(OBJS) $(LIBPATH) $(LIB) -lm -lrt -o $(TARGET)  

	chmod a+x $(TARGET)

clean:

	rm -rf *.o sysmonitor

