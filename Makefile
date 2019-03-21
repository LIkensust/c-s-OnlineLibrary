.PHONY:all
all:ser cli bookjson

ser:service.cc
cli:client.cc

.PHONY:bookjson
bookjson:
	g++  ./client.cc  -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o new_cli.o  -D_LINUX_OS_ -lmysqlclient -lhiredis 
	g++   ./service.cc -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ser_epoll_pthread.o  -D_LINUX_OS_ -lpthread -std=c++11 -lmysqlclient -lhiredis 
	g++   -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ./make_test_json_file.o ./make_test_json_file.cc  -D_LINUX_OS_  
	g++   -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ./CJsonObject/CJsonObject.o ./CJsonObject/CJsonObject.cpp  -D_LINUX_OS_   
	gcc   -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ./CJsonObject/cJSON.o ./CJsonObject/cJSON.c  -D_LINUX_OS_ 
	g++ -g -o CreateJson ./make_test_json_file.o ./CJsonObject/CJsonObject.o ./CJsonObject/cJSON.o  -D_LINUX_OS_ -lmysqlclient -lhiredis 
	g++ -g -o ser ./ser_epoll_pthread.o ./CJsonObject/CJsonObject.o ./CJsonObject/cJSON.o  -D_LINUX_OS_ -lpthread -lmysqlclient -std=c++11 -lhiredis 
	g++ -g -o cli ./new_cli.o ./CJsonObject/CJsonObject.o ./CJsonObject/cJSON.o  -D_LINUX_OS_ -lmysqlclient -lhiredis
.PHONY:clean
clean:
	rm -f test.out
	rm -f make_test_json_file.o ./CJsonObject/CJsonObject.o ./CJsonObject./cJSON.o ser_epoll_pthread.o new_cli.o 
	rm -f CreateJson
	rm -f cli
	rm -f ser
