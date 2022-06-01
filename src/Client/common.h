#pragma once
#define WINDOWSPLATFROM                 //Cross PlatFrom Setting
#ifdef WINDOWSPLATFROM
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<sys/socket.h>
#endif //WINDOWSPLATFROM

#define PORT 8834

#include<iostream>
#include<string>
#include<mutex>
#include<vector>
#include<queue>
#include<list>
#include<functional>
#include<thread>