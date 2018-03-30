#include "MapManger.h"
#include <cmath>
#include <algorithm>
#include "DBManager.h"
#include "Common.h"
#include "Server.h"

MapManger::MapManger():isCreating(false)
{
}


MapManger::~MapManger()
{
}


//2.从数据库中载入地图
bool MapManger::load()
{
	clear();

	UNIQUE_LCK lck(mtx_stations);
	UNIQUE_LCK lck2(mtx_lines);
	UNIQUE_LCK lck3(mtx_lmr);
	UNIQUE_LCK lck4(mtx_adj);
	UNIQUE_LCK lck5(mtx_reverse);

	QString queryStationSql = "select id,station_x,station_y,station_name,station_rfid,station_color_r,station_color_g,station_color_b from agv_station";
	QList<QVariant> params;
	QList<QList<QVariant> > result = DBManager::getInstance()->query(queryStationSql, params);
	for (int i = 0;i < result.length();++i) {
		QList<QVariant> qsl = result.at(i);
		if (qsl.length() != 8) {
			QString ss = "select error!!!!!!" + queryStationSql;
			//g_log->log(AGV_LOG_LEVEL_ERROR, ss);
			return false;
		}
		AgvStation *station = new AgvStation;
		station->id = (qsl.at(0).toInt());
		station->x = (qsl.at(1).toDouble());
		station->y = (qsl.at(2).toDouble());
		station->name = (qsl.at(3).toString().toStdString());
		station->rfid = (qsl.at(4).toInt());
		station->color_r = (qsl.at(5).toInt());
		station->color_g = (qsl.at(6).toInt());
		station->color_b = (qsl.at(7).toInt());
		g_m_stations.insert(std::make_pair(station->id, station));
	}

	//lines
	QString squeryLineSql = "select id,line_startStation,line_endStation,line_line,line_length,line_draw,line_p1x,line_p1y,line_p2x,line_p2y,line_color_r,line_color_g,line_color_b from agv_line";
	result = DBManager::getInstance()->query(squeryLineSql, params);
	for (int i = 0;i < result.length();++i) {
		QList<QVariant> qsl = result.at(i);
		if (qsl.length() != 13) {
			QString ss;
			ss = "select error!!!!!!" + squeryLineSql;
			//g_log->log(AGV_LOG_LEVEL_ERROR, ss);
			return false;
		}
		if ((qsl.at(3).toBool())) {
			//直线
			AgvLine *line = new AgvLine;
			line->id = (qsl.at(0).toInt());
			line->startStation = (qsl.at(1).toInt());
			line->endStation = (qsl.at(2).toInt());
			line->line = (qsl.at(3).toBool());
			line->length = (qsl.at(4).toInt());
			line->draw = (qsl.at(5).toInt());
			/*line->p1x = (qsl.at(6).toDouble());
			line->p1y = (qsl.at(7).toDouble());
			line->p2x = (qsl.at(8).toDouble());
			line->p2y = (qsl.at(9).toDouble());*/
			line->color_r = (qsl.at(10).toInt());
			line->color_g = (qsl.at(11).toInt());
			line->color_b = (qsl.at(12).toInt());

			g_m_lines.insert(std::make_pair(line->id, line));
		}
		else {
			//曲线
			AgvArc *line = new AgvArc;
			line->id = (qsl.at(0).toInt());
			line->startStation = (qsl.at(1).toInt());
			line->endStation = (qsl.at(2).toInt());
			line->line = (qsl.at(3).toBool());
			line->length = (qsl.at(4).toInt());
			line->draw = (qsl.at(5).toInt());
			line->p1x = (qsl.at(6).toDouble());
			line->p1y = (qsl.at(7).toDouble());
			line->p2x = (qsl.at(8).toDouble());
			line->p2y = (qsl.at(9).toDouble());
			line->color_r = (qsl.at(10).toInt());
			line->color_g = (qsl.at(11).toInt());
			line->color_b = (qsl.at(12).toInt());

			g_m_lines.insert(std::make_pair(line->id, line));
		}
	}

	//反方向线路
	for (auto itr = g_m_lines.begin();itr != g_m_lines.end();++itr) {
		for (auto pos = g_m_lines.begin();pos != g_m_lines.end();++pos) {
			if (itr->first != pos->first
				&& itr->second->startStation == pos->second->endStation
				&&itr->second->endStation == pos->second->startStation) {
				g_reverseLines[itr->first] = pos->first;
				g_reverseLines[pos->first] = itr->first;
			}
		}
	}

	//lmr
	QString queryLmrSql = "select lmr_lastLine,lmr_nextLine,lmr_lmr from agv_lmr";
	result = DBManager::getInstance()->query(queryLmrSql, params);
	for (int i = 0;i < result.length();++i) {
		QList<QVariant> qsl = result.at(i);
		if (qsl.length() != 3) {
			QString ss = "select error!!!!!!" + queryLmrSql;
			//g_log->log(AGV_LOG_LEVEL_ERROR, ss);

			return false;
		}
		PATH_LEFT_MIDDLE_RIGHT ll;
		ll.lastLine = qsl.at(0).toInt();
		ll.nextLine = qsl.at(1).toInt();
		g_m_lmr.insert(std::make_pair(ll, qsl.at(2).toInt()));
	}

	//adj
	QString queryAdjSql = "select adj_startLine,adj_endLine from agv_adj";
	result = DBManager::getInstance()->query(queryAdjSql, params);
	for (int i = 0;i < result.length();++i)
	{
		QList<QVariant> qsl = result.at(i);
		if (qsl.length() != 2) {
			QString ss = "select error!!!!!!" + queryAdjSql;
			//g_log->log(AGV_LOG_LEVEL_ERROR, ss);
			return false;
		}
		int endLineId = qsl.at(1).toInt();
		int startLine = qsl.at(0).toInt();
		if (g_m_l_adj.find(startLine) != g_m_l_adj.end()) {
			g_m_l_adj[startLine].push_back(endLineId);
		}
		else {
			std::list<int> lines;
			lines.push_back(endLineId);
			g_m_l_adj[startLine] = lines;
		}
	}

	return true;
}

void MapManger::interCreateStart(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	//TODO:check any task doing?
	bool taskDoing = true;
	if (taskDoing)
	{
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_TASKING;
		sprintf_s(response.return_head.error_info, "there are some task is taking", strlen("there are some task is taking"), sizeof(response.return_head.error_info));
	}
	else {
		createMapStart();
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	}

	conn->write_all(response);
}

void MapManger::interCreateAddStation(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (!isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_NOT_CTREATING;
		sprintf_s(response.return_head.error_info, "is not creating map", strlen("is not creating map"), sizeof(response.return_head.error_info));
	}
	else {
		if (msg.head.body_length % sizeof(STATION_INFO) != 0) {
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
			sprintf_s(response.return_head.error_info, "error STATION_INFO length", strlen("error STATION_INFO length"), sizeof(response.return_head.error_info));
		}
		else {
			int n = msg.head.body_length % sizeof(STATION_INFO);
			for (int i = 0;i < n;++i) {
				STATION_INFO station;
				memcpy(&station, msg.body + i * (sizeof(STATION_INFO)), sizeof(STATION_INFO));
				addStation(station);
			}
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		}
		
	}
	conn->write_all(response);
}

void MapManger::interCreateAddLine(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (!isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_NOT_CTREATING;
		sprintf_s(response.return_head.error_info, "is not creating map", strlen("is not creating map"), sizeof(response.return_head.error_info));
	}
	else {
		if (msg.head.body_length % sizeof(AGV_LINE) != 0) {
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
			sprintf_s(response.return_head.error_info, "error AGV_LINE length", strlen("error AGV_LINE length"), sizeof(response.return_head.error_info));
		}
		else {
			int n = msg.head.body_length % sizeof(AGV_LINE);
			for (int i = 0;i < n;++i) {
				AGV_LINE line;
				memcpy(&line, msg.body + i * (sizeof(AGV_LINE)), sizeof(AGV_LINE));
				addLine(line);
			}
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		}

	}
	conn->write_all(response);
}

void MapManger::interCreateAddArc(TcpConnection::Pointer conn, Client_Request_Msg msg) 
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (!isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_NOT_CTREATING;
		sprintf_s(response.return_head.error_info, "is not creating map", strlen("is not creating map"), sizeof(response.return_head.error_info));
	}
	else {
		if (msg.head.body_length % sizeof(AGV_ARC) != 0) {
			response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_LENGTH;
			sprintf_s(response.return_head.error_info, "error AGV_ARC length", strlen("error AGV_ARC length"), sizeof(response.return_head.error_info));
		}
		else {
			int n = msg.head.body_length % sizeof(AGV_ARC);
			for (int i = 0;i < n;++i) {
				AGV_ARC arc;
				memcpy(&arc, msg.body + i * (sizeof(AGV_ARC)), sizeof(AGV_ARC));
				addArc(arc);
			}
			response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		}

	}
	conn->write_all(response);
}

void MapManger::interCreateFinish(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (!isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_NOT_CTREATING;
		sprintf_s(response.return_head.error_info, "is not creating map", strlen("is not creating map"), sizeof(response.return_head.error_info));
	}
	else {
		createMapFinish();
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
	}
	conn->write_all(response);


	//通知所有客户端，map更新了
	Server::getInstance()->notifyAll(Server::ENUM_NOTIFY_ALL_TYPE_MAP_UPDATE);
}

void MapManger::interListStation(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_CTREATING;
		sprintf_s(response.return_head.error_info, "is creating map", strlen("is creating map"), sizeof(response.return_head.error_info));
	}
	else {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		std::list<STATION_INFO> stations = getStationList();
		for (auto s : stations) {
			if (response.head.body_length + sizeof(STATION_INFO) > CLIENT_MSG_REQUEST_BODY_MAX_SIZE) {
				conn->write_all(response);
				response.head.body_length = 0;
			}
			memcpy(response.body + response.head.body_length, &s, sizeof(STATION_INFO));
			response.head.body_length += sizeof(STATION_INFO);
		}		
	}
	conn->write_all(response);
}

void MapManger::interListLine(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_CTREATING;
		sprintf_s(response.return_head.error_info, "is creating map", strlen("is creating map"), sizeof(response.return_head.error_info));
	}
	else {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		std::list<AGV_LINE> lines = getLineList();
		for (auto l : lines) {
			if (response.head.body_length + sizeof(AGV_LINE) > CLIENT_MSG_REQUEST_BODY_MAX_SIZE) {
				conn->write_all(response);
				response.head.body_length = 0;
			}
			memcpy(response.body + response.head.body_length, &l, sizeof(AGV_LINE));
			response.head.body_length += sizeof(AGV_LINE);
		}
	}
	conn->write_all(response);
}

void MapManger::interListArc(TcpConnection::Pointer conn, Client_Request_Msg msg)
{
	Client_Response_Msg response;
	memset(&response, 0, sizeof(Client_Response_Msg));
	memcpy(&response.head, &msg.head, sizeof(Client_Common_Head));
	response.head.body_length = 0;
	response.return_head.result = CLIENT_RETURN_MSG_RESULT_FAIL;

	if (isCreating) {
		response.return_head.error_code = CLIENT_RETURN_MSG_ERROR_CODE_CTREATING;
		sprintf_s(response.return_head.error_info, "is creating map", strlen("is creating map"), sizeof(response.return_head.error_info));
	}
	else {
		response.return_head.result = CLIENT_RETURN_MSG_RESULT_SUCCESS;
		std::list<AGV_ARC> arcs = getArcList();
		for (auto a : arcs) {
			if (response.head.body_length + sizeof(AGV_ARC) > CLIENT_MSG_REQUEST_BODY_MAX_SIZE) {
				conn->write_all(response);
				response.head.body_length = 0;
			}
			memcpy(response.body + response.head.body_length, &a, sizeof(AGV_ARC));
			response.head.body_length += sizeof(AGV_ARC);
		}
	}
	conn->write_all(response);
}

void MapManger::clear()
{
	UNIQUE_LCK lck(mtx_stations);
	UNIQUE_LCK lck2(mtx_lines);
	UNIQUE_LCK lck3(mtx_lmr);
	UNIQUE_LCK lck4(mtx_adj);
	UNIQUE_LCK lck5(mtx_reverse);

	for (auto p : g_m_stations)
	{
		delete p.second;
	}
	for (auto p : g_m_lines)
	{
		delete p.second;
	}
	g_m_stations.clear();
	g_m_lines.clear();
	g_m_lmr.clear();
	g_m_l_adj.clear();
	g_reverseLines.clear();
}

//重置地图
void MapManger::createMapStart()
{
	//如果有任务正在执行或者等待执行，那么返回错误
	clear();
	//update database
	QString deleteStationSql = "delete from agv_station;";
	QList<QVariant> params;

	bool b = DBManager::getInstance()->exeSql(deleteStationSql, params);
	if (!b) {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "can not clear table agv_station!");
	}
	QString deleteLineSql = "delete from agv_line;";
	b = DBManager::getInstance()->exeSql(deleteLineSql, params);
	if (!b) {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "can not clear table agv_line!");
	}
	QString deleteLmrSql = "delete from agv_lmr;";
	b = DBManager::getInstance()->exeSql(deleteLmrSql, params);
	if (!b) {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "can not clear table agv_lmr!");
	}
	QString deleteAdjSql = "delete from agv_adj;";
	b = DBManager::getInstance()->exeSql(deleteAdjSql, params);
	if (!b) {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "can not clear table agv_adj!");
	}
	isCreating = true;
}

//后添加站点
bool MapManger::addStation(STATION_INFO s)
{
	QString insertSql = "INSERT INTO agv_station (id,station_name, station_x,station_y,station_rfid,station_color_r,station_color_g,station_color_b) VALUES (?,?,?,?,?,?,?,?);";
	QList<QVariant> params;
	params << s.id << QString::fromStdString(s.name) << s.x << s.y
		<< s.rfid << s.r << s.g << s.b;
	if (DBManager::getInstance()->exeSql(insertSql, params)) {
		UNIQUE_LCK lck(mtx_stations);
		g_m_stations.insert(std::make_pair(s.id, new AgvStation(s)));
		return true;
	}
	else {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "save agv statiom to database fail!");
		return false;
	}
}

//后添加线路
bool MapManger::addLine(AGV_LINE l)
{
	QString insertSql = "INSERT INTO agv_line (id,line_startStation,line_endStation,line_color_r,line_color_g,line_color_b,line_line,line_length,line_draw) VALUES (?,?,?,?,?,?,?,?,?,?);";
	QList<QVariant> params;
	params << l.id << l.startStation << l.endStation << l.r << l.g << l.b << true << l.length << l.draw;

	if (DBManager::getInstance()->exeSql(insertSql, params))
	{
		UNIQUE_LCK lck(mtx_lines);
		g_m_lines.insert(std::make_pair(l.id, new AgvLine(l)));
		return true;
	}
	else {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "save agv line to database fail!");
		return false;
	}
}

//重置后添加曲线
bool MapManger::addArc(AGV_ARC arc)
{
	QString insertSql = "INSERT INTO agv_line (id,line_startStation,line_endStation,line_color_r,line_color_g,line_color_b,line_line,line_length,line_draw,line_p1x,line_p1y,line_p2x,line_p2y) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
	QList<QVariant> params;
	params << arc.id
		<< arc.startStation
		<< arc.endStation
		<< arc.r
		<< arc.g
		<< arc.b
		<< false
		<< arc.length
		<< arc.draw
		<< arc.p1x
		<< arc.p1y
		<< arc.p2x
		<< arc.p2y;

	if (DBManager::getInstance()->exeSql(insertSql, params))
	{
		UNIQUE_LCK lck(mtx_lines);
		g_m_lines.insert(std::make_pair(arc.id, new AgvArc(arc)));
		return true;
	}
	else {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "save agv line to database fail!");
		return false;
	}
}

int MapManger::getLMR(AgvLine *lastLine, AgvLine *nextLine)
{
	if (lastLine->endStation != nextLine->startStation)return PATH_LMF_NOWAY;

	double lastAngle, nextAngle;

	double l_startX = g_m_stations[lastLine->startStation]->x;
	double l_startY = g_m_stations[lastLine->startStation]->y;
	double l_endX = g_m_stations[lastLine->endStation]->x;
	double l_endY = g_m_stations[lastLine->endStation]->y;
	double n_startX = g_m_stations[nextLine->startStation]->x;
	double n_startY = g_m_stations[nextLine->startStation]->y;
	double n_endX = g_m_stations[nextLine->endStation]->x;
	double n_endY = g_m_stations[nextLine->endStation]->y;

	if (lastLine->line) {
		lastAngle = atan2(l_endY - l_startY, l_endX - l_startX);
	}
	else {
		lastAngle = atan2(l_endY - (dynamic_cast<AgvArc *>(lastLine))->p2y, l_endX - dynamic_cast<AgvArc *>(lastLine)->p2x);
	}

	if (nextLine->line) {
		nextAngle = atan2(n_endY - n_startY, n_endX - n_startX);
	}
	else {
		nextAngle = atan2(dynamic_cast<AgvArc *>(nextLine)->p1y - n_startY, dynamic_cast<AgvArc *>(nextLine)->p1x - n_startX);
	}

	double changeAngle = nextAngle - lastAngle;

	while (changeAngle > M_PI) {
		changeAngle -= 2 * M_PI;
	}
	while (changeAngle < -1 * M_PI) {
		changeAngle += 2 * M_PI;
	}

	//夹角小于20° 认为直线行驶
	if (abs(changeAngle) <= 20 * M_PI / 180) {
		//角度基本一致
		//如果只有一条线路，那就不算左中右，为了将来的东西防止出问题。如果不只一条线路，并且下一条线路是弧线，则endAngle要重新计算
		if (!nextLine->line)
		{
			//这种情况比较多见，怎么优化呢？？回头再说吧
			nextAngle = atan2(n_endY - n_startY, n_endX - n_startX);
			double changeAngle = nextAngle - lastAngle;

			while (changeAngle > M_PI) {
				changeAngle -= 2 * M_PI;
			}
			while (changeAngle < -1 * M_PI) {
				changeAngle += 2 * M_PI;
			}
			if (abs(changeAngle) <= 20 * M_PI / 180) {
				return PATH_LMR_MIDDLE;
			}
			else if (changeAngle > 0) {
				return PATH_LMR_RIGHT;
			}
			else {
				return PATH_LMR_LEFT;
			}
		}
		return PATH_LMR_MIDDLE;
	}

	//夹角大于80°，认为拐的弧度过大，不能过去
	if (abs(changeAngle) >= 100 * M_PI / 180) {
		//拐角特别大！
		return PATH_LMF_NOWAY;
	}

	if (changeAngle > 0) {
		return PATH_LMR_RIGHT;
	}
	else {
		return PATH_LMR_LEFT;
	}
}


//重置完了地图//通知所有客户端地图更新,
void MapManger::createMapFinish()
{
	UNIQUE_LCK lck(mtx_stations);
	UNIQUE_LCK lck2(mtx_lines);
	UNIQUE_LCK lck3(mtx_lmr);
	UNIQUE_LCK lck4(mtx_adj);
	UNIQUE_LCK lck5(mtx_reverse);
	//A. ----- 通过添加的站点，线路，计算所有的 adj。然后计算所有的lmr。
	//1. 构建反向线 有A--B的线路，然后将B--A的线路推算出来
	int maxId = 0;
	std::for_each(g_m_lines.begin(), g_m_lines.end(), [&](std::pair<int, AgvLine *> pp) {
		if (pp.second->id > maxId) {
			maxId = pp.second->id;
		}
	});
	std::for_each(g_m_lines.begin(), g_m_lines.end(), [&](std::pair<int, AgvLine *> pp) {
		if (pp.second->line) {
			AgvLine *l = pp.second;
			AgvLine *r = new AgvLine(*l);
			r->draw = !l->draw;
			r->id = ++maxId;
			r->startStation = l->endStation;
			r->endStation = l->startStation;
			g_m_lines.insert(std::make_pair(r->id, r));
		}
		else {
			AgvArc *a = dynamic_cast<AgvArc *>(pp.second);
			AgvArc *r = new AgvArc(*a);
			r->draw = !a->draw;
			r->id = ++maxId;
			r->startStation = a->endStation;
			r->endStation = a->startStation;
			r->p1x = a->p2x;
			r->p1y = a->p2y;
			r->p2x = a->p1x;
			r->p2y = a->p1y;
			g_m_lines.insert(std::make_pair(r->id, r));
		}
	});
	//2.构建LMR
	for (auto itr = g_m_lines.begin();itr != g_m_lines.end();++itr) {
		AgvLine *a = itr->second;
		for (auto pos = g_m_lines.begin();pos != g_m_lines.end();++pos) {
			AgvLine *b = pos->second;
			if (a == b || *a == *b)continue;
			//a-->station -->b （a线路的终点是b线路的起点。那么计算一下这三个点的左中右信息）
			if (a->endStation == b->startStation && a->startStation != b->endStation) {
				PATH_LEFT_MIDDLE_RIGHT p;
				p.lastLine = a->id;
				p.nextLine = b->id;
				if (g_m_lmr.find(p) != g_m_lmr.end())continue;
				g_m_lmr[p] = getLMR(a, b);
				//保存到数据库
				QString insertSql = "insert into agv_lmr(lmr_lastLine,lmr_nextLine,lmr_lmr) values(?,?,?);";
				QList<QVariant> params;
				params << p.lastLine << p.nextLine << g_m_lmr[p];
				DBManager::getInstance()->exeSql(insertSql, params);
			}
		}
	}
	//3.构建ADJ
	for (auto itr = g_m_lines.begin();itr != g_m_lines.end();++itr) {
		AgvLine *a = itr->second;
		for (auto pos = g_m_lines.begin();pos != g_m_lines.end();++pos) {
			AgvLine *b = pos->second;
			if (a == b || *a == *b)continue;

			PATH_LEFT_MIDDLE_RIGHT p;
			p.lastLine = a->id;
			p.nextLine = b->id;

			if (a->endStation == b->startStation && a->startStation != b->endStation) {
				if (g_m_l_adj.find(a->id) != g_m_l_adj.end())
				{
					if (std::find(g_m_l_adj[a->id].begin(), g_m_l_adj[a->id].end(), b->id) != g_m_l_adj[a->id].end()) {
						continue;
					}
					if (g_m_lmr.find(p) != g_m_lmr.end() && g_m_lmr[p] != PATH_LMF_NOWAY) {
						g_m_l_adj[a->id].push_back(b->id);
					}
				}
			}
		}
	}

	//4.将adj保存到数据库
	for (auto itr = g_m_l_adj.begin();itr != g_m_l_adj.end();++itr) {
		std::list<int> lines = itr->second;
		QString insertSql = "insert into agv_adj (adj_startLine,adj_endLine) values(?,?)";
		QList<QVariant> params;
		for (auto pos = lines.begin();pos != lines.end();++pos)
		{
			params.clear();
			params << itr->first << *pos;
			DBManager::getInstance()->exeSql(insertSql, params);
		}
	}

	isCreating = false;	
}

//获取最优路径
std::list<int> MapManger::getBestPath(int agvId, int lastStation, int startStation, int endStation, int &distance, bool canChangeDirect)
{
	distance = DISTANCE_INFINITY;
	int disA = DISTANCE_INFINITY;
	int disB = DISTANCE_INFINITY;
	std::list<int> a = getPath(agvId, lastStation, startStation, endStation, disA, false);
	std::list<int> b;
	if (canChangeDirect) {
		b = getPath(agvId, startStation, lastStation, endStation, disB, true);
		if (disA != DISTANCE_INFINITY && disB != DISTANCE_INFINITY) {
			distance = disA < disB ? disA : disB;
			if (disA < disB)return a;
			return b;
		}
	}
	if (disA != DISTANCE_INFINITY) {
		distance = disA;
		return a;
	}
	distance = disB;
	return b;
}

//占领一个站点
void MapManger::occuStation(int station, int occuAgv)
{
	UNIQUE_LCK lck(mtx_stations);
	if (g_m_stations.find(station) != g_m_stations.end()) {
		g_m_stations[station]->occuAgv = occuAgv;
	}
}

//线路的反向占用//这辆车行驶方向和线路方向相反
void MapManger::addOccuLine(int line, int occuAgv)
{
	UNIQUE_LCK lck(mtx_lines);
	if (g_m_lines.find(line) != g_m_lines.end()) {
		g_m_lines[line]->occuAgvs.push_back(occuAgv);
	}
}

//如果车辆占领该站点，释放
void MapManger::freeStation(int station, int occuAgv)
{
	UNIQUE_LCK lck(mtx_stations);
	if (g_m_stations.find(station) != g_m_stations.end()) {
		if (g_m_stations[station]->occuAgv == occuAgv)
			g_m_stations[station]->occuAgv = 0;
	}
}

//如果车辆在线路的占领表中，释放出去
void MapManger::freeLine(int line, int occuAgv)
{
	UNIQUE_LCK lck(mtx_lines);
	if (g_m_lines.find(line) != g_m_lines.end()) {
		g_m_lines[line]->occuAgvs.remove(occuAgv);
	}
}

//释放车辆占用的线路，除了某条线路【因为车辆停在了一条线路上】
void MapManger::freeOtherLine(int occuAgv, int exceptLine)
{
	UNIQUE_LCK lck(mtx_lines);
	for (auto p : g_m_lines) {
		if (p.first == exceptLine)continue;
		p.second->occuAgvs.remove(occuAgv);
	}
}

//释放车辆占用的站点，除了某个站点【因为车辆站在某个站点上】
void MapManger::freeOtherStation(int agvId, int excepetStation)
{
	UNIQUE_LCK lck(mtx_stations);
	for (auto p : g_m_stations) {
		if (p.first == excepetStation)continue;
		if (p.second->occuAgv == agvId) {
			p.second->occuAgv = 0;
		}
	}
}

//通过起止站点，获取线路
int MapManger::getLineId(int startStation, int endStation)
{
	UNIQUE_LCK lck(mtx_lines);
	for (auto p : g_m_lines) {
		if (p.second->startStation == startStation && p.second->endStation == endStation)
			return p.first;
	}
	return 0;
}

//通过ID获取一个站点
AgvStation* MapManger::getAgvStation(int stationId)
{
	if (isCreating)return NULL;//正在创建地图的话，不可以处理这些数据
	UNIQUE_LCK lck(mtx_stations);
	for (auto p : g_m_stations) {
		if (p.first == stationId)return p.second;
	}
	return NULL;
}

AgvStation* MapManger::getAgvStationByRfid(int rfid)
{
	if (isCreating)return NULL;//正在创建地图的话，不可以处理这些数据
	UNIQUE_LCK lck(mtx_stations);
	for (auto p : g_m_stations) {
		if (p.second->rfid == rfid)return p.second;
	}
	return NULL;
}

//获取所有的站点[只用于读取列表，不可用于计算]
std::list<STATION_INFO> MapManger::getStationList()
{
	std::list<STATION_INFO> l;
	UNIQUE_LCK lck(mtx_stations);
	for (auto p : g_m_stations) {
		STATION_INFO s;
		s.x = p.second->x;
		s.y = p.second->y;
		sprintf_s(s.name,p.second->name.c_str(), p.second->name.length(),64);
		s.occuagv = p.second->occuAgv;
		s.id = p.second->id;
		s.rfid = p.second->rfid;
		s.r = p.second->color_r;
		s.g = p.second->color_g;
		s.b = p.second->color_b;
		l.push_back(s);
	}
	return l;
}

//通过ID获取一个线路
AgvLine* MapManger::getAgvLine(int lineId)
{
	if (isCreating)return NULL;//正在创建地图的话，不可以处理这些数据
	UNIQUE_LCK lck(mtx_lines);
	if (g_m_lines.find(lineId) != g_m_lines.end())
		return g_m_lines[lineId];
	return NULL;
}

//获取所有的线路[只用于读取列表，不可用于计算]
std::list<AGV_LINE> MapManger::getLineList()
{
	std::list<AGV_LINE> l;
	UNIQUE_LCK lck(mtx_lines);
	for (auto p : g_m_lines) {
		AGV_LINE ll;
		ll.id = p.second->id;
		ll.startStation = p.second->startStation;
		ll.endStation = p.second->endStation;
		ll.length = p.second->length;
		ll.r = p.second->color_r;
		ll.g = p.second->color_g;
		ll.b = p.second->color_b;
		ll.draw = p.second->draw;
		l.push_back(ll);
	}
	return l;
}

//获取所有的曲线线路
std::list<AGV_ARC> MapManger::getArcList()
{
	std::list<AGV_ARC> l;
	UNIQUE_LCK lck(mtx_lines);
	for (auto p : g_m_lines) {
		AgvArc * ap = dynamic_cast<AgvArc *>(p.second);
		AGV_ARC ll;
		ll.id = ap->id;
		ll.startStation = ap->startStation;
		ll.endStation = ap->endStation;
		ll.length = ap->length;
		ll.r = ap->color_r;
		ll.g = ap->color_g;
		ll.b = ap->color_b;
		ll.draw = ap->draw;
		ll.p1x = ap->p1x;
		ll.p1y = ap->p1y;
		ll.p2x = ap->p2x;
		ll.p2y = ap->p2y;
		l.push_back(ll);
	}
	return l;
}

//获取反向线路的ID
int MapManger::getReverseLine(int lineId)
{
	UNIQUE_LCK lck(mtx_reverse);
	if (g_reverseLines.find(lineId) != g_reverseLines.end())
		return g_reverseLines[lineId];
	return 0;
}

//获取一个线路到另一个线路的转向方向 L left M middle R right
int MapManger::getLMR(int startLineId, int nextLineId)
{
	PATH_LEFT_MIDDLE_RIGHT p;
	p.lastLine = startLineId;
	p.nextLine = nextLineId;
	UNIQUE_LCK lck(mtx_lmr);
	if (g_m_lmr.find(p) != g_m_lmr.end())return g_m_lmr[p];
	return PATH_LMF_NOWAY;
}

std::list<int> MapManger::getPath(int agvId, int lastPoint, int startPoint, int endPoint, int &distance, bool changeDirect)
{
	UNIQUE_LCK lck(mtx_stations);
	UNIQUE_LCK lck2(mtx_lines);
	UNIQUE_LCK lck3(mtx_lmr);
	UNIQUE_LCK lck4(mtx_adj);
	UNIQUE_LCK lck5(mtx_reverse);

	std::list<int> result;

	distance = DISTANCE_INFINITY;

	if (lastPoint == 0) lastPoint = startPoint;
	if (g_m_stations.find(lastPoint) == g_m_stations.end())return result;
	if (g_m_stations.find(startPoint) == g_m_stations.end()) return result;
	if (g_m_stations.find(endPoint) == g_m_stations.end()) return result;
	if (g_m_stations[endPoint]->occuAgv != 0 && g_m_stations[endPoint]->occuAgv != agvId)return result;
	if (g_m_stations[startPoint]->occuAgv != 0 && g_m_stations[startPoint]->occuAgv != agvId)return result;

	if (startPoint == endPoint) {
		if (changeDirect && lastPoint != startPoint) {
			for (auto itr = g_m_lines.begin();itr != g_m_lines.end();++itr) {
				if (itr->second->startStation == lastPoint && itr->second->endStation == startPoint) {
					result.push_back(itr->first);
					distance = itr->second->length;
				}
			}
		}
		else {
			distance = 0;
		}
		return result;
	}

	std::multimap<int, int> Q;//key -->  distance ;value --> station;

	for (auto l : g_m_lines) {
		l.second->father = -1;
		l.second->distance = DISTANCE_INFINITY;
		l.second->color = AGV_LINE_COLOR_WHITE;
	}

	if (lastPoint == startPoint) {
		for (auto l : g_m_lines) {
			if (l.second->startStation == startPoint) {
				AgvLine *reverse = g_m_lines[g_reverseLines[l.first]];
				if (reverse->occuAgvs.size() == 0 && (g_m_stations[ l.second->endStation ]->occuAgv==0 || g_m_stations[l.second->endStation]->occuAgv == agvId)) {
					l.second->distance = l.second->length;
					l.second->color = AGV_LINE_COLOR_GRAY;
					Q.insert(std::make_pair(l.second->distance, l.second->id));
				}
			}
		}
	}
	else {
		for (auto l : g_m_lines) {
			if (l.second->startStation == lastPoint && l.second->endStation == startPoint) {
				AgvLine *reverse = g_m_lines[g_reverseLines[l.first]];
				if (reverse->occuAgvs.size() == 0 && (g_m_stations[l.second->endStation]->occuAgv == 0 || g_m_stations[l.second->endStation]->occuAgv == agvId)) {
					l.second->distance = 0;
					l.second->color = AGV_LINE_COLOR_GRAY;
					Q.insert(std::make_pair(l.second->distance, l.second->id));
					break;
				}
			}
		}
	}

	while (Q.size() > 0) {
		auto front = Q.begin();
		int startLine = front->second;

		if (g_m_l_adj.find(startLine) == g_m_l_adj.end()) {
			g_m_lines[startLine]->color = AGV_LINE_COLOR_BLACK;
			for (auto ll = Q.begin();ll != Q.end();) {
				if (ll->second == startLine) {
					ll = Q.erase(ll);
				}
				else {
					++ll;
				}
			}
			continue;
		}
		for (auto k : g_m_l_adj[startLine]) {
			if (g_m_lines.find(k) == g_m_lines.end()) {
				continue;
			}
			AgvLine *line = g_m_lines[k];
			if (line->color == AGV_LINE_COLOR_BLACK)continue;
			if (line->color == AGV_LINE_COLOR_WHITE) {
				AgvLine *reverse = g_m_lines[g_reverseLines[line->id]];
				if (reverse->occuAgvs.size() == 0 && (g_m_stations[line->endStation]->occuAgv == 0 || g_m_stations[line->endStation]->occuAgv == agvId)) {
					line->distance = g_m_lines[startLine]->distance + line->length;
					line->color = AGV_LINE_COLOR_GRAY;
					line->father = startLine;
					Q.insert(std::make_pair(line->distance, line->id));
				}
			}
			else if (line->color == AGV_LINE_COLOR_GRAY) {
				if (line->distance > g_m_lines[startLine]->distance + line->length) {
					AgvLine *reverse = g_m_lines[g_reverseLines[line->id]];
					if (reverse->occuAgvs.size() == 0 && (g_m_stations[line->endStation]->occuAgv == 0 || g_m_stations[line->endStation]->occuAgv == agvId)) {
						int old_distance = line->distance;
						line->distance = g_m_lines[startLine]->distance + line->length;
						line->father = startLine;
						for (auto iiitr = Q.begin();iiitr != Q.end();) {
							if (iiitr->second == line->id && iiitr->first == old_distance) {
								iiitr = Q.erase(iiitr);
							}
						}
						Q.insert(std::make_pair(line->distance, line->id));
					}
				}
			}
		}

		g_m_lines[startLine]->color = AGV_LINE_COLOR_BLACK;
		for (auto ll = Q.begin();ll != Q.end();) {
			if (ll->second == startLine) {
				ll = Q.erase(ll);
			}
			else {
				++ll;
			}
		}
		continue;
	}

	int index = -1;
	int minDis = DISTANCE_INFINITY;

	for (auto ll : g_m_lines) {
		if (ll.second->endStation == endPoint) {
			if (ll.second->distance < minDis) {
				minDis = ll.second->distance;
				index = ll.second->id;
			}
		}
	}

	distance = minDis;

	while (true) {
		if (index == -1)break;
		result.push_front(index);
		index = g_m_lines[index]->father;
	}

	if (result.size() > 0 && lastPoint != startPoint) {
		if (!changeDirect) {
			AgvLine * agv_line = g_m_lines[*(result.begin())];
			if (agv_line->startStation == lastPoint && agv_line->endStation == startPoint) {
				result.erase(result.begin());
			}
		}
	}

	return result;
}
