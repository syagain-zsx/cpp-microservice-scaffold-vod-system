all: client server
client: client.cc
	g++ -std=c++17 $^ -o $@ -lpthread
server: server.cc
	g++ -std=c++17 $^ -o $@ -lpthread
clean:
	rm -rf server client
