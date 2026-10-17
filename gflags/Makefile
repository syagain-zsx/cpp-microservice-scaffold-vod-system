CXXFLAGS = -Wall -std=c++14
LDFLAGS  = -lgflags

main : main.cc child.cc child.h
	g++ $(CXXFLAGS) main.cc child.cc -o main $(LDFLAGS)

.PHONY : clean
clean :
	rm -f main
