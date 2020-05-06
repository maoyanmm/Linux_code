CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -ljsoncpp -lncurses

SVRBIN=../bin/svr_out
CLIBIN=../bin/cli_out

.PHONY:all
all:$(SVRBIN) $(CLIBIN)
$(SVRBIN):Server.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)
$(CLIBIN):Client.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY:clean
clean:
	rm $(SVRBIN) $(CLIBIN)

