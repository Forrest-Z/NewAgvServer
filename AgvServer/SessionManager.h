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

	//ͨ�����ӣ������û���Ϣ
	typedef boost::unordered_map<TcpConnection::Pointer, Session> MapConnSession;
	typedef boost::shared_ptr<MapConnSession> MapConnSessionPointer;
	
	//ͨ���û�ID����������
	typedef boost::unordered_map<int, TcpConnection::Pointer> MapIdConn;
	typedef boost::shared_ptr<MapIdConn> MapIdConnSession;

	//�û�ID���û�sock ����
	void SaveSession(TcpConnection::Pointer conn, int id,std::string username = "",int role = -1)
	{
		if (m_sessions->find(conn) != m_sessions->end()) {
			//��������Ѿ������ˣ�ɾ���ɵ�ID
			auto itr = m_idSock->find((*m_sessions)[conn].id);
			if (itr != m_idSock->end())
				m_idSock->erase(itr);
		}
		(*m_sessions)[conn].id = id;
		(*m_sessions)[conn].username = username;
		(*m_sessions)[conn].role = role;
		(*m_idSock)[id] = conn;
	}

	void RemoveSession(TcpConnection::Pointer conn)
	{
		auto itr = m_idSock->find((*m_sessions)[conn].id);
		if (itr != m_idSock->end())
			m_idSock->erase(itr);
		m_sessions->erase(conn);
	}

	~SessionManager();

	static Pointer Instance()
	{
		static Pointer m_inst = Pointer(new SessionManager());
		return m_inst;
	}

	MapConnSessionPointer getSession()
	{
		return m_sessions;
	}

	MapIdConnSession getIdSock()
	{
		return m_idSock;
	}

	int getUnloginId()
	{
		return --m_unlogin_id;
	}


private:
	SessionManager();

	MapConnSessionPointer m_sessions;
	MapIdConnSession m_idSock;

	boost::atomic_uint32_t m_unlogin_id;
};

