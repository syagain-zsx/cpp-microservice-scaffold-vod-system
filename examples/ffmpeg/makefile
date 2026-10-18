all: hls server
hls:hls.cc
	g++ -g -std=c++17 $^ -o $@ -lavcodec -lavformat -lavutil
server:server.cc
	g++ -std=c++17 $^ -o $@ -lpthread
clean:
	rm -rf hls server