CC=g++
CFLAGS= -c -Wall

all:hello

hello: mapreduce.o imagemr.o 
	$(CC) mapreduce.o imagemr.o -o image.out -pthread -std=c++11
mapreduce.o: mapreduce.cpp
	$(CC) $(CFLAGS) mapreduce.cpp -pthread -std=c++11

imagemr.o: imagemr.cpp
	$(CC) $(CFLAGS) imagemr.cpp -pthread -std=c++11

clean:
	rm -rf *o hello
