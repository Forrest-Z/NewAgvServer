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
	//载入数据库

	//得到一个agv的 列表{id,name,ip,port}
	int id = 1;
	std::string name = "agv01";
	std::string ip = "192.168.1.1";
	int port = 9999;

	Agv::Pointer p(new Agv);
	p->init(id, ip, port);
	SaveAgv(id, p);
}

std::list<Client_Response_Msg> AgvManager::getPositions()
{
	std::list<Client_Response_Msg> msgs;
	std::unique_lock<std::mutex> lck(mtx);
	Client_Response_Msg msg;
	memset(&msg, 0, sizeof(Client_Response_Msg));
	msg.head.head = 0x88;
	msg.head.queuenumber = 0;
	msg.head.tail = 0xAA;
	msg.head.todo = CLIENT_MSG_TODO_PUB_AGV_POSITION;
	msg.head.body_length = 0;
	for (auto itr = m_mapIdAgvs->begin();itr != m_mapIdAgvs->end();++itr) {
		if (msg.head.body_length + sizeof(AGV_POSITION_INFO) >= CLIENT_MSG_REQUEST_BODY_MAX_SIZE) {
			msgs.push_back(msg);
			msg.head.body_length = 0;
			memset(msg.body, 0, sizeof(msg.body));
		}
		memcpy(msg.body + msg.head.body_length, &(itr->second->positioninfo), sizeof(AGV_POSITION_INFO));
		msg.head.body_length += sizeof(AGV_POSITION_INFO);
	}
	msgs.push_back(msg);

	return msgs;
}

std::list<Client_Response_Msg> AgvManager::getstatuss()
{
	std::list<Client_Response_Msg> msgs;

	return msgs;
}