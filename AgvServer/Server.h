#pragma once

#include <boost/bind.hpp>
#include <boost/pool/pool.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>

#include "TcpServer.h"
#include "Common.h"

//����ģʽ
class Server
{
public:
	virtual ~Server();

	static Server* getInstance();

	//1.��ʼ�����ݿ����ӣ����ǻ���
	bool initSql();

	//2.�������ݿ����ݣ���ʼ�����е�member
	bool initAll();

	//3.��������
	void initListen();

	//4.��ʼ����־
	bool initLog();

	//4.��ʼ����ͼ
	bool initMap();

	//4.��ʼ����ͼ
	bool initAgv();

	void pushPackage(Client_Request_Msg package);

	void PopPackage();

	void publisher_agv_position();
	void publisher_agv_status();
	void publisher_task();
	void publisher_log(AGV_LOG log);

	//֪ͨ�������ڹ㲥
	typedef enum {
		ENUM_NOTIFY_ALL_TYPE_MAP_UPDATE = 0,
	}ENUM_NOTIFY_ALL_TYPE;
	void notifyAll(ENUM_NOTIFY_ALL_TYPE type);

	template <typename T>
	void RunTask(T task)
	{
		m_task_pool.submit(task);
	}

	template <typename T>
	void RunPackTask(T task)
	{
		m_pack_pool.submit(task);
	}
private:
	Server();

	static  Server *m_instance;

	//�����̳߳�
	boost::basic_thread_pool m_task_pool;

	//��Ϣ(pack)�����̳߳�
	boost::basic_thread_pool m_pack_pool;

	//��Ϣ����
	boost::lockfree::queue<Client_Request_Msg> *m_package;

	int getQueueLength() const
	{
		return _push_times - _pop_times;
	}

	boost::atomic_uint32_t               _push_times;
	boost::atomic_uint32_t               _pop_times;
};
