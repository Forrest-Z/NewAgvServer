#pragma once

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include "TcpConnection.h"
#include "Session.h"

class SessionManager :private boost::noncopyable, public boost::enable_shared_from_this<SessionManager>
{
public:
	//通过tcp查找用户信息//通过连接，查找用户名字、id、角色
	typedef boost::unordered_map<TcpConnection::Pointer, Session> SessionMap;
	typedef boost::shared_ptr<SessionMap> SessionPointer;
	
	//通过用户ID，查找连接
	typedef boost::unordered_map<int, TcpConnection::Pointer> _umap_idSock;
	typedef boost::shared_ptr<_umap_idSock> umap_idSock;


	typedef boost::shared_ptr<SessionManager> Pointer;

	//用户ID和用户sock 保存
	void SaveSession(TcpConnection::Pointer conn, int id,std::string username = "",int role = -1)
	{
		(*m_sessions)[conn].id = id;
		(*m_sessions)[conn].username = username;
		(*m_sessions)[conn].role = role;
		(*m_idSock)[id] = conn;
	}

	void RemoveSession(TcpConnection::Pointer conn)
	{
		m_sessions->erase(conn);
	}

	~SessionManager();

	static Pointer Instance()
	{
		static Pointer m_inst = Pointer(new SessionManager());
		return m_inst;
	}

	SessionPointer getSession()
	{
		return m_sessions;
	}

	umap_idSock getIdSock()
	{
		return m_idSock;
	}

private:
	SessionManager();

	SessionPointer m_sessions;
	umap_idSock m_idSock;

};

