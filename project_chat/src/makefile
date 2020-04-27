CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -ljsoncpp

SVRBIN=../bin/ChatSvr
CLIBIN=../bin/ChatCli

.PHONY:all
all:$(SVRBIN) $(CLIBIN)
$(SVRBIN):ChatServer.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)
$(CLIBIN):ChatClient.cpp
	g++ $^ -o $@ $(CFLAGS)

.PHONY:clean
clean:
	rm $(SVRBIN) $(CLIBIN)
