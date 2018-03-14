#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include "Common.h"
#include "TcpConnection.h"

class UserManager :private boost::noncopyable, public boost::enable_shared_from_this<UserManager>
{
public:
	typedef boost::shared_ptr<UserManager> Pointer;

	//通过连接，查找用户信息
	typedef boost::unordered_map<int, USER_INFO> MapIdUser;
	typedef boost::shared_ptr<MapIdUser> MapIdUserPointer;

	static Pointer Instance()
	{
		static Pointer m_inst = Pointer(new UserManager());
		return m_inst;
	}

	void saveUser(USER_INFO &u)
	{
		(*m_idUsers)[u.id] = u;
	}

	void removeUser(int id)
	{
		m_idUsers->erase(id);
	}

	MapIdUserPointer getUserMap()
	{
		return m_idUsers;
	}

	void login(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void logout(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void changePassword(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void list(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void remove(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void add(TcpConnection::Pointer conn, Client_Request_Msg msg);

	void modify(TcpConnection::Pointer conn, Client_Request_Msg msg);

	virtual ~UserManager();
private:
	UserManager();
	MapIdUserPointer m_idUsers;
};

