build: 
	g++ 	server/server.cpp     login/login.cpp    common/common.cpp -o -std=c++17 -lstdc++fs server/server && \
	g++    	client/client.cpp     common/common.cpp   -o -std=c++17  -lstdc++fs client/client 
