#pragma once

//=====================================
//iocp.h 头文件
//=====================================


#include <string>
#include <memory>
#include <ostream>
#include <thread>
#include <vector>

#include "iocpmodel.h"
#include "iocpcommon.h"

#pragma comment(lib,"ws2_32.lib")

namespace iocp
{
	/*iocp类*/
	class DLL_EXPORT CIocp
	{
	public:
		typedef DWORD THREAD_NUM;
		typedef HANDLE COMPLETIONPORT;
		typedef unsigned long ADDRESS;
		typedef unsigned short PORT;

		CIocp();
		~CIocp();

		void Start(const ADDRESS_FAMILY&, const ADDRESS&, const PORT&);
		void Stop();

	private:
		void __Initialize();
		void __Uninitialize();

		/*绑定监听端口*/
		void __Bind();
		/*监听*/
		void __Listen();

		/*构造重叠IO的套接字*/
		void __ConstructSocket(SOCKET&);
		/*创建完成端口*/
		void __CreateComletionPort();
		/*绑定完成端口*/
		void __BindCompletionPort(IOCP_SOCKETINFO_PTR);

		void __OnAccept(IOCP_CONTEXT&);
		void __OnRecv(IOCP_CONTEXT&);
		void __OnSend(IOCP_CONTEXT&);

		bool __PostAccept(IOCP_CONTEXT_PTR);
		bool __PostRecv(IOCP_CONTEXT_PTR);
		bool __PostSend(IOCP_CONTEXT_PTR);

		/*工作*/
		static void __WorkerThreadFunc(CIocp*);

	private:
		//服务端
		IOCP_SOCKETINFO_PTR m_pSockinfoServer;
		//线程数量
		THREAD_NUM m_unThreadNum;
		//完成端口
		NEED_CLOSE COMPLETIONPORT m_hCompletionPort;
		//AcceptEx函数指针
		LPFN_ACCEPTEX m_lpfnAcceptEx;
		//GetAcceptExSockaddrs函数指针
		LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockaddrs;
		//是否关闭
		bool m_bShutdown;
		//工作者线程
		std::thread* m_tWorkerThreads;
		//连入的所有客户端
		std::allocator<IOCP_SOCKETINFO_PTR> m_clients;
		IOCP_SOCKETINFO_PTR* m_begin;
		//接入客户端数量
		int m_nClient;
		HANDLE m_hMutex;
	};

	#ifdef NDEBUG
	#include "iocp.inl"
	#endif // NDEBUG
}


