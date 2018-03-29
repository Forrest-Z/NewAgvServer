#pragma once

#include <list>
#include <map>
#include <mutex>

#include "Common.h"
#include "AgvLine.h"
#include "AgvArc.h"
#include "AgvStation.h"
#include <boost/noncopyable.hpp>
#include <boost/atomic/atomic.hpp>
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

	//�����ݿ��������ͼ
	bool load();

	//�û��ӿ�
	void interCreateStart(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interCreateAddStation(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interCreateAddLine(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interCreateAddArc(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interCreateFinish(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interListStation(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interListLine(TcpConnection::Pointer conn, Client_Request_Msg msg);
	void interListArc(TcpConnection::Pointer conn, Client_Request_Msg msg);

private:
	//���õ�ͼ
	void createMapStart();

	//���ú����վ��
	bool addStation(STATION_INFO s);

	//���ú������·[������������·]
	bool addLine(AGV_LINE l);

	//���ú��������[������������·]
	bool addArc(AGV_ARC arc);

	//�������˵�ͼ//֪ͨ���пͻ��˵�ͼ����,
	void createMapFinish();

	//��ȡ����·��
	std::list<int> getBestPath(int agvId, int lastStation, int startStation, int endStation, int &distance, bool canChangeDirect);

	//ռ��һ��վ��
	void occuStation(int station, int occuAgv);

	//��·�ķ���ռ��//��������ʻ�������·�����෴
	void addOccuLine(int line,int occuAgv);

	//�������ռ���վ�㣬�ͷ�
	void freeStation(int station, int occuAgv);

	//�����������·��ռ����У��ͷų�ȥ
	void freeLine(int line, int occuAgv);

	//�ͷų���ռ�õ���·������ĳ����·����Ϊ����ͣ����һ����·�ϡ�
	void freeOtherLine( int occuAgv,int exceptLine = 0 );

	//�ͷų���ռ�õ�վ�㣬����ĳ��վ�㡾��Ϊ����վ��ĳ��վ���ϡ�
	void freeOtherStation(int agvId, int excepetStation = 0);

	//ͨ����ֹվ�㣬��ȡ��·
	int getLineId(int startStation, int endStation);

	//ͨ��ID��ȡһ��վ��
	AgvStation* getAgvStation(int stationId);

	AgvStation* getAgvStationByRfid(int rfid);
	//��ȡ���е�վ��
	std::list<STATION_INFO> getStationList();

	//ͨ��ID��ȡһ����·
	AgvLine* getAgvLine(int lineId);

	//��ȡ���е�ֱ����·
	std::list<AGV_LINE> getLineList();
	
	//��ȡ���е�������·
	std::list<AGV_ARC> getArcList();

	//��ȡ������·��ID
	int getReverseLine(int lineId);

	//��ȡһ����·����һ����·��ת���� L left M middle R right
	int getLMR(int startLineId, int nextLineId);

	

	std::mutex mtx_stations;
	std::map<int, AgvStation *> g_m_stations;//վ��

	std::mutex mtx_lines;
	std::map<int, AgvLine *> g_m_lines;//��·

	std::mutex mtx_lmr;
	std::map<PATH_LEFT_MIDDLE_RIGHT, int > g_m_lmr; //������

	std::mutex mtx_adj;
	std::map<int, std::list<int> > g_m_l_adj;  //��һ����·����һ����·�Ĺ�����

	std::mutex mtx_reverse;
	std::map<int, int> g_reverseLines;//��·�����ķ�������·�ļ��ϡ�

	MapManger();

	void clear();

	int getLMR(AgvLine *lastLine, AgvLine *nextLine);

	std::list<int> getPath(int agvId, int lastPoint, int startPoint, int endPoint, int &distance, bool changeDirect);

	boost::atomics::atomic_bool isCreating;

};

