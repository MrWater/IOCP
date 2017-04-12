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
static int cnt = 0;
inline void 
iocp::CIocp::__OnAccept(IOCP_CONTEXT& context)
{
	WaitForSingleObject(m_hMutex, INFINITE);
	std::cout << context.m_szBuffer << "             "<< ++cnt << endl;
	ReleaseMutex(m_hMutex);
}

inline void 
iocp::CIocp::__OnRecv(IOCP_CONTEXT& context)
{
}

inline void 
iocp::CIocp::__OnSend(IOCP_CONTEXT& context)
{
}

inline bool
iocp::CIocp::__PostAccept(IOCP_CONTEXT_PTR pContext)
{
	DWORD dwBytes = 0;
	
	pContext->m_operationType = IOCP_OPERATION_TYPE::_ACCEPT;

	__ConstructSocket(pContext->m_socket);

	IOCP_SOCKETINFO_PTR pSocketInfo = new IOCP_SOCKETINFO;
	pSocketInfo->m_socket = pContext->m_socket;
	m_clients.construct(m_begin + ++m_nClient, pSocketInfo);

	if (m_lpfnAcceptEx != nullptr)
	{
		if (!m_lpfnAcceptEx(m_pSockinfoServer->m_socket,
			pContext->m_socket,
			&pContext->m_wsabuf,
			pContext->m_wsabuf.len - (2 * (nSOCKADDR_LEN + 16)),
			nSOCKADDR_LEN + 16,
			nSOCKADDR_LEN + 16,
			&dwBytes,
			&pContext->m_overlapped))
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				//TODO: ¸Ä³É·µ»Ø×´Ì¬Âë£¿
				return false;
			}

			return true;
		}

		return false;
	}

	return false;
}

inline bool
iocp::CIocp::__PostRecv(IOCP_CONTEXT_PTR pContext)
{
	return false;
}

inline bool
iocp::CIocp::__PostSend(IOCP_CONTEXT_PTR pContext)
{
	return false;
}

#endif