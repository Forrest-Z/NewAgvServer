#include "SessionManager.h"

SessionManager::SessionManager() :
	m_sessions(new MapConnSession()),
	m_idSock(new MapIdConn()),
	m_unlogin_id(-1)
{
}


SessionManager::~SessionManager()
{
}


//用户ID和用户sock 保存
void SessionManager::SaveSession(TcpConnection::Pointer conn, int id, std::string username, int role)
{
	m_sessionmtx.lock();
	if (m_sessions->find(conn) != m_sessions->end()) {
		//这个连接已经存在了，删除旧的ID
		m_sockmtx.lock();
		auto itr = m_idSock->find((*m_sessions)[conn].id);
		if (itr != m_idSock->end())
			m_idSock->erase(itr);
		m_sockmtx.unlock();
	}
	(*m_sessions)[conn].id = id;
	(*m_sessions)[conn].username = username;
	(*m_sessions)[conn].role = role;
	m_sessionmtx.unlock();
	m_sockmtx.lock();
	(*m_idSock)[id] = conn;
	m_sockmtx.unlock();
}

void SessionManager::RemoveSession(TcpConnection::Pointer conn)
{
	m_sessionmtx.lock();
	auto itr = m_idSock->find((*m_sessions)[conn].id);
	if (itr != m_idSock->end())
		m_idSock->erase(itr);
	m_sessionmtx.unlock();
	m_sockmtx.lock();
	m_sessions->erase(conn);
	m_sockmtx.unlock();
}

SessionManager::MapConnSessionPointer SessionManager::getSession()
{
	std::unique_lock<std::mutex> lck(m_sessionmtx);
	return m_sessions;
}

SessionManager::MapIdConnSession SessionManager::getIdSock()
{
	std::unique_lock<std::mutex> lck(m_sockmtx);
	return m_idSock;
}

int SessionManager::getUnloginId()
{
	return --m_unlogin_id;
}

void SessionManager::addSubAgvPosition(TcpConnection::Pointer conn) {
	removeSubAgvPosition(conn);
	std::unique_lock<std::mutex> lck(m_agv_posion_mtx);
	subs_agv_posion->push_back(conn);
}

void SessionManager::addSubAgvStatus(TcpConnection::Pointer conn) {
	removeSubAgvStatus(conn);
	std::unique_lock<std::mutex> lck(m_agv_status_mtx);
	subs_agv_status->push_back(conn);
}

void SessionManager::addSubLog(TcpConnection::Pointer conn) {
	removeSubLog(conn);
	std::unique_lock<std::mutex> lck(m_log_mtx);
	subs_log->push_back(conn);
}

void SessionManager::addSubTask(TcpConnection::Pointer conn) {
	removeSubTask(conn);
	std::unique_lock<std::mutex> lck(m_task_mtx);
	subs_task->push_back(conn);
}

void SessionManager::removeSubAgvPosition(TcpConnection::Pointer conn) {
	std::unique_lock<std::mutex> lck(m_agv_posion_mtx);
	for (auto itr = subs_agv_posion->begin();itr != subs_agv_posion->end();)
	{
		if ((*itr) == conn)
			itr = subs_agv_posion->erase(itr);
		else
			++itr;
	}
}

void SessionManager::removeSubAgvStatus(TcpConnection::Pointer conn) {
	std::unique_lock<std::mutex> lck(m_agv_status_mtx);
	for (auto itr = subs_agv_status->begin();itr != subs_agv_status->end();)
	{
		if ((*itr) == conn)
			itr = subs_agv_status->erase(itr);
		else
			++itr;
	}
}

void SessionManager::removeSubLog(TcpConnection::Pointer conn) {
	std::unique_lock<std::mutex> lck(m_log_mtx);
	for (auto itr = subs_log->begin();itr != subs_log->end();)
	{
		if ((*itr) == conn)
			itr = subs_log->erase(itr);
		else
			++itr;
	}
}

void SessionManager::removeSubTask(TcpConnection::Pointer conn) {
	std::unique_lock<std::mutex> lck(m_task_mtx);
	for (auto itr = subs_task->begin();itr != subs_task->end();)
	{
		if ((*itr) == conn)
			itr = subs_task->erase(itr);
		else
			++itr;
	}
}

void SessionManager::subAgvPositionForeach(SubCallback cb)
{
	std::unique_lock<std::mutex> lck(m_agv_posion_mtx);
	for (auto itr = subs_agv_posion->begin();itr != subs_agv_posion->end();++itr) {
		cb(*itr);
	}
}
void SessionManager::subAgvStatusForeach(SubCallback cb)
{
	std::unique_lock<std::mutex> lck(m_agv_status_mtx);
	for (auto itr = subs_agv_status->begin();itr != subs_agv_status->end();++itr) {
		cb(*itr);
	}
}
void SessionManager::subLogForeach(SubCallback cb)
{
	std::unique_lock<std::mutex> lck(m_log_mtx);
	for (auto itr = subs_log->begin();itr != subs_log->end();++itr) {
		cb(*itr);
	}
}
void SessionManager::subTaskForeach(SubCallback cb)
{
	std::unique_lock<std::mutex> lck(m_task_mtx);
	for (auto itr = subs_task->begin();itr != subs_task->end();++itr) {
		cb(*itr);
	}
}