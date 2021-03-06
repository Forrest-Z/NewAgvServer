#include <chrono>
#include "Server.h"
#include "SessionManager.h"
#include "MsgProcessor.h"
#include "AgvManager.h"
#include "DBManager.h"
#include "AgvLogManager.h"
#include "MapManger.h"
#include "AgvManager.h"
#define  PackageCount          10*1024     //队列中可以存放未处理包的数量

Server *Server::m_instance = NULL;

Server::Server() :
	m_task_pool(std::thread::hardware_concurrency() * 16),
	m_pack_pool(5),
	m_package(new boost::lockfree::queue<Client_Request_Msg>(PackageCount))
{
}

Server::~Server()
{
}

Server* Server::getInstance()
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
	//初始化数据库连接
	return DBManager::getInstance()->createConnection();
}

bool Server::initLog()
{
	AgvLogManager::PUB_LOG_FUNC pub_func = std::bind(&Server::publisher_log, this, std::placeholders::_1);
	AgvLogManager::getInstance()->init(pub_func);
	return true;
}
bool Server::initMap()
{
	return MapManger::getInstance()->load();
}

bool Server::initAgv()
{
	AgvManager::getInstance()->init();
	return true;
}

//2.根据数据库内容，初始化所有的member
bool Server::initAll()
{
	//初始化数据库连接
	if (!initSql())return false;

	//初始化日志
	if (!initLog())return false;

	//载入地图
	if (!initMap())return false;

	//载入agv
	if (!initAgv())return false;

	//初始化任务处理线程

	//初始化消息处理
	Server  *srv = Server::getInstance();
	srv->RunTask(boost::bind(&Server::PopPackage, srv));
	srv->RunTask(boost::bind(&Server::PopPackage, srv));
	srv->RunTask(boost::bind(&Server::PopPackage, srv));
	srv->RunTask(boost::bind(&Server::PopPackage, srv));

	//初始化广播发布
	srv->RunTask(boost::bind(&Server::publisher_agv_position, srv));//实时发布agv 位置信息
	srv->RunTask(boost::bind(&Server::publisher_agv_status, srv));//实时发布agv 状态信息
	srv->RunTask(boost::bind(&Server::publisher_task, srv));//实时发布当前所有任务信息
															//实时日志发布由 日志处理线程 完成

	//开始监听
	initListen();

	return true;
}

//3.开启监听
void Server::initListen()
{
	boost::asio::io_context io_context;

	TcpServer netsrv(io_context, 5555);
	netsrv.start();
	io_context.run();
}


void Server::pushPackage(Client_Request_Msg package)
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
	Client_Request_Msg pack;
	while (1)
	{
		if (m_package->pop(pack))
		{
			_pop_times += 1;
			TcpConnection::Pointer conn = SessionManager::getInstance()->getConnById(pack.id);
			if (conn == NULL)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			else
			{
				RunPackTask(boost::bind(&MsgProcess, conn, pack));
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void Server::publisher_agv_position()
{
	const int position_pub_interval = 100;//100ms
	std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
		if (interval.count() >= position_pub_interval) {
			beginTime = endTime;

			//组装信息
			std::list<Client_Response_Msg> msgs = AgvManager::getInstance()->getPositions();
			if (msgs.size() <= 0)continue;
			//执行发送
			SessionManager::getInstance()->subAgvPositionForeach([&](TcpConnection::Pointer conn) {
				for (auto itr = msgs.begin(); itr != msgs.end();++itr)
					conn->write_all(*itr);
			});
		}
	}
}

void Server::publisher_agv_status()
{
	const int status_pub_interval = 200;//100ms
	std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
		if (interval.count() >= status_pub_interval) {
			beginTime = endTime;
			//组装信息
			std::list<Client_Response_Msg> msgs = AgvManager::getInstance()->getstatuss();
			if (msgs.size() <= 0)continue;
			//执行发送
			SessionManager::getInstance()->subAgvStatusForeach([&](TcpConnection::Pointer conn) {
				for (auto itr = msgs.begin(); itr != msgs.end();++itr)
					conn->write_all(*itr);
			});
		}
	}
}


void Server::publisher_task()
{
	const int task_pub_interval = 500;//100ms
	std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
		if (interval.count() >= task_pub_interval) {
			beginTime = endTime;

			//组装信息
			Client_Response_Msg msg;
			//TODO

			//执行发送
			SessionManager::getInstance()->subTaskForeach([&](TcpConnection::Pointer conn) {conn->write_all(msg);});
		}
	}
}

void Server::publisher_log(AGV_LOG log)
{
	Client_Response_Msg msg;
	memset(&msg, 0, sizeof(Client_Response_Msg));
	msg.head.head = 0x88;
	msg.head.queuenumber = 0;
	msg.head.tail = 0xAA;
	msg.head.todo = CLIENT_MSG_TODO_PUB_LOG;
	msg.head.body_length = 0;
	//赋值
	memcpy(msg.body, &log, sizeof(AGV_LOG));
	msg.head.body_length = sizeof(AGV_LOG);
	SessionManager::getInstance()->subLogForeach([&](TcpConnection::Pointer conn) {conn->write_all(msg);});
}

void Server::notifyAll(ENUM_NOTIFY_ALL_TYPE type)
{
	if (type == ENUM_NOTIFY_ALL_TYPE_MAP_UPDATE) {
		Client_Response_Msg msg;
		memset(&msg, 0, sizeof(Client_Response_Msg));
		msg.head.head = 0x88;
		msg.head.queuenumber = 0;
		msg.head.tail = 0xAA;
		msg.head.todo = CLIENT_MSG_TODO_NOTIFY_ALL_MAP_UPDATE;
		msg.head.body_length = 0;
		SessionManager::getInstance()->notifyForeach([&](TcpConnection::Pointer conn) {conn->write_all(msg);});
	}
}