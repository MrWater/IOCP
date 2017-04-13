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
	StopService();
}

void 
CIocp::StartService(
	const ADDRESS_FAMILY& addressFamily, 
	const ADDRESS& address, 
	const PORT& port)
{
	if (m_bStarted)
		return;

	m_bStarted = true;

	//================================
	//初始化serveraddr
	m_pSockinfoServer->m_sockaddr.sin_addr.s_addr = address;
	m_pSockinfoServer->m_sockaddr.sin_family = addressFamily;
	m_pSockinfoServer->m_sockaddr.sin_port = port;
	//=================================

	__Bind();
	__Listen();

	if (!__PostAccept())
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
CIocp::StopService()
{
	if (m_bStoped)
		return;

	m_bStoped = true;
	m_bShutdown = true;


	if (nullptr != m_tWorkerThreads)
	{
		for (THREAD_NUM i = 0; i < m_unThreadNum; i++)
		{
			PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, nullptr);
		}
		
		for (THREAD_NUM i = 0; i < m_unThreadNum; i++)
		{
			m_tWorkerThreads[i].join();
		}
	}

	

	__Uninitialize();
}

void 
CIocp::__Initialize()
{
	m_bShutdown = false;
	m_bStoped = false;

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
	for (auto& client : m_listClients)
	{
		DELETE_PTR_SINGLE(client);
	}

	for (auto& item : m_listIoContextNeedtoFree)
	{
		DELETE_PTR_SINGLE(item);
	}

	CLOSE_HANDLE(m_hCompletionPort);
	DELETE_PTR_SINGLE(m_pSockinfoServer);
	DELETE_PTR_ARRAY(m_tWorkerThreads);
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
	if (nullptr == pIocp)
		return;

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
			//这里的pIoContext是在堆上申请的
			IOCP_IOCONTEXT_PTR pIoContext = CONTAINING_RECORD(pOverlapped, IOCP_IOCONTEXT, m_overlapped);

			if (nullptr == pIoContext)
				continue;

			if (0 == dwBytes && (IOCP_OPERATION_TYPE::_RECV == pIoContext->m_operationType ||
				IOCP_OPERATION_TYPE::_SEND == pIoContext->m_operationType))
			{
				//客户端断开连接
				pIocp->m_listClients.remove(pIoContext->m_pSocketInfo);

				continue;
			}
			else
			{
				switch (pIoContext->m_operationType)
				{
				case IOCP_OPERATION_TYPE::_ACCEPT:
					pIocp->__OnAccept(pIoContext);
					pIocp->__PostRecv(pIoContext->m_pSocketInfo);
					pIocp->__PostAccept();
					break;

				case IOCP_OPERATION_TYPE::_RECV:
					pIocp->__OnRecv(pIoContext);
					pIocp->__PostRecv(pIoContext->m_pSocketInfo);
					break;

				case IOCP_OPERATION_TYPE::_SEND:
					pIocp->__OnSend(pIoContext);
					break;

				default:
					break;
				}
			}

			//==================================
			//释放内存
			if (pIoContext != nullptr)
			{
				pIocp->m_mutex.lock();
				pIocp->m_listIoContextNeedtoFree.remove(pIoContext);
				DELETE_PTR_SINGLE(pIoContext);
				pIocp->m_mutex.unlock();

			}
			//==================================
		}
		else
		{
			cout << GetLastError() << endl;
		}
	}
}


