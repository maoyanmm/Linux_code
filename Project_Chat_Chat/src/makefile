CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -ljsoncpp

SVRBIN=./out_svr

.PHONY:all
all:$(SVRBIN)
$(SVRBIN):Server.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY:clean
clean:
	rm $(SVRBIN)

