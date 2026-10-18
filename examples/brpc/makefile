all: sync_server sync_client async_client async_server http_client http_server
sync_server: sync_server.cc cal.pb.cc
	g++ -std=c++17 $^ -o $@ -lbrpc -lleveldb -lprotobuf -lpthread -ldl -lssl -lcrypto -lgflags
sync_client: sync_client.cc cal.pb.cc
	g++ -std=c++17 $^ -o $@ -lbrpc -lleveldb -lprotobuf -lpthread -ldl -lssl -lcrypto -lgflags
async_client: async_client.cc cal.pb.cc
	g++ -std=c++17 $^ -o $@ -lbrpc -lleveldb -lprotobuf -lpthread -ldl -lssl -lcrypto -lgflags
async_server: async_server.cc cal.pb.cc
	g++ -std=c++17 $^ -o $@ -lbrpc -lleveldb -lprotobuf -lpthread -ldl -lssl -lcrypto -lgflags
http_client: http_client.cc cal.pb.cc
	g++ -std=c++17 $^ -o $@ -lbrpc -lleveldb -lprotobuf -lpthread -ldl -lssl -lcrypto -lgflags
http_server: http_server.cc cal.pb.cc
	g++ -std=c++17 $^ -o $@ -lbrpc -lleveldb -lprotobuf -lpthread -ldl -lssl -lcrypto -lgflags

%.pb.cc: %.proto
	protoc --cpp_out=./ $^
clean:
	rm -rf sync_server sync_client async_client async_server http_client http_server *.pb.*