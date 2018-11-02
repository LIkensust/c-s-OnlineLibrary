.PHONY:all
all:test bookjson

.PHONY:test
test:
	g++ do_job.cc -o test.out

.PHONY:bookjson
bookjson:
	g++   -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ./make_test_json_file.o ./make_test_json_file.cc  -D_LINUX_OS_ 
	g++   -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ./CJsonObject/CJsonObject.o ./CJsonObject/CJsonObject.cpp  -D_LINUX_OS_ 
	gcc   -O2 -Wall -ggdb -m64 -D_GNU_SOURCE=1 -D_REENTRANT -D__GUNC__  -c -o ./CJsonObject/cJSON.o ./CJsonObject/cJSON.c  -D_LINUX_OS_ 
	g++ -g -o CreateJson ./make_test_json_file.o ./CJsonObject/CJsonObject.o ./CJsonObject/cJSON.o  -D_LINUX_OS_

.PHONY:clean
clean:
	rm -f test.out
	rm -f make_test_json_file.o ./CJsonObject/CJsonObject.o ./CJsonObject./cJSON.o 
	rm -f CreateJson

