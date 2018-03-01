#include "Server.h"
#include "SessionManager.h"
#include "MsgProcessor.h"
#define  PackageCount          10*1024     //队列中可以存放未处理包的数量

Server *Server::m_instance = NULL;

Server::Server() :
	m_task_pool(std::thread::hardware_concurrency() * 16),
	m_pack_pool(5),
	m_package(new boost::lockfree::queue<Client_Msg>(PackageCount))
{
}

Server::~Server()
{
}

Server* Server::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new Server();
	}
	return m_instance;
}

//1.初始化数据库连接，这是基础
bool Server::initSql()
{
	return true;
}

//2.根据数据库内容，初始化所有的member
bool Server::initAll()
{
	Server  *srv = Server::GetInstance();
	//4个线程并发处理消息
	srv->RunTask(boost::bind(&Server::PopPackage, srv));
	srv->RunTask(boost::bind(&Server::PopPackage, srv));
	srv->RunTask(boost::bind(&Server::PopPackage, srv));
	srv->RunTask(boost::bind(&Server::PopPackage, srv));


	srv->RunTask(boost::bind(&Server::publisher_agv_position, srv));
	srv->RunTask(boost::bind(&Server::publisher_agv_status, srv));
	srv->RunTask(boost::bind(&Server::publisher_task, srv));
	srv->RunTask(boost::bind(&Server::publisher_log, srv));

	return true;
}

//3.开启监听
void Server::initListen()
{
	boost::asio::io_context io_context;

	TcpServer netsrv(io_context, 9999);
	netsrv.start();
	io_context.run();
}

void Server::pushPackage(Client_Msg package)
{
	_push_times += 1;
	uint16_t t = 0;
	//        Logger::GetLogger()->Debug("Queue length:%d",getQueueLength());
	while (getQueueLength() > 50)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
		t++;
		if (t > 1000)
		{
			break;
		}
	}
	m_package->push(package);
}

void Server::PopPackage()
{
	Client_Msg pack;
	SessionManager::umap_idSock t_roleSock = SessionManager::Instance()->getIdSock();
	while (1)
	{
		if (m_package->pop(pack))
		{
			_pop_times += 1;
			TcpConnection::Pointer conn = (*t_roleSock)[pack.user_id];
			if (conn == NULL)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(1));
				continue;
			}
			else
			{
				//CommandParse(conn, pack.data);
				RunPackTask(boost::bind(&MsgProcess, conn, pack));
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}

void Server::publisher_agv_position()
{

}

void Server::publisher_agv_status()
{

}

void Server::publisher_task()
{

}

void Server::publisher_log()
{

}

