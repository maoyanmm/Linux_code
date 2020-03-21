all:cli srv

cli:client.cpp
	g++ $^ -o $@ -lpthread
srv:srv_thread.cpp
	g++ $^ -o $@ -lpthread
