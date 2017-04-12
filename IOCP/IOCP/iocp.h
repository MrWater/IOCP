#pragma once

//=====================================
//iocp.h ͷ�ļ�
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
	/*iocp��*/
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

		/*�󶨼����˿�*/
		void __Bind();
		/*����*/
		void __Listen();

		/*�����ص�IO���׽���*/
		void __ConstructSocket(SOCKET&);
		/*������ɶ˿�*/
		void __CreateComletionPort();
		/*����ɶ˿�*/
		void __BindCompletionPort(IOCP_SOCKETINFO_PTR);

		void __OnAccept(IOCP_CONTEXT&);
		void __OnRecv(IOCP_CONTEXT&);
		void __OnSend(IOCP_CONTEXT&);

		bool __PostAccept(IOCP_CONTEXT_PTR);
		bool __PostRecv(IOCP_CONTEXT_PTR);
		bool __PostSend(IOCP_CONTEXT_PTR);

		/*����*/
		static void __WorkerThreadFunc(CIocp*);

	private:
		//�����
		IOCP_SOCKETINFO_PTR m_pSockinfoServer;
		//�߳�����
		THREAD_NUM m_unThreadNum;
		//��ɶ˿�
		NEED_CLOSE COMPLETIONPORT m_hCompletionPort;
		//AcceptEx����ָ��
		LPFN_ACCEPTEX m_lpfnAcceptEx;
		//GetAcceptExSockaddrs����ָ��
		LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockaddrs;
		//�Ƿ�ر�
		bool m_bShutdown;
		//�������߳�
		std::thread* m_tWorkerThreads;
		//��������пͻ���
		std::allocator<IOCP_SOCKETINFO_PTR> m_clients;
		IOCP_SOCKETINFO_PTR* m_begin;
		//����ͻ�������
		int m_nClient;
		HANDLE m_hMutex;
	};

	#ifdef NDEBUG
	#include "iocp.inl"
	#endif // NDEBUG
}


