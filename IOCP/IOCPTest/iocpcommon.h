#pragma once

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <algorithm>
#include <memory>

#define CLOSE_HANDLE(x) if(x != nullptr && x != INVALID_HANDLE_VALUE) { CloseHandle(x); x = nullptr; }
#define CLOSE_SOCKET(x) if(x != INVALID_SOCKET) { closesocket(x); x = INVALID_SOCKET; }
#define DELETE_PTR_SINGLE(x) if(x != nullptr) { delete x; x = nullptr; }
#define DELETE_PTR_ARRAY(x) if(x != nullptr) { delete[] x; x = nullptr; }
#define SOCKADDR_PTR(x) reinterpret_cast<sockaddr*>(x)
#define SOCKADDR_PTR_PTR(x) reinterpret_cast<LPSOCKADDR*>(x)

typedef int ERROR_CODE;

static const int nSOCKADDR_LEN = sizeof(SOCKADDR_IN);
static const int nIPV4_LEN = 32;

#ifdef NEED_CLOSE
#undef NEED_CLOSE
#endif // NEED_CLOSE

//需要手动关闭，防止内存泄漏
#define NEED_CLOSE

#ifdef _DLL_EXPORT
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT 
#endif // _DLL_EXPORT