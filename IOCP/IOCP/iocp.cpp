//======================================
//iocp.cpp iocp源文件
//======================================

#include <thread>
#include <algorithm>
#include <iostream>
#include "iocp.h"

using namespace iocp;
using namespace std;

#ifndef NDEBUG
#include "iocp.inl"
#endif // !NDEBUG


CIocp::CIocp()
{
	__Initialize();
}


CIocp::~CIocp()
{
	__Uninitialize();
}

void 
CIocp::Start(
	const ADDRESS_FAMILY& addressFamily, 
	const ADDRESS& address, 
	const PORT& port)
{
	//================================
	//初始化serveraddr
	m_pSockinfoServer->m_sockaddr.sin_addr.s_addr = address;
	m_pSockinfoServer->m_sockaddr.sin_family = addressFamily;
	m_pSockinfoServer->m_sockaddr.sin_port = port;
	//=================================

	__Bind();
	__Listen();

	IOCP_CONTEXT_PTR pContext = new IOCP_CONTEXT;
	if (!__PostAccept(pContext))
	{
		__Uninitialize();
		return;
	}
	
	for (THREAD_NUM i = 0; i < m_unThreadNum; i++)
	{
		m_tWorkerThreads[i] = thread(__WorkerThreadFunc, this);
	}
}

void
CIocp::Stop()
{
	m_bShutdown = true;

	if (nullptr != m_tWorkerThreads)
	{
		for (THREAD_NUM i = 0; i < m_unThreadNum; i++)
		{
			PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, nullptr);
			m_tWorkerThreads[i].join();
		}
	}

	__Uninitialize();
}

void 
CIocp::__Initialize()
{
	m_bShutdown = false;
	m_begin = m_clients.allocate(1);
	m_nClient = 0;
	m_hMutex = CreateMutex(NULL, FALSE, NULL);

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_unThreadNum = si.dwNumberOfProcessors * 2;
	m_tWorkerThreads = new thread[m_unThreadNum];

	m_hCompletionPort = nullptr;
	m_lpfnAcceptEx = nullptr;
	m_lpfnGetAcceptExSockaddrs = nullptr;

	WSADATA wsaData;
	DWORD dwBytes = 0;
	
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return;
	}

	m_pSockinfoServer = new IOCP_SOCKETINFO;

	__ConstructSocket(m_pSockinfoServer->m_socket);
	if (INVALID_SOCKET == m_pSockinfoServer->m_socket)
	{
		__Uninitialize();
		return;
	}

	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

	if (SOCKET_ERROR == WSAIoctl(
		m_pSockinfoServer->m_socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&dwBytes,
		nullptr,
		nullptr))
	{
		__Uninitialize();
		return;
	}

	if (SOCKET_ERROR == WSAIoctl(
		m_pSockinfoServer->m_socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockaddrs,
		sizeof(guidGetAcceptExSockaddrs),
		&m_lpfnGetAcceptExSockaddrs,
		sizeof(m_lpfnGetAcceptExSockaddrs),
		&dwBytes,
		nullptr,
		nullptr))
	{
		__Uninitialize();
		return;
	}

	//======================================
	//分别进行创建完成端口和绑定
	__CreateComletionPort();
	__BindCompletionPort(m_pSockinfoServer);
	//======================================
}

void
CIocp::__CreateComletionPort()
{
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

	if(nullptr == m_hCompletionPort)
	{
		__Uninitialize();
		return;
	}
}

void 
CIocp::__Uninitialize()
{
	int nClient = m_nClient;
	while (nClient != 0)
	{
		m_clients.destroy(m_begin + nClient--);
	}

	m_clients.deallocate(m_begin, m_nClient);

	CLOSE_SOCKET(m_pSockinfoServer->m_socket);
	CLOSE_HANDLE(m_hCompletionPort);
	CLOSE_HANDLE(m_hMutex);
	DELETE_PTR(m_pSockinfoServer);
	DELETE_PTR(m_tWorkerThreads);
	WSACleanup();
}

void 
CIocp::__Bind()
{
	if (SOCKET_ERROR == ::bind(m_pSockinfoServer->m_socket, SOCKADDR_PTR(&m_pSockinfoServer->m_sockaddr), nSOCKADDR_LEN))
	{
		__Uninitialize();
		return;
	}
}

void 
CIocp::__Listen()
{
	if (SOCKET_ERROR == listen(m_pSockinfoServer->m_socket, SOMAXCONN))
	{
		__Uninitialize();
		return;
	}
}

void
CIocp::__WorkerThreadFunc(CIocp* pIocp)
{
	DWORD dwBytes = 0;
	IOCP_SOCKETINFO_PTR pSocketInfo = nullptr;
	OVERLAPPED* pOverlapped = nullptr;

	while (!pIocp->m_bShutdown)
	{
		BOOL bRet = GetQueuedCompletionStatus(pIocp->m_hCompletionPort,
			&dwBytes,
			(PULONG_PTR)&pSocketInfo,
			&pOverlapped,
			WSA_INFINITE);

		//退出
		if (nullptr == pSocketInfo)
		{
			break;
		}

 		if (bRet)
		{
			IOCP_CONTEXT_PTR pContext = CONTAINING_RECORD(pOverlapped, IOCP_CONTEXT, m_overlapped);

			if (0 == dwBytes && (IOCP_OPERATION_TYPE::_RECV == pContext->m_operationType ||
				IOCP_OPERATION_TYPE::_SEND == pContext->m_operationType))
			{
				//TODO:删掉
				std::cout << "断开连接";
				continue;
			}
			else
			{
				IOCP_CONTEXT_PTR pNewContext = new IOCP_CONTEXT;

				switch (pContext->m_operationType)
				{
				case IOCP_OPERATION_TYPE::_ACCEPT:
					pIocp->__PostAccept(pNewContext);
					pIocp->__OnAccept(*pContext);
					break;

				case IOCP_OPERATION_TYPE::_RECV:
					pIocp->__OnRecv(*pContext);
					break;

				case IOCP_OPERATION_TYPE::_SEND:
					pIocp->__OnSend(*pContext);
					break;

				default:
					delete pNewContext;
					break;
				}
			}
		}
		else
		{
			cout << GetLastError() << endl;
			cout << WSAGetLastError() << endl;
		}
	}
}


