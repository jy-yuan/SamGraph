all: main

main: main.o Graph.o IEStop.o
	mpicxx -pthread main.o Graph.o IEStop.o -o main

main.o:
	mpicxx -pthread -c main.cpp

Graph.o: Graph.cpp Graph.hpp
	g++ -std=c++11 -c -O2 Graph.cpp

IEStop.o: IEStop.cpp IEStop.hpp
	g++ -std=c++11 -c -O2 IEStop.cpp

clean:
	rm -f *.o main
