all:
	clang++ main.cpp -lssl -lcrypto -lpthread -lboost_system -std=c++14
