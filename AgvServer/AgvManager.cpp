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
	//载入数据库
	QString querySql = "select id,agv_name,agv_ip,agv_port from agv_agv";
	QList<QVariant> params;
	QList<QList<QVariant> > result = DBManager::getInstance()->query(querySql, params);

	for (auto qsl : result) {
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
			p->init(id, name, ip, port);
			SaveAgv(id, p);
		}
	}
}

void AgvManager::onFinish(Agv::Pointer agv)
{
	//TODO,完成了一个事情
}

void AgvManager::onError(int code, Agv::Pointer agv)
{
	//发生了错误
}

void AgvManager::onInterupt(Agv::Pointer agv)
{
	//被打断了一个任务
}


//1.只有里程计
void AgvManager::updateOdometer(int odometer, Agv::Pointer agv)
{
	if (odometer == agv->lastStationOdometer)//在原来的位置没动
		return;

	//正在移动中，不再原来的站点的位置了
	if (agv->nowStation > 0 && agv->nextStation > 0)
	{
		//释放站点的占用
		MapManger::getInstance()->freeStation(agv->nowStation, agv->baseinfo.id);
		//添加对线路的占用
		AgvLine *line = MapManger::getInstance()->getAgvLine(agv->nowStation, agv->nextStation);
		if (line != NULL)
			MapManger::getInstance()->addOccuLine(line->id,agv->baseinfo.id);

		//如果之前在一个站点，现在相当于离开了那个站点
		agv->lastStation = agv->nowStation;
		agv->nowStation = 0;
	}

	if (agv->lastStation <= 0) return;//上一站未知，那么未知直接就是未知的
	if (agv->nextStation <= 0) return;//下一站未知，那么我不知道方向。

									  //如果两个都知道了，那么我就可以计算当前位置了
	odometer -= agv->lastStationOdometer;

	//例程是否超过了到下一个站点的距离
	if (agv->currentPath.size() <= 0)
		return;


	//这里需要合并所有的锁，因为要计算
	//需要如下几个因素 agvline + startstaion+ endstation
	AgvLine *line = MapManger::getInstance()->getAgvLine(agv->currentPath.front());
	if (line == NULL || line->id <= 0)return;
	AgvStation *startStation = MapManger::getInstance()->getAgvStation(line->startStation);
	if (startStation == NULL || startStation->id <= 0)return;
	AgvStation *endStation = MapManger::getInstance()->getAgvStation(line->endStation);
	if (endStation == NULL || endStation->id <= 0)return;

	if (odometer <= line->length)
	{
		//计算位置
		if (line->line) {
			double theta = atan2(endStation->y - startStation->y, endStation->x - startStation->x);
			agv->positioninfo.rotation = (theta * 180 / M_PI);
			agv->positioninfo.x = (startStation->x + 1.0*odometer / line->length*cos(theta));
			agv->positioninfo.y = (startStation->y + 1.0*odometer / line->length*sin(theta));
		}
		else {
			AgvArc *agvarc = dynamic_cast<AgvArc *>(line);
			//在新的绘图下，计算当前坐标，以及rotation
			double t = 1.0*odometer / agvarc->length;
			if (t<0) {
				t = 0.0;
			}
			if (t>1) {
				t = 1.0;
			}
			//计算坐标
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

//2.有站点信息和里程计信息
void AgvManager::updateStationOdometer(int rfid, int odometer, Agv::Pointer agv)
{
	AgvStation *sstation = MapManger::getInstance()->getAgvStationByRfid(rfid);
	if (sstation == NULL || sstation->id <= 0)return;

	//到达了这么个站点
	agv->positioninfo.x = (sstation->x);
	agv->positioninfo.y = (sstation->y);

	//设置当前站点
	if (agv->nowStation == 0) {
		//释放线路的占用，
		AgvLine *line = MapManger::getInstance()->getAgvLine(agv->nowStation, agv->nextStation);
		if (line != NULL)
			MapManger::getInstance()->freeLine(line->id, agv->baseinfo.id);
		//设置对站点的占用
		MapManger::getInstance()->occuStation(agv->nowStation, agv->baseinfo.id);
	}

	agv->nowStation = sstation->id;
	agv->lastStationOdometer = odometer;

	//获取path中的下一站
	int nextStationTemp = 0;
	for (auto itr = agv->currentPath.begin();itr!= agv->currentPath.end();++itr)
	{
		AgvLine *line = MapManger::getInstance()->getAgvLine(*itr);
		if (line == NULL || line->id <= 0)continue;
		if (line->endStation == sstation->id)
		{
			if (++itr == agv->currentPath.end()) {
				AgvLine *lineNext = MapManger::getInstance()->getAgvLine(*itr);
				if (lineNext == NULL || lineNext->id < 0) {
					nextStationTemp = 0;
					break;
				}
				nextStationTemp = lineNext->endStation;
			}
			else {
				nextStationTemp = 0;
			}
			break;			
		}
	}
	agv->nextStation = nextStationTemp;

	//到站消息上报(更新任务信息、更新线路占用问题)
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

//手动控制
void AgvManager::interHandRequest(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//TODO:
}
void AgvManager::interHandRelease(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//TODO:
}
void AgvManager::interHandForward(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//TODO:
}
void AgvManager::interHandBackward(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//TODO:
}
void AgvManager::interHandTurnLeft(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//TODO:
}
void AgvManager::interHandTurnRight(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	//TODO:
}

void AgvManager::interList(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	UNIQUE_LCK lck(mtx);
	for (auto itr = m_mapIdAgvs->begin();itr != m_mapIdAgvs->end();++itr) {
		if (response.head.body_length + sizeof(AGV_BASE_INFO) >= CLIENT_MSG_REQUEST_BODY_MAX_SIZE) {
			conn->write_all(response);
			response.head.body_length = 0;
			memset(response.body, 0, sizeof(response.body));
		}
		memcpy(response.body + response.head.body_length, &(itr->second->baseinfo), sizeof(AGV_BASE_INFO));
		response.head.body_length += sizeof(AGV_BASE_INFO);
	}
	conn->write_all(response);
}

void AgvManager::interAdd(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	//TODO:添加到数据库，获取ID返回
	if (msg.head.body_length != sizeof(AGV_BASE_INFO) ) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		AGV_BASE_INFO baseinfo;
		memcpy(&baseinfo, msg.body, sizeof(AGV_BASE_INFO));
		//存库
		QString insertSql = "insert into agv_agv(name,ip,port) values(?,?,?);SELECT @@Identity";
		QList<QVariant> param;
		param << QString::fromLatin1(baseinfo.name) << QString::fromLatin1(baseinfo.ip) << baseinfo.port;
		QList<QList<QVariant> > result = DBManager::getInstance()->query(insertSql, param);
		if (result.length() > 0 && result[0].length()>0) {
			int id = result[0][0].toInt();
			Agv *agv = new Agv();
			agv->init(id, baseinfo.name,baseinfo.ip, baseinfo.port);
			UNIQUE_LCK lck(mtx);
			(*m_mapIdAgvs).insert(std::make_pair(agv->baseinfo.id, agv));
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		}
		else {
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
		}
	}
	conn->write_all(response);
}

void AgvManager::interDelete(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	//TODO:添加到数据库，获取ID返回
	if (msg.head.body_length != sizeof(int)) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		uint32_t id;
		memcpy(&id, msg.body, sizeof(uint32_t));
		//存库
		QString insertSql = "delete from agv_agv where id=?";
		QList<QVariant> param;
		param << id ;
		DBManager::getInstance()->exeSql(insertSql, param);
		if (DBManager::getInstance()->exeSql(insertSql, param)) {
			UNIQUE_LCK lck(mtx);
			auto itr = (*m_mapIdAgvs).find(id);
			if (itr != (*m_mapIdAgvs).end())
				(*m_mapIdAgvs).erase(itr);
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		}
		else {
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
		}
	}
	conn->write_all(response);
}

void AgvManager::interModify(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	//TODO:添加到数据库，获取ID返回
	if (msg.head.body_length != sizeof(AGV_BASE_INFO)) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
	}
	else {
		AGV_BASE_INFO baseinfo;
		memcpy(&baseinfo, msg.body, sizeof(AGV_BASE_INFO));
		//存库
		QString insertSql = "update agv_agv set name=?,ip=?,port=? where id = ?;";
		QList<QVariant> param;
		param << QString::fromLatin1(baseinfo.name) << QString::fromLatin1(baseinfo.ip) << baseinfo.port << baseinfo.id;
		if (DBManager::getInstance()->exeSql(insertSql, param)) {
			UNIQUE_LCK lck(mtx);
			memcpy( (*m_mapIdAgvs)[baseinfo.id]->baseinfo.name,baseinfo.name,strlen(baseinfo.name));
			memcpy((*m_mapIdAgvs)[baseinfo.id]->baseinfo.ip, baseinfo.ip, strlen(baseinfo.ip));
			(*m_mapIdAgvs)[baseinfo.id]->baseinfo.port = baseinfo.port;
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		}
		else {
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_SAVE_SQL_FAIL;
		}
	}
	conn->write_all(response);
}

void AgvManager::agvForeach(AgvEachCallback cb)
{
	std::unique_lock<std::mutex> lck(mtx);
	for (auto itr = m_mapIdAgvs->begin();itr != m_mapIdAgvs->end();++itr) {
		cb(itr->second);
	}
}


bool AgvManager::agvStartTask(int agvId, std::list<int> path)
{
	std::unique_lock<std::mutex> lck(mtx);
	if (m_mapIdAgvs->find(agvId) == m_mapIdAgvs->end())return false;
	if (path.empty())return false;
	Agv::Pointer agv = (*m_mapIdAgvs)[agvId];

	//TODO:这里需要启动小车，告诉小车下一站和下几站，还有就是左中右信息(回头再说左中右)
	agv->currentPath = (path);
	MapManger::Pointer mapManager = MapManger::getInstance();
	//获取小车在这之前的线路
	int agvLastLine = 0;
	if (agv->nowStation == 0 && agv->lastStation != 0) {
		AgvLine * line = mapManager->getAgvLine(agv->lastStation, agv->nowStation);
		if(line!=NULL) agvLastLine = line->id;
	}
	else if (agv->lastStation != 0 && agv->nextStation != 0) {
		AgvLine * line = mapManager->getAgvLine(agv->lastStation, agv->nextStation);
		if (line != NULL) agvLastLine = line->id;
	}

	//获取path中的下一站
	if (agv->nowStation != mapManager->getAgvLine(agv->currentPath.front())->startStation) {
		agv->nextStation = mapManager->getAgvLine(agv->currentPath.front())->startStation;
	}
	else {
		agv->nextStation = mapManager->getAgvLine(agv->currentPath.front())->endStation;
	}

	//现在开始让小车执行长队列
	std::vector<AgvOrder> orders;
	for (auto itr = path.begin();itr != path.end();) {
		AgvOrder order;
		int thisLine = *itr;
		order.rfid = mapManager->getAgvStation(mapManager->getAgvLine(thisLine)->endStation)->rfid;
		int lmr = mapManager->getLMR(agvLastLine, thisLine);
		agvLastLine = thisLine;
		//根本没有这个左中右的接口！！！！！ 白忙活
		/*if (lmr == PATH_LMR_LEFT) {
			order.order = AgvOrder::ORDER_FORWARD;
			order.param = 5;
		}*/

		order.order = AgvOrder::ORDER_FORWARD;
		order.param = 5;
		orders.push_back(order);
	}
	
	agv->startTask(orders);
}