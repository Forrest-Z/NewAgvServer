#include "MapManger.h"
#include <cmath>
#include <algorithm>
#include "DBManager.h"
#include "Common.h"
#include "Server.h"

MapManger::MapManger()
{
}


MapManger::~MapManger()
{
}

//���õ�ͼ
void MapManger::createMapStart()
{
	std::unique_lock<std::mutex> lck(mtx_stations);
	std::unique_lock<std::mutex> lck2(mtx_lines);
	std::unique_lock<std::mutex> lck3(mtx_lmr);
	std::unique_lock<std::mutex> lck4(mtx_adj);
	std::unique_lock<std::mutex> lck5(mtx_reverse);

	for (auto p : g_m_stations)
	{
		delete p.second;
	}
	g_m_stations.clear();

	for (auto p : g_m_lines)
	{
		delete p.second;
	}
	g_m_lines.clear();
	g_m_lmr.clear();
	g_m_l_adj.clear();
	g_reverseLines.clear();
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
}

//���ú����վ��
void MapManger::addStation(AgvStation s)
{
	QString insertSql = "INSERT INTO agv_station (id,station_name, station_x,station_y,station_rfid,station_color_r,station_color_g,station_color_b) VALUES (?,?,?,?,?,?,?,?);";
	QList<QVariant> params;
	params <<s.id << QString::fromStdString(s.name) << s.x << s.y
		<< s.rfid << s.color_r << s.color_g << s.color_b;
	if (DBManager::getInstance()->exeSql(insertSql, params)) {
		std::unique_lock<std::mutex> lck(mtx_stations);
		g_m_stations.insert(std::make_pair(s.id,new AgvStation(s)));
	}
	else {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "save agv statiom to database fail!");
	}
}

//���ú������·
void MapManger::addLine(AgvLine l)
{
	assert(l.line);
	QString insertSql = "INSERT INTO agv_line (id,line_startStation,line_endStation,line_color_r,line_color_g,line_color_b,line_line,line_length,line_draw) VALUES (?,?,?,?,?,?,?,?,?,?);";
	QList<QVariant> params;
	params <<l.id<< l.startStation << l.endStation  << l.color_r << l.color_g << l.color_b << l.line << l.length << l.draw;

	if (DBManager::getInstance()->exeSql(insertSql, params))
	{
		std::unique_lock<std::mutex> lck(mtx_lines);
		g_m_lines.insert(std::make_pair(l.id, new AgvLine(l)));
	}
	else {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "save agv line to database fail!");
	}
}

//���ú��������
void MapManger::addArc(AgvArc arc)
{
	assert(!arc.line);
	QString insertSql = "INSERT INTO agv_line (id,line_startStation,line_endStation,line_color_r,line_color_g,line_color_b,line_line,line_length,line_draw,line_p1x,line_p1y,line_p2x,line_p2y) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
	QList<QVariant> params;
	params << arc.id
		<< arc.startStation
		<< arc.endStation
		<< arc.color_r
		<< arc.color_g
		<< arc.color_b
		<< arc.line
		<< arc.length
		<< arc.draw
		<< arc.p1x
		<< arc.p1y
		<< arc.p2x
		<< arc.p2y;

	if (DBManager::getInstance()->exeSql(insertSql, params))
	{
		std::unique_lock<std::mutex> lck(mtx_lines);
		g_m_lines.insert(std::make_pair(arc.id, new AgvArc(arc)));
	}
	else {
		//g_log->log(AGV_LOG_LEVEL_ERROR, "save agv line to database fail!");
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

	while (changeAngle>M_PI) {
		changeAngle -= 2 * M_PI;
	}
	while (changeAngle<-1 * M_PI) {
		changeAngle += 2 * M_PI;
	}

	//�н�С��20�� ��Ϊֱ����ʻ
	if (abs(changeAngle) <= 20 * M_PI / 180) {
		//�ǶȻ���һ��
		//���ֻ��һ����·���ǾͲ��������ң�Ϊ�˽����Ķ�����ֹ�����⡣�����ֻһ����·��������һ����·�ǻ��ߣ���endAngleҪ���¼���
		if (!nextLine->line)
		{
			//��������Ƚ϶������ô�Ż��أ�����ͷ��˵��
			nextAngle = atan2(n_endY - n_startY, n_endX - n_startX);
			double changeAngle = nextAngle - lastAngle;

			while (changeAngle>M_PI) {
				changeAngle -= 2 * M_PI;
			}
			while (changeAngle<-1 * M_PI) {
				changeAngle += 2 * M_PI;
			}
			if (abs(changeAngle) <= 20 * M_PI / 180) {
				return PATH_LMR_MIDDLE;
			}
			else if (changeAngle>0) {
				return PATH_LMR_RIGHT;
			}
			else {
				return PATH_LMR_LEFT;
			}
		}
		return PATH_LMR_MIDDLE;
	}

	//�нǴ���80�㣬��Ϊ�յĻ��ȹ��󣬲��ܹ�ȥ
	if (abs(changeAngle) >= 100 * M_PI / 180) {
		//�ս��ر��
		return PATH_LMF_NOWAY;
	}

	if (changeAngle>0) {
		return PATH_LMR_RIGHT;
	}
	else {
		return PATH_LMR_LEFT;
	}

}


//�������˵�ͼ//֪ͨ���пͻ��˵�ͼ����,
void MapManger::createMapFinish()
{
	//A. ----- ͨ����ӵ�վ�㣬��·���������е� adj��Ȼ��������е�lmr��
	//1. ���������� ��A--B����·��Ȼ��B--A����·�������
	int maxId = 0;
	std::unique_lock<std::mutex> lck(mtx_lines);
	std::for_each(g_m_lines.begin(), g_m_lines.end(),[&](std::pair<int, AgvLine *> pp) {
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
	//2.����LMR
	for (auto itr = g_m_lines.begin();itr != g_m_lines.end();++itr) {
		AgvLine *a = itr->second;
		for (auto pos = g_m_lines.begin();pos != g_m_lines.end();++pos) {
			AgvLine *b = pos->second;
			if (a == b || *a == *b)continue;
			//a-->station -->b ��a��·���յ���b��·����㡣��ô����һ�������������������Ϣ��
			if (a->endStation == b->startStation && a->startStation != b->endStation) {
				PATH_LEFT_MIDDLE_RIGHT p;
				p.lastLine = a->id;
				p.nextLine = b->id;
				std::unique_lock<std::mutex> lcktemp(mtx_lmr);
				if (g_m_lmr.find(p)!=g_m_lmr.end())continue;
				g_m_lmr[p] = getLMR(a, b);
				//���浽���ݿ�
				QString insertSql = "insert into agv_lmr(lmr_lastLine,lmr_nextLine,lmr_lmr) values(?,?,?);";
				QList<QVariant> params;
				params << p.lastLine << p.nextLine << g_m_lmr[p];
				DBManager::getInstance()->exeSql(insertSql, params);
			}
		}
	}
	//3.����ADJ
	for (auto itr = g_m_lines.begin();itr != g_m_lines.end();++itr) {
		AgvLine *a = itr->second;
		for (auto pos = g_m_lines.begin();pos != g_m_lines.end();++pos) {
			AgvLine *b = pos->second;
			if (a == b || *a == *b)continue;

			PATH_LEFT_MIDDLE_RIGHT p;
			p.lastLine = a->id;
			p.nextLine = b->id;

			if (a->endStation == b->startStation && a->startStation != b->endStation) {
				std::unique_lock<std::mutex> lckTemp(mtx_adj);
				if (g_m_l_adj.find(a->id)!= g_m_l_adj.end()) 
				{
					if (std::find(g_m_l_adj[a->id].begin(), g_m_l_adj[a->id].end(), b->id) != g_m_l_adj[a->id].end()) {
						continue;
					}	
					std::unique_lock<std::mutex> lckTemp2(mtx_lmr);
					if (g_m_lmr.find(p) != g_m_lmr.end() && g_m_lmr[p] != PATH_LMF_NOWAY) {
						g_m_l_adj[a->id].push_back(b->id);
					}
				}
			}
		}
	}

	//4.��adj���浽���ݿ�
	std::unique_lock<std::mutex> lckTemp(mtx_adj);
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

	//B. ----- ֪ͨ���пͻ��ˣ�map������
	Server::getInstance()->notifyAll(Server::ENUM_NOTIFY_ALL_TYPE_MAP_UPDATE);
}

//2.�����ݿ��������ͼ
bool MapManger::load()
{

	return true;
}

//��ȡ����·��
std::list<int> MapManger::getBestPath()
{
	std::list<int> l;

	return l;
}

//ռ��һ��վ��
void MapManger::occuStation(int station, int occuAgv)
{

}

//��·�ķ���ռ��//��������ʻ�������·�����෴
void MapManger::addOccuLine(int line, int occuAgv)
{

}

//�������ռ���վ�㣬�ͷ�
void MapManger::freeStation(int station, int occuAgv)
{

}

//�����������·��ռ����У��ͷų�ȥ
void MapManger::freeLine(int line, int occuAgv)
{

}

//�ͷų���ռ�õ���·������ĳ����·����Ϊ����ͣ����һ����·�ϡ�
void MapManger::freeOtherLine(int occuAgv, int exceptLine)
{

}

//�ͷų���ռ�õ�վ�㣬����ĳ��վ�㡾��Ϊ����վ��ĳ��վ���ϡ�
void MapManger::freeOtherStation(int agvId, int excepetStation)
{

}

//ͨ����ֹվ�㣬��ȡ��·
int MapManger::getLineId(int startStation, int endStation)
{


	return 0;
}

//ͨ��ID��ȡһ��վ��
AgvStation* MapManger::getAgvStation(int stationId)
{


	return NULL;
}

AgvStation* MapManger::getAgvStationByRfid(int rfid)
{


	return NULL;
}

//��ȡ���е�վ��
std::list<AgvStation> MapManger::getStationList()
{
	std::list<AgvStation> l;

	return l;
}

//ͨ��ID��ȡһ����·
AgvLine* MapManger::getAgvLine(int lineId)
{
	return NULL;
}

//��ȡ���е���·
std::list<AgvLine> MapManger::getLineList()
{
	std::list<AgvLine> l;

	return l;
}

//��ȡ������·��ID
int MapManger::getReverseLine(int lineId)
{


	return 0;
}

//��ȡһ����·����һ����·��ת���� L left M middle R right
int MapManger::getLMR(int startLineId, int nextLineId)
{


	return 0;
}

std::list<int> MapManger::getPath(int agvId, int lastPoint, int startPoint, int endPoint, int &distance, bool changeDirect)
{
	std::list<int> l;

	return l;
}
