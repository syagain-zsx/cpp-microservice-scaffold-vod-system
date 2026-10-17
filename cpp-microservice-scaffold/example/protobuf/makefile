serialize: serialize.cc person.pb.cc
	g++ -std=c++17 $^ -o $@ -lprotobuf -lpthread
%.pb.cc: %.proto
	protoc -I ./ --cpp_out=. $^
.PHONY: clean
clean:
	rm -f serialize *.pb.*