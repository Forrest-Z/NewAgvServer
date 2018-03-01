#include "SessionManager.h"

SessionManager::SessionManager() :
	m_sessions(new SessionMap()),
	m_idSock(new _umap_idSock())
{
}


SessionManager::~SessionManager()
{
}
