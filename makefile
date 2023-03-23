build: 
	g++ 	server/server.cpp     login/login.cpp    common/common.cpp -o server/server && \
	g++    	client/client.cpp     common/common.cpp   -o   client/client 
