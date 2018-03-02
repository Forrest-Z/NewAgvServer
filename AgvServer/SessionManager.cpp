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
