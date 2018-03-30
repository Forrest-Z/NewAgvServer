#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "TcpConnection.h"
#include "Session.h"

class SessionManager :private boost::noncopyable, public boost::enable_shared_from_this<SessionManager>
{
public:
	typedef boost::shared_ptr<SessionManager> Pointer;

	//通过连接，查找用户信息
	typedef boost::unordered_map<TcpConnection::Pointer, Session> MapConnSession;
	typedef boost::shared_ptr<MapConnSession> MapConnSessionPointer;
	
	//通过用户ID，查找连接
	typedef boost::unordered_map<int, TcpConnection::Pointer> MapIdConn;
	typedef boost::shared_ptr<MapIdConn> MapIdConnSession;

	//用户订阅信息
	typedef std::vector<TcpConnection::Pointer> SubSession;
	typedef boost::shared_ptr<SubSession> VectorPointerSessionPtr;

	typedef std::function< void(TcpConnection::Pointer) >  SubCallback;

	void logout(int id);

	//用户ID和用户sock 保存
	void SaveSession(TcpConnection::Pointer conn, int id, std::string username = "", int role = -1);

	void RemoveSession(TcpConnection::Pointer conn);

	~SessionManager();

	static SessionManager::Pointer getInstance()
	{
		static SessionManager::Pointer m_inst = SessionManager::Pointer(new SessionManager());
		return m_inst;
	}

	TcpConnection::Pointer getConnById(int id);

	Session getSessionByConn(TcpConnection::Pointer conn);

	int getUnloginId();

	void addSubAgvPosition(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void addSubAgvStatus(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void addSubLog(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void addSubTask(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void removeSubAgvPosition(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void removeSubAgvStatus(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void removeSubLog(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void removeSubTask(TcpConnection::Pointer conn, Client_Request_Msg msg);

	//执行遍历回调
	void subAgvPositionForeach(SubCallback cb);
	void subAgvStatusForeach(SubCallback cb);
	void subLogForeach(SubCallback cb);
	void subTaskForeach(SubCallback cb);

	void notifyForeach(SubCallback cb);

private:
	SessionManager();

	std::mutex m_sessionmtx;
	MapConnSessionPointer m_sessions; //CONNECTION --> SESSION(username,id,role) 的map

	std::mutex m_sockmtx;
	MapIdConnSession m_idSock;//ID --> CONNECTION 的map

	boost::atomic_uint32_t m_unlogin_id;

	std::mutex m_agv_posion_mtx;
	VectorPointerSessionPtr subs_agv_posion;//订阅者

	std::mutex m_agv_status_mtx;
	VectorPointerSessionPtr subs_agv_status;//订阅者

	std::mutex m_log_mtx;
	VectorPointerSessionPtr subs_log;//订阅者

	std::mutex m_task_mtx;
	VectorPointerSessionPtr subs_task;//订阅者
};

