#pragma once
#define _WIN3264                 //Cross PlatFrom Setting
#ifdef _WIN3264   
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
using SOCKADDR_IN = sockaddr_in;
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#endif //_WIN3264   

#define PORT 8834

#include<iostream>
#include<string>
#include<mutex>
#include<vector>
#include<thread>
#include<future>
