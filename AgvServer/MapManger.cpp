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

//重置地图
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

//重置后添加站点
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

//重置后添加线路
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

//重置后添加曲线
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

	//夹角小于20° 认为直线行驶
	if (abs(changeAngle) <= 20 * M_PI / 180) {
		//角度基本一致
		//如果只有一条线路，那就不算左中右，为了将来的东西防止出问题。如果不只一条线路，并且下一条线路是弧线，则endAngle要重新计算
		if (!nextLine->line)
		{
			//这种情况比较多见，怎么优化呢？？回头再说吧
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

	//夹角大于80°，认为拐的弧度过大，不能过去
	if (abs(changeAngle) >= 100 * M_PI / 180) {
		//拐角特别大！
		return PATH_LMF_NOWAY;
	}

	if (changeAngle>0) {
		return PATH_LMR_RIGHT;
	}
	else {
		return PATH_LMR_LEFT;
	}

}


//重置完了地图//通知所有客户端地图更新,
void MapManger::createMapFinish()
{
	//A. ----- 通过添加的站点，线路，计算所有的 adj。然后计算所有的lmr。
	//1. 构建反向线 有A--B的线路，然后将B--A的线路推算出来
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
				std::unique_lock<std::mutex> lcktemp(mtx_lmr);
				if (g_m_lmr.find(p)!=g_m_lmr.end())continue;
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

	//4.将adj保存到数据库
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

	//B. ----- 通知所有客户端，map更新了
	Server::getInstance()->notifyAll(Server::ENUM_NOTIFY_ALL_TYPE_MAP_UPDATE);
}

//2.从数据库中载入地图
bool MapManger::load()
{

	return true;
}

//获取最优路径
std::list<int> MapManger::getBestPath()
{
	std::list<int> l;

	return l;
}

//占领一个站点
void MapManger::occuStation(int station, int occuAgv)
{

}

//线路的反向占用//这辆车行驶方向和线路方向相反
void MapManger::addOccuLine(int line, int occuAgv)
{

}

//如果车辆占领该站点，释放
void MapManger::freeStation(int station, int occuAgv)
{

}

//如果车辆在线路的占领表中，释放出去
void MapManger::freeLine(int line, int occuAgv)
{

}

//释放车辆占用的线路，除了某条线路【因为车辆停在了一条线路上】
void MapManger::freeOtherLine(int occuAgv, int exceptLine)
{

}

//释放车辆占用的站点，除了某个站点【因为车辆站在某个站点上】
void MapManger::freeOtherStation(int agvId, int excepetStation)
{

}

//通过起止站点，获取线路
int MapManger::getLineId(int startStation, int endStation)
{


	return 0;
}

//通过ID获取一个站点
AgvStation* MapManger::getAgvStation(int stationId)
{


	return NULL;
}

AgvStation* MapManger::getAgvStationByRfid(int rfid)
{


	return NULL;
}

//获取所有的站点
std::list<AgvStation> MapManger::getStationList()
{
	std::list<AgvStation> l;

	return l;
}

//通过ID获取一个线路
AgvLine* MapManger::getAgvLine(int lineId)
{
	return NULL;
}

//获取所有的线路
std::list<AgvLine> MapManger::getLineList()
{
	std::list<AgvLine> l;

	return l;
}

//获取反向线路的ID
int MapManger::getReverseLine(int lineId)
{


	return 0;
}

//获取一个线路到另一个线路的转向方向 L left M middle R right
int MapManger::getLMR(int startLineId, int nextLineId)
{


	return 0;
}

std::list<int> MapManger::getPath(int agvId, int lastPoint, int startPoint, int endPoint, int &distance, bool changeDirect)
{
	std::list<int> l;

	return l;
}
