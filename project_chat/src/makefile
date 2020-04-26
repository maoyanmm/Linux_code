CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -ljsoncpp
SVRBIN=../bin/ChatSvr

.PHONY:ChatSvr
$(SVRBIN):ChatServer.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY:clean
clean:
	rm $(SVRBIN)
