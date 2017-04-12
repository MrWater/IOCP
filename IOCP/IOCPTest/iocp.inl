//==================================
//iocp.inl 内联文件，用于选择性内联
//==================================


#ifndef _IOCP_INL_
#define _IOCP_INL_

#include "iocp.h"

inline void
iocp::CIocp::__ConstructSocket(SOCKET& socket)
{
	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == socket)
	{
		__Uninitialize();
		return;
	}
}

inline void 
iocp::CIocp::__BindCompletionPort(IOCP_SOCKETINFO_PTR pSocketInfo)
{
	if (nullptr == pSocketInfo)
		return;

	if (nullptr == CreateIoCompletionPort((HANDLE)pSocketInfo->m_socket,
		m_hCompletionPort,
		(ULONG_PTR)pSocketInfo,
		0))
	{
		__Uninitialize();
		return;
	}
}

inline void 
iocp::CIocp::__OnAccept(IOCP_IOCONTEXT_PTR pIoContext)
{
	SOCKADDR_IN* clientAddr = nullptr;
	SOCKADDR_IN* serverAddr = nullptr;
	int nLocalLen = nSOCKADDR_LEN;
	int nRemoteLen = nSOCKADDR_LEN;

	m_lpfnGetAcceptExSockaddrs(pIoContext->m_wsabuf.buf,
		pIoContext->m_wsabuf.len - 2 * nIPV4_LEN,
		nIPV4_LEN,
		nIPV4_LEN,
		SOCKADDR_PTR_PTR(&clientAddr),
		&nLocalLen,
		SOCKADDR_PTR_PTR(&serverAddr),
		&nRemoteLen);

	//绑定完成端口，进行异步IO，不要忘记！！！！！
	__BindCompletionPort(pIoContext->m_pSocketInfo);

	pIoContext->m_pSocketInfo->m_sockaddr = *clientAddr;
	std::cout << pIoContext->m_wsabuf.buf << std::endl;
}

inline void 
iocp::CIocp::__OnRecv(IOCP_IOCONTEXT_PTR pIoContext) const
{
	std::cout << pIoContext->m_wsabuf.buf << std::endl;
}

inline void 
iocp::CIocp::__OnSend(IOCP_IOCONTEXT_PTR pIoContext) const
{
}

inline bool
iocp::CIocp::__PostAccept()
{
	DWORD dwBytes = 0;

	IOCP_SOCKETINFO_PTR pSocketInfo = new IOCP_SOCKETINFO;
	m_listClients.push_back(pSocketInfo);

	IOCP_IOCONTEXT_PTR pIoContext = new IOCP_IOCONTEXT;
	pIoContext->m_operationType = IOCP_OPERATION_TYPE::_ACCEPT;
	pIoContext->m_pSocketInfo = pSocketInfo;
	m_listIoContextNeedtoFree.push_back(pIoContext);

	__ConstructSocket(pSocketInfo->m_socket);

	if (m_lpfnAcceptEx != nullptr)
	{
		//数据包括源地址和目的地址以及数据，所以IPV4减去两个32
		if (!m_lpfnAcceptEx(m_pSockinfoServer->m_socket,
			pSocketInfo->m_socket,
			pIoContext->m_wsabuf.buf,
			pIoContext->m_wsabuf.len - (2 * nIPV4_LEN),
			nIPV4_LEN,
			nIPV4_LEN,
			&dwBytes,
			&pIoContext->m_overlapped))
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				//TODO: 改成返回状态码？
				__Uninitialize();
				return false;
			}

			return true;
		}

		__Uninitialize();
		return false;
	}

	__Uninitialize();
	return false;
}

inline bool
iocp::CIocp::__PostRecv(IOCP_SOCKETINFO_PTR pSocketInfo)
{
	IOCP_IOCONTEXT_PTR pIoContext = new IOCP_IOCONTEXT;
	pIoContext->m_pSocketInfo = pSocketInfo;
	pIoContext->m_operationType = IOCP_OPERATION_TYPE::_RECV;
	m_listIoContextNeedtoFree.push_back(pIoContext);

	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	
	if (SOCKET_ERROR == WSARecv(pSocketInfo->m_socket,
		&pIoContext->m_wsabuf,
		1,
		&dwBytes,
		&dwFlags,
		&pIoContext->m_overlapped,
		nullptr))
	{
 		if (WSA_IO_PENDING != WSAGetLastError())
		{
			__Uninitialize();
			return false;
		}
	}

	return true;
}

inline bool
iocp::CIocp::__PostSend(IOCP_SOCKETINFO_PTR pSocketInfo)
{
	return false;
}

#endif