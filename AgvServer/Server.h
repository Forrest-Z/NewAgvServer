#pragma once

#include <boost/bind.hpp>
#include <boost/pool/pool.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include "TcpServer.h"

//����ģʽ
class Server
{
public:
	virtual ~Server();

	static Server* GetInstance();

	//1.��ʼ�����ݿ����ӣ����ǻ���
	bool initSql();

	//2.�������ݿ����ݣ���ʼ�����е�member
	bool initAll();

	//3.��������
	void initListen();


	void pushPackage(Client_Msg package);

	void PopPackage();

	void publisher_agv_position();
	void publisher_agv_status();
	void publisher_task();
	void publisher_log();

	template <typename T>
	void RunTask(T task)
	{
		m_task_pool.schedule(task);
	}

	template <typename T>
	void RunPackTask(T task)
	{
		m_pack_pool.schedule(task);
	}
private:
	Server();

	static  Server *m_instance;

	//�����̳߳�
	boost::basic_thread_pool m_task_pool;

	//��Ϣ(pack)�����̳߳�
	boost::basic_thread_pool m_pack_pool;

	//��Ϣ����
	boost::lockfree::queue<Client_Msg> *m_package;

	int         getQueueLength() const
	{
		return _push_times - _pop_times;
	}

	boost::atomic_uint32_t               _push_times;
	boost::atomic_uint32_t               _pop_times;
};