#include "AgvManager.h"



AgvManager::AgvManager():
	m_mapIdAgvs(new MapIdAgv)
{
}


AgvManager::~AgvManager()
{
}

void AgvManager::init()
{
	//�������ݿ�

	//�õ�һ��agv�� �б�{id,name,ip,port}
	int id = 1;
	std::string name = "agv01";
	std::string ip = "192.168.1.1";
	int port = 9999;

	Agv::Pointer p(new Agv);
	p->init(id, ip, port);
	SaveAgv(id, p);
}