#pragma once

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


	/*Socket信息*/
	typedef struct tagIocpSocketInfo
	{
		NEED_CLOSE SOCKET m_socket;
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

	/*iocp中io的结构*/
	typedef struct tagIocpIoContext
	{
		OVERLAPPED m_overlapped;
		WSABUF m_wsabuf;
		char m_szBuffer[MAX_BUFFER_LEN];
		IOCP_OPERATION_TYPE m_operationType;
		IOCP_SOCKETINFO_PTR m_pSocketInfo;

		tagIocpIoContext()
		{
			ZeroMemory(&m_overlapped, sizeof(m_overlapped));
			ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
			
			m_wsabuf.buf = m_szBuffer;
			m_wsabuf.len = MAX_BUFFER_LEN;

			m_pSocketInfo = nullptr;
			m_operationType = IOCP_OPERATION_TYPE::_NONE;
		}

		//tagIocpContext(const tagIocpContext& context)
		//{
		//	if (&context == this)
		//		return;

		//	//m_overlapped = context.m_overlapped;
		//	m_wsabuf = context.m_wsabuf;
		//	std::copy(std::begin(context.m_szBuffer), std::end(context.m_szBuffer), m_szBuffer);
		//	m_pSocketInfo = context.m_pSocketInfo;
		//	m_operationType = context.m_operationType;
		//}

		//tagIocpContext(tagIocpContext&& context) noexcept
		//{
		//	if (&context == this)
		//		return;

		//	//m_overlapped = context.m_overlapped;
		//	m_wsabuf = context.m_wsabuf;
		//	std::copy(std::begin(context.m_szBuffer), std::end(context.m_szBuffer), m_szBuffer);
		//	m_pSocketInfo = context.m_pSocketInfo;
		//	m_operationType = context.m_operationType;
		//}

		~tagIocpIoContext()
		{
		}
	} IOCP_IOCONTEXT, *IOCP_IOCONTEXT_PTR;
}