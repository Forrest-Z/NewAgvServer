#include "AgvManager.h"
#include "DBManager.h"
#include "MapManger.h"

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
	QString querySql = "select id,agv_name,agv_ip,agv_port from agv_agv";
	QList<QVariant> params;
	QList<QList<QVariant> > result = DBManager::getInstance()->query(querySql, params);

	for (int i = 0;i < result.length();++i) {
		QList<QVariant> qsl = result.at(i);
		if (qsl.length() == 4) {
			int id = (qsl.at(0).toInt());
			std::string name = (qsl.at(1).toString().toStdString());
			std::string ip = qsl.at(2).toString().toStdString();
			int port = 9999;

			Agv::TaskFinishCallback _finish = std::bind(&AgvManager::onFinish, this, std::placeholders::_1);
			Agv::TaskErrorCallback _error = std::bind(&AgvManager::onError, this, std::placeholders::_1, std::placeholders::_2);
			Agv::TaskInteruptCallback _interupt = std::bind(&AgvManager::onInterupt, this, std::placeholders::_1);
			Agv::UpdateMCallback _updateM = std::bind(&AgvManager::updateOdometer, this, std::placeholders::_1, std::placeholders::_2);
			Agv::UpdateMRCallback _updateMR = std::bind(&AgvManager::updateStationOdometer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

			Agv::Pointer p(new Agv);
			p->init(id, ip, port);
			SaveAgv(id, p);
		}
	}
}

void AgvManager::onFinish(Agv::Pointer agv)
{
	//TODO,�����һ������
}

void AgvManager::onError(int code, Agv::Pointer agv)
{
	//�����˴���
}

void AgvManager::onInterupt(Agv::Pointer agv)
{
	//�������һ������
}


//1.ֻ����̼�
void AgvManager::updateOdometer(int odometer, Agv::Pointer agv)
{
	if (odometer == agv->lastStationOdometer)//��ԭ����λ��û��
		return;

	//�����ƶ��У�����ԭ����վ���λ����
	if (agv->nowStation > 0 && agv->nextStation > 0)
	{
		//���֮ǰ��һ��վ�㣬�����൱���뿪���Ǹ�վ��
		agv->lastStation = agv->nowStation;
		agv->nowStation = 0;
	}

	if (agv->lastStation <= 0) return;//��һվδ֪����ôδֱ֪�Ӿ���δ֪��
	if (agv->nextStation <= 0) return;//��һվδ֪����ô�Ҳ�֪������

									  //���������֪���ˣ���ô�ҾͿ��Լ��㵱ǰλ����
	odometer -= agv->lastStationOdometer;

	//�����Ƿ񳬹��˵���һ��վ��ľ���
	if (agv->currentPath.size() <= 0)
		return;


	//������Ҫ�ϲ����е�������ΪҪ����
	//��Ҫ���¼������� agvline + startstaion+ endstation
	AgvLine *line = MapManger::getInstance()->getAgvLine(agv->currentPath.at(0));
	if (line->id <= 0)return;
	AgvStation *startStation = MapManger::getInstance()->getAgvStation(line->startStation);
	if (startStation->id <= 0)return;
	AgvStation *endStation = MapManger::getInstance()->getAgvStation(line->endStation);
	if (endStation->id <= 0)return;

	if (odometer <= line->length)
	{
		//����λ��
		if (line->line) {
			double theta = atan2(endStation->y - startStation->y, endStation->x - startStation->x);
			agv->positioninfo.rotation = (theta * 180 / M_PI);
			agv->positioninfo.x = (startStation->x + 1.0*odometer / line->length*cos(theta));
			agv->positioninfo.y = (startStation->y + 1.0*odometer / line->length*sin(theta));
		}
		else {
			AgvArc *agvarc = dynamic_cast<AgvArc *>(line);
			//���µĻ�ͼ�£����㵱ǰ���꣬�Լ�rotation
			double t = 1.0*odometer / agvarc->length;
			if (t<0) {
				t = 0.0;
			}
			if (t>1) {
				t = 1.0;
			}
			//��������
			double startX = startStation->x;
			double startY = startStation->y;
			double endX = endStation->x;
			double endY = endStation->y;
			agv->positioninfo.x = (startX*(1 - t)*(1 - t)*(1 - t)
				+ 3 * agvarc->p1x*t*(1 - t)*(1 - t)
				+ 3 * agvarc->p2x*t*t*(1 - t)
				+ endX * t*t*t);

			agv->positioninfo.y = (startY*(1 - t)*(1 - t)*(1 - t)
				+ 3 * agvarc->p1y*t*(1 - t)*(1 - t)
				+ 3 * agvarc->p2y*t*t*(1 - t)
				+ endY * t*t*t);

			double X = startX * 3 * (1 - t)*(1 - t) * (-1) +
				3 * agvarc->p1x * ((1 - t) * (1 - t) + t * 2 * (1 - t) * (-1)) +
				3 * agvarc->p2x * (2 * t * (1 - t) + t * t * (-1)) +
				endX * 3 * t *t;

			double Y = startY * 3 * (1 - t)*(1 - t) * (-1) +
				3 * agvarc->p1y * ((1 - t) *(1 - t) + t * 2 * (1 - t) * (-1)) +
				3 * agvarc->p2y * (2 * t * (1 - t) + t * t * (-1)) +
				endY * 3 * t *t;

			agv->positioninfo.rotation = (atan2(Y, X) * 180 / M_PI);
		}
	}
}

//2.��վ����Ϣ����̼���Ϣ
void AgvManager::updateStationOdometer(int rfid, int odometer, Agv::Pointer agv)
{
	AgvStation *sstation = MapManger::getInstance()->getAgvStationByRfid(rfid);
	if (sstation->id <= 0)return;

	//��������ô��վ��
	agv->positioninfo.x = (sstation->x);
	agv->positioninfo.y = (sstation->y);

	//���õ�ǰվ��
	agv->nowStation = sstation->id;
	agv->lastStationOdometer = odometer;

	//��ȡpath�е���һվ
	int nextStationTemp = 0;
	for (int i = 0;i<agv->currentPath.size();++i)
	{
		AgvLine *line = MapManger::getInstance()->getAgvLine(agv->currentPath.at(i));
		if (line->id <= 0)continue;
		if (line->endStation == sstation->id)
		{
			if (i + 1 != agv->currentPath.size())
			{
				AgvLine *lineNext = MapManger::getInstance()->getAgvLine(agv->currentPath.at(i + 1));
				nextStationTemp = lineNext->endStation;
			}
			else
				nextStationTemp = 0;
			break;
		}
	}
	agv->nextStation = nextStationTemp;

	//��վ��Ϣ�ϱ�(����������Ϣ��������·ռ������)
	//emit carArriveStation(agv->baseinfo.id, sstation->id);
}

std::list<Client_Response_Msg> AgvManager::getPositions()
{
	std::list<Client_Response_Msg> msgs;
	UNIQUE_LCK lck(mtx);
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
	UNIQUE_LCK lck(mtx);
	Client_Response_Msg msg;
	memset(&msg, 0, sizeof(Client_Response_Msg));
	msg.head.head = 0x88;
	msg.head.queuenumber = 0;
	msg.head.tail = 0xAA;
	msg.head.todo = CLIENT_MSG_TODO_PUB_AGV_STATUS;
	msg.head.body_length = 0;
	for (auto itr = m_mapIdAgvs->begin();itr != m_mapIdAgvs->end();++itr) {
		if (msg.head.body_length + sizeof(AGV_STATUS_INFO) >= CLIENT_MSG_REQUEST_BODY_MAX_SIZE) {
			msgs.push_back(msg);
			msg.head.body_length = 0;
			memset(msg.body, 0, sizeof(msg.body));
		}
		memcpy(msg.body + msg.head.body_length, &(itr->second->statusinfo), sizeof(AGV_STATUS_INFO));
		msg.head.body_length += sizeof(AGV_STATUS_INFO);
	}
	msgs.push_back(msg);

	return msgs;
}