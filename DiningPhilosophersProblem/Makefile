CXXFLAGS = -Wall -Wextra -O2 -std=c++20

all: philosophers

philosophers: main.o
	g++ $(CXXFLAGS) main.o -o philosophers

main.o: main.cpp
	g++ $(CXXFLAGS) -c main.cpp

clean:
	rm *.o philosophers