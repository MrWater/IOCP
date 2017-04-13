#pragma once

//=====================================
//iocp.h 头文件
//
//说明：暂时只为了走通整个流程，很多不好的地方或未处理的地方
//=====================================


#include <string>
#include <memory>
#include <ostream>
#include <thread>
#include <vector>
#include <mutex>
#include <list>

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

		/*开启服务*/
		void Start(const ADDRESS_FAMILY&, const ADDRESS&, const PORT&);
		/*停止服务*/
		void Stop();

	private:
		/*初始化*/
		void __Initialize();
		/*释放内存*/
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

		/*Accept事件处理*/
		void __OnAccept(IOCP_IOCONTEXT_PTR) ;
		/*Recv事件处理*/
		void __OnRecv(IOCP_IOCONTEXT_PTR) const;
		/*Send事件处理*/
		void __OnSend(IOCP_IOCONTEXT_PTR) const;

		/*投递Accept*/
		bool __PostAccept();
		/*投递Recv*/
		bool __PostRecv(IOCP_SOCKETINFO_PTR);
		/*投递Send*/
		bool __PostSend(IOCP_SOCKETINFO_PTR);

		/*工作者线程调用*/
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
		std::list<IOCP_SOCKETINFO_PTR> m_listClients;
		//
		std::mutex m_mutex;
		//记录在堆上申请的但是未释放的内存
		std::list<IOCP_IOCONTEXT_PTR> m_listIoContextNeedtoFree;

		bool m_bStoped;
		bool m_bStarted;
	};

	#ifdef NDEBUG
	#include "iocp.inl"
	#endif // NDEBUG
}


