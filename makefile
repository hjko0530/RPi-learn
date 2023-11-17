all: *.cpp
	g++ TCP_client_socket.cpp -o TCP_client_socket.o
	g++ TCP_server_socket.cpp -o TCP_server_socket.o
	g++ fork.cpp -o fork.o
run_client: TCP_client_socket.cpp
	./TCP_client_socket.o 0.0.0.0
run_server: TCP_server_socket.cpp
	./TCP_server_socket.o
run_fork: fork.cpp
	./fork.o
.PHONY:clean
clean:
	rm *.o

	