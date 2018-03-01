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
	//ͨ��tcp�����û���Ϣ//ͨ�����ӣ������û����֡�id����ɫ
	typedef boost::unordered_map<TcpConnection::Pointer, Session> SessionMap;
	typedef boost::shared_ptr<SessionMap> SessionPointer;
	
	//ͨ���û�ID����������
	typedef boost::unordered_map<int, TcpConnection::Pointer> _umap_idSock;
	typedef boost::shared_ptr<_umap_idSock> umap_idSock;


	typedef boost::shared_ptr<SessionManager> Pointer;

	//�û�ID���û�sock ����
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

