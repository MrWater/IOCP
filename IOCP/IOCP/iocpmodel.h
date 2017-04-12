#pragma once

#include <memory>

#include "iocpcommon.h"

#define MAX_BUFFER_LEN 4096

namespace iocp
{
	typedef void(__stdcall *COMPLETION_CALLBACK)(\
		unsigned long, \
		unsigned long, \
		LPWSAOVERLAPPED, \
		unsigned long);


	/*重叠IO和完成回调函数*/
	typedef struct tagIocpOverlappedWithEvent
	{
		//重叠io指针
		LPWSAOVERLAPPED m_overlapped;
		//io完成回调
		COMPLETION_CALLBACK m_callback;
	} IOCP_OVERLAPPED_WITH_EVENT, *IOCP_OVERLAPPED_WITH_EVENT_PTR;


	typedef enum class enumIocpOperationType
	{
		_NONE,
		_ACCEPT,
		_SEND,
		_RECV
	} IOCP_OPERATION_TYPE;


	/*iocp中io的结构*/
	typedef struct tagIocpContext
	{
		OVERLAPPED m_overlapped;
		WSABUF m_wsabuf;
		char m_szBuffer[MAX_BUFFER_LEN];
		IOCP_OPERATION_TYPE m_operationType;
		SOCKET m_socket;

		tagIocpContext()
		{
			ZeroMemory(&m_overlapped, sizeof(m_overlapped));
			ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
			
			m_wsabuf.buf = m_szBuffer;
			m_wsabuf.len = MAX_BUFFER_LEN;

			m_operationType = IOCP_OPERATION_TYPE::_NONE;
			m_socket = INVALID_SOCKET;
		}

		~tagIocpContext()
		{
			CLOSE_SOCKET(m_socket);
		}
	} IOCP_CONTEXT, *IOCP_CONTEXT_PTR;


	/*Socket信息*/
	typedef struct tagIocpSocketInfo
	{
		SOCKET m_socket;
		SOCKADDR_IN m_sockaddr;

		tagIocpSocketInfo()
		{
			m_socket = INVALID_SOCKET;
			//ZeroMemory(&m_sockaddr, nSOCKADDR_LEN);
			memset(&m_sockaddr, 0, nSOCKADDR_LEN);
		}

		~tagIocpSocketInfo()
		{
			CLOSE_SOCKET(m_socket);
		}
	} IOCP_SOCKETINFO, *IOCP_SOCKETINFO_PTR;
}