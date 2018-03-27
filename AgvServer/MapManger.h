#pragma once

#include <list>
#include <map>
#include <mutex>

#include "Common.h"
#include "AgvLine.h"
#include "AgvArc.h"
#include "AgvStation.h"
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

class MapManger :private boost::noncopyable, public boost::enable_shared_from_this<MapManger>
{
public:
	typedef boost::shared_ptr<MapManger> Pointer;

	virtual ~MapManger();

	static Pointer getInstance()
	{
		static Pointer m_inst = Pointer(new MapManger());
		return m_inst;
	}

	//重置地图
	void createMapStart();

	//重置后添加站点
	void addStation(AgvStation s);

	//重置后添加线路[不包含反向线路]
	void addLine(AgvLine l);

	//重置后添加曲线[不包含反向线路]
	void addArc(AgvArc arc);

	//重置完了地图//通知所有客户端地图更新,
	void createMapFinish();

	//2.从数据库中载入地图
	bool load();

	//获取最优路径
	std::list<int> getBestPath();

	//占领一个站点
	void occuStation(int station, int occuAgv);

	//线路的反向占用//这辆车行驶方向和线路方向相反
	void addOccuLine(int line,int occuAgv);

	//如果车辆占领该站点，释放
	void freeStation(int station, int occuAgv);

	//如果车辆在线路的占领表中，释放出去
	void freeLine(int line, int occuAgv);

	//释放车辆占用的线路，除了某条线路【因为车辆停在了一条线路上】
	void freeOtherLine( int occuAgv,int exceptLine = 0 );

	//释放车辆占用的站点，除了某个站点【因为车辆站在某个站点上】
	void freeOtherStation(int agvId, int excepetStation = 0);

	//通过起止站点，获取线路
	int getLineId(int startStation, int endStation);

	//通过ID获取一个站点
	AgvStation* getAgvStation(int stationId);

	AgvStation* getAgvStationByRfid(int rfid);
	//获取所有的站点
	std::list<AgvStation> getStationList();

	//通过ID获取一个线路
	AgvLine* getAgvLine(int lineId);

	//获取所有的线路
	std::list<AgvLine> getLineList();

	//获取反向线路的ID
	int getReverseLine(int lineId);

	//获取一个线路到另一个线路的转向方向 L left M middle R right
	int getLMR(int startLineId, int nextLineId);

	std::list<int> getPath(int agvId, int lastPoint, int startPoint, int endPoint, int &distance, bool changeDirect);

private:
	std::mutex mtx_stations;
	std::map<int, AgvStation *> g_m_stations;//站点

	std::mutex mtx_lines;
	std::map<int, AgvLine *> g_m_lines;//线路

	std::mutex mtx_lmr;
	std::map<PATH_LEFT_MIDDLE_RIGHT, int > g_m_lmr; //左中右

	std::mutex mtx_adj;
	std::map<int, std::list<int> > g_m_l_adj;  //从一条线路到另一条线路的关联表

	std::mutex mtx_reverse;
	std::map<int, int> g_reverseLines;//线路和它的反方向线路的集合。

	MapManger();

	int getLMR(AgvLine *lastLine, AgvLine *nextLine);
};

