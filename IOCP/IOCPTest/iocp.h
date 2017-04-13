#pragma once

//=====================================
//iocp.h ͷ�ļ�
//
//˵������ʱֻΪ����ͨ�������̣��ܶ಻�õĵط���δ����ĵط�
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

		/*��������*/
		void Start(const ADDRESS_FAMILY&, const ADDRESS&, const PORT&);
		/*ֹͣ����*/
		void Stop();

	private:
		/*��ʼ��*/
		void __Initialize();
		/*�ͷ��ڴ�*/
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

		/*Accept�¼�����*/
		void __OnAccept(IOCP_IOCONTEXT_PTR) ;
		/*Recv�¼�����*/
		void __OnRecv(IOCP_IOCONTEXT_PTR) const;
		/*Send�¼�����*/
		void __OnSend(IOCP_IOCONTEXT_PTR) const;

		/*Ͷ��Accept*/
		bool __PostAccept();
		/*Ͷ��Recv*/
		bool __PostRecv(IOCP_SOCKETINFO_PTR);
		/*Ͷ��Send*/
		bool __PostSend(IOCP_SOCKETINFO_PTR);

		/*�������̵߳���*/
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
		std::list<IOCP_SOCKETINFO_PTR> m_listClients;
		//
		std::mutex m_mutex;
		//��¼�ڶ�������ĵ���δ�ͷŵ��ڴ�
		std::list<IOCP_IOCONTEXT_PTR> m_listIoContextNeedtoFree;

		bool m_bStoped;
		bool m_bStarted;
	};

	#ifdef NDEBUG
	#include "iocp.inl"
	#endif // NDEBUG
}


