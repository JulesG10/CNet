#pragma once

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


#pragma comment (lib, "Ws2_32.lib")
#define CNetSocket SOCKET


#ifdef CNET_EXPORTS
#define CNET_EXPORTS __declspec(dllexport)
#else
#define CNET_EXPORTS __declspec(dllimport)
#endif


#include <iostream>
#include <algorithm>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>




